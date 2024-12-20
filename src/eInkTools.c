/** Tools for writing to e-Ink display
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
int hardware_reset(int rq_fd);
int write_spi(int spi_fd, uint8_t* data, int length);
int write_command(int spi_fd, int rq_fd, uint8_t command);
int write_data(int spi_fd, int rq_fd, uint8_t command);
int set_data_command(int rq_fd, int dataCommand);
int clean_gpio(int rq_fd);
int is_busy(int rq_fd);
int wait_busy(int rq_fd);


int main() {
    //E ink display must be connected and have sufficient source voltage before beginning
    
    // Open relevant gpio lines
    int rq_fd = gpio_init();
    if (rq_fd < 0) {
        return -1;
    }
    int ret = hardware_reset(rq_fd);

    if (ret < 0) return -1;

    usleep(10 * 1000);

    // Open SPI device
    int spi_fd = spi_init();
    if (spi_fd < 0) {
        return -1;
    }

    // Send software reset
    printf("software reset\n");
    wait_busy(rq_fd);
    ret = write_command(spi_fd, rq_fd, 0x12);
    if (ret < 0) return -1;
    wait_busy(rq_fd);
    usleep(10 * 1000);

    // Set gate driver output control
    printf("Gate driver output control\n");
    ret = write_command(spi_fd, rq_fd, 0x01);
    if (ret < 0) return -1;
    wait_busy(rq_fd);

    
    printf("Write gate driver output data\n");
    write_data(spi_fd, rq_fd, 0xF9);
    write_data(spi_fd, rq_fd, 0x00);
    write_data(spi_fd, rq_fd, 0x00);
    wait_busy(rq_fd);
    
    // Define data entry mode sequence
    printf("Data entry mode sequence\n");
    write_command(spi_fd, rq_fd, 0x11);
    wait_busy(rq_fd);
    write_data(spi_fd, rq_fd, 0x03);
    wait_busy(rq_fd);
    
    // Set ram Y address start and end
    printf("Set RAM X address start end\n");
    write_command(spi_fd, rq_fd, 0x44);
    wait_busy(rq_fd);

    printf("Writing ram x address data\n");
    write_data(spi_fd, rq_fd, 0x00);
    write_data(spi_fd, rq_fd, (120 >> 3) & 0xFF);
    wait_busy(rq_fd);
    
    // Set ram Y address start and end
    printf("Set RAM Y address start end\n");    
    write_command(spi_fd, rq_fd, 0x45);
    wait_busy(rq_fd);

    uint16_t height = 249;
    printf("Writing ram y address data\n");
    write_data(spi_fd, rq_fd, 0x00);
    write_data(spi_fd, rq_fd, 0x00);
    write_data(spi_fd, rq_fd, height & 0xFF);
    write_data(spi_fd, rq_fd, (height >> 8) & 0xFF);
    wait_busy(rq_fd);

    // Set panel border
    printf("Set panel border\n");
    write_command(spi_fd, rq_fd, 0x3C);
    write_data(spi_fd, rq_fd, 0x05);
    wait_busy(rq_fd);

    // Display update control
    printf("display update control 1\n");
    write_command(spi_fd, rq_fd, 0x21);
    write_data(spi_fd, rq_fd, 0x00);
    write_data(spi_fd, rq_fd, 0x80);
    wait_busy(rq_fd);
    
    
    // Sense temperature
    printf("Sense temperature\n");
    write_command(spi_fd, rq_fd, 0x18);
    write_data(spi_fd, rq_fd, 0x80);
    wait_busy(rq_fd);

    // Display update control optioning
    printf("Display update control options\n");
    write_command(spi_fd, rq_fd, 0x22);
    wait_busy(rq_fd);

    write_data(spi_fd, rq_fd, 0x91);
    wait_busy(rq_fd);


    // Set initial X address
    printf("Set initial X address\n");
    write_command(spi_fd, rq_fd, 0x4E);
    wait_busy(rq_fd);

    write_data(spi_fd, rq_fd, 0x00);
    wait_busy(rq_fd);

    // Set initial Y address
    printf("Set initial Y address\n");
    write_command(spi_fd, rq_fd, 0x4F);
    
    write_data(spi_fd, rq_fd, 0x00);
    write_data(spi_fd, rq_fd, 0x00);
    

    // Write bits to display.
    printf("Write bits to display\n");
    write_command(spi_fd, rq_fd, 0x24);


    for (int i = 0; i < 250; i++) {
        for (int j = 0; j < 16; j++) {
            if (i % 16) {
                write_data(spi_fd, rq_fd, 0xFF);
            }
            else {
                write_data(spi_fd, rq_fd, 0x00);
            }

        }
    }


    
    // Display update control optioning
    printf("Display update control optioning\n");
    write_command(spi_fd, rq_fd, 0x22);
    write_data(spi_fd, rq_fd, 0xF7);

    // Master activation - activate display update sequence
    printf("Master activation - activate display update sequence\n");
    write_command(spi_fd, rq_fd, 0x20);
    wait_busy(rq_fd);

/*
    // Deep sleep command
    printf("Enter deep sleep\n");
    write_command(spi_fd, rq_fd, 0x10);
    wait_busy(rq_fd);
    
    printf("Send deep sleep data\n");
    write_data(spi_fd, rq_fd, 0x03);
    wait_busy(rq_fd); */
    
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
    return request.fd;
}


int hardware_reset(int rq_fd) {
    
    struct gpio_v2_line_values values;
    int ret;
/*
    values.mask = 1<<0 | 1<<0;
    values.bits = 1<<0;
    ret = ioctl(rq_fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &values);
    if (ret < 0) {
        perror("Failed to set initial line values");
        close(rq_fd);
        return -1;
    } */

//    usleep(10 * 1000);

    values.mask = 1<<0 | 1<<1; // Activate the 0th and 1st indexes from request offsets
    values.bits = 0;
    ret = ioctl(rq_fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &values);
    if (ret < 0) {
        perror("Failed to set initial line values");
        close(rq_fd);
        return -1;
    }


    usleep(10 * 1000);



    // De activeate reset signal - drive reset pin high
    values.mask = 1<<0;
    values.bits = 1<<0;
    ret = ioctl(rq_fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &values);
    if (ret < 0) {
        perror("Failed to turn off reset pin");
        close(rq_fd);
        return -1;
    }

    usleep(20 * 1000);

    return 0;
}


int write_spi(int spi_fd, uint8_t* commands, int length) {
    uint8_t *write = commands;
    uint8_t read[length];
    memset(read, 0, length);
    struct spi_ioc_transfer ts;
    memset(&ts, 0, sizeof(ts));
    ts.tx_buf = (unsigned long)write; // Buffer to write to SPI device
    ts.rx_buf = (unsigned long)read; // Buffer to read from SPI device
    ts.len = length; // Temporarily change word read size from default
    ts.bits_per_word = SPI_BITS_PER_WORD;
    ts.delay_usecs = 0;
    ts.cs_change = 0;
    ts.word_delay_usecs = 0;


    if(ioctl(spi_fd, SPI_IOC_MESSAGE(1), &ts) < 0) {
        perror("Failed to perform SPI transaction");
        close(spi_fd);
        return -1;
    }
    return 0;
}

int write_command(int spi_fd, int rq_fd, uint8_t command) {
    uint8_t commands[10];
    commands[0] = command;
    set_data_command(rq_fd, COMMAND);
//    usleep(10 * 1000);

    int ret = write_spi(spi_fd, commands, 1);
    if (ret < 0) return -1;
    return 0;
}


// Writes a byte as data to the SPI stream
int write_data(int spi_fd, int rq_fd, uint8_t command) {
    uint8_t commands[10];
    commands[0] = command;
    set_data_command(rq_fd, DATA);

    int ret = write_spi(spi_fd, commands, 1);
    if (ret < 0) return -1;
    return 0;
}
    

// Sets the D/C# pin to 1 for DATA and 0 for COMMAND
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


// Sets used gpio pins to 0
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

// Checks whether the busy pin is BUSY or FREE
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


// Waits until the busy pin is FREE
int wait_busy(int rq_fd) {
    int count = 0;
    while (is_busy(rq_fd) == BUSY) {
        printf("BUSY\n");
        count++;
        if (count >= 200) {
            printf("Error: wait for busy pin timeout\n");
            return -1;
        }
        usleep(500 * 1000);
    }

    return 0;
}
