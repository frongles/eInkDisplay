/** Program to open a device connected through the SPI terminal and feed some data through.
 * Fraser Crumpler
 * 12/12/2024
 * 
 */


#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <linux/gpio.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define SPI_DEV "/dev/spidev0.0"
#define SPI_MODE SPI_MODE_0
#define SPI_SPEED 20000000
#define SPI_BITS_PER_WORD 8
#define GPIO_DEV "/dev/gpiochip0"
#define DATA 1
#define COMMAND 0
#define BUSY 0
#define FREE 1


int spi_init();
int gpio_init();
int write_spi(int spi_fd, uint8_t* data, int length);
int set_data_command(int rq_fd, int dataCommand);
int clean_gpio(int rq_fd);
int is_busy(int rq_fd);
int wait_busy(int rq_fd);

int main() {
    //E ink display must be connected and have sufficient source voltage before beginning
    int ret;
    // Open relevant gpio lines
    printf("gpio init\n");
    int rq_fd = gpio_init();
    if (rq_fd < 0) {
        return -1;
    }
    usleep(500 * 1000);
    // Open SPI device
    printf("spi init\n");
    int spi_fd = spi_init();
    if (spi_fd < 0) {
        return -1;
    }
    usleep(500 * 1000);
    
    // Send software reset
    uint8_t message[10];
    message[0] = 0x12;
    ret = write_spi(spi_fd, message, 1);
    wait_busy(rq_fd);

    if (ret < 0) { return -1;}
    usleep(500 * 1000);
    
    printf("send gate driver control\n");
    message[0] = 0x01;
    ret = write_spi(spi_fd, message, 1);
    printf("check busy\n");
    wait_busy(rq_fd);



    clean_gpio(rq_fd);
    close(spi_fd);
    close(rq_fd);

    return 0;
}    
    

// Access the SPI driver and returns the open file descriptor
int spi_init() {

    // opens SPI_STREAM for O_RDWR read and write
    int spi_fd = open(SPI_DEV, O_RDWR);
    if (spi_fd < 0) {
        perror("Failed to open SPI device");
        return -1;
    }

    // Writes (SPI_IOC_WR_MODE) the mode on the device
    // To read the mode on the device, use SPI_IOC_RD_MODE)
    uint8_t mode = SPI_MODE;
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        perror("Failed to set mode on SPI device");
        close(spi_fd);
        return -1;
    }

    // Assign 'maximum' clock speed in Hertz.
    uint32_t speed = SPI_SPEED;
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        perror("Failed to set clock speed on SPI device");
        close(spi_fd);
        return -1;
    }

    uint8_t bitsPerWord = SPI_BITS_PER_WORD;
    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bitsPerWord) < 0){
        perror("Failed to set bits per word on SPI device");
        close(spi_fd);
        return -1;
    }


    return spi_fd;
}




// Connect to GPIO device, activates reset signal and configures for writing command.
int gpio_init() {
    // Open gpio device
    int gpio_fd = open(GPIO_DEV, O_RDONLY);
    if (gpio_fd < 0) {
        perror("gpio_init unable to open gpio device");
	return -1;
    }

    // Attributes for configuration
    struct gpio_v2_line_attribute attribute_output;
    memset(&attribute_output, 0, sizeof(attribute_output));
    attribute_output.id = GPIO_V2_LINE_ATTR_ID_FLAGS;
    attribute_output.flags = GPIO_V2_LINE_FLAG_OUTPUT;

    struct gpio_v2_line_attribute attribute_input;
    memset(&attribute_input, 0, sizeof(attribute_input));
    attribute_input.id = GPIO_V2_LINE_ATTR_ID_FLAGS;
    attribute_input.flags = GPIO_V2_LINE_FLAG_INPUT;

    struct gpio_v2_line_config_attribute config_attr_output;
    memset(&config_attr_output, 0, sizeof(config_attr_output));
    config_attr_output.attr = attribute_output;
    config_attr_output.mask = (1<<0) | (1<<1);

    struct gpio_v2_line_config_attribute config_attr_input;
    memset(&config_attr_input, 0, sizeof(config_attr_input));
    config_attr_input.attr = attribute_input;
    config_attr_input.mask = 1<<2;

    // Configuration for line request
    struct gpio_v2_line_config config;
    memset(&config, 0, sizeof(config)); // Initialises struct values to 0
    config.flags = GPIO_V2_LINE_FLAG_OUTPUT;  // indicate that the line will be used for output
    config.num_attrs = 2; // 2 attributes
    config.attrs[0] = config_attr_output;
    config.attrs[1] = config_attr_input;


    /* Name    Description         boardNum    value     flags
     *  VCC    3.3V source         1           ---------------    Not user control - always on
     *  GND    Ground              9           ---------------    Not user control
     *  DIN    MOSI                19          ---------------    Not user control - SPI device
     *  CLK    Clock               23          ---------------    Not user control - SPI device
     *  D/C    GPIO25 Data/Command 22          1 for data, 0 for command  Output
     *  RST    GPIO17 Reset        11          0    active low       output
     *  BSY    GPIO24 BUSY         18          Input, Active high
     */
    
    // Line request info
    struct gpio_v2_line_request request;
    memset(&request, 0, sizeof(request));
    (request.offsets)[0] = 17;
    (request.offsets)[1] = 25;
    (request.offsets)[2] = 24;
    strncpy(request.consumer, "LED breadboard", sizeof(request.consumer));
    request.num_lines = 3; 
    request.event_buffer_size = 0;
    request.config = config;

    // Make request
    int ret = ioctl(gpio_fd, GPIO_V2_GET_LINE_IOCTL, &request);
    close(gpio_fd);
    if (ret < 0) {
        perror("gpio_init unable to get line from ioctl");
        return -1;
    }

    // Set initial GPIO values
    struct gpio_v2_line_values values;
    values.mask = (1<<0) | (1<<1); // Activate the 0th and 1st indexes from request offsets
    values.bits = 0x00; // Set all values to 0
    ret = ioctl(request.fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &values);
    if (ret < 0) {
        perror("Failed to set initial line values");
        close(request.fd);
        return -1;
    }

    usleep(10 * 1000);
    // De activeate reset signal
    values.mask = 1<<0;
    values.bits = 1<<0;
    ret = ioctl(request.fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &values);
    if (ret < 0) {
        perror("Failed to turn off reset pin");
        close(request.fd);
        return -1;
    }


    return request.fd;
}



int write_spi(int spi_fd, uint8_t* commands, int length) {
    uint8_t *write = commands;
    uint8_t read[length];
    memset(read, 0, length);
    struct spi_ioc_transfer ts;
    memset(&ts, 0, sizeof(ts));    
    ts.tx_buf = (unsigned long)write; // Buffer to write to SPI device
    ts.rx_buf = (unsigned long)read; // Buffer to read from SPI device
    ts.len = sizeof(write); // Temporarily change word read size from default


    if(ioctl(spi_fd, SPI_IOC_MESSAGE(1), &ts) < 0) {
        perror("Failed to perform SPI transaction");
        close(spi_fd);
        return -1;
    }
    return 0;
}

    


int set_data_command(int rq_fd, int dataCommand) {

    // Set initial GPIO values
    struct gpio_v2_line_values values;
    values.mask = 2;
    if(dataCommand == DATA) {
        values.bits = 2;
    }
    else  {
        values.bits = 0;
    }
    int ret = ioctl(rq_fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &values);
    if (ret < 0) {
        perror("Failed to set data command");
        return -1;
    }

    return 0;

}



int clean_gpio(int rq_fd) {
    
    struct gpio_v2_line_values values;
    values.mask = 3;
    values.bits = 0;
    int ret = ioctl(rq_fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &values);
    if (ret < 0) {
        perror("Failed to set data command");
        return -1;
    }

    return 0;
}

// returns BUSY or FREE
int is_busy(int rq_fd) {
    struct gpio_v2_line_values values;
    memset(&values, 0, sizeof(values));
    values.mask = 1<<2;
    values.bits = 0;
    int ret = ioctl(rq_fd, GPIO_V2_LINE_GET_VALUES_IOCTL, &values);
    if (ret < 0) {
        perror("Failed to check busy line");
        return -1;
    }

    if (values.bits == 1<<2) {
        return BUSY;
    }
    else return FREE;
}
    

int wait_busy(int rq_fd) {
    int count = 0;
    while (is_busy(rq_fd) == BUSY) {
        printf("BUSY\n");
        count++;
        if (count >= 200) {
            printf("Error: wait for busy pin timeout\n");
            return -1;
        }
        sleep(1);
    }

    return 0;
}
