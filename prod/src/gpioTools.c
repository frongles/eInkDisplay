// Fraser Crumpler 24-12-2024
// For use with e Ink display. Contains gpio related functions

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/gpio.h>
#include <string.h>
#include <stdint.h>

#include "../include/gpioTools.h"

static int line_rq = -1;

// Connect to GPIO device, activates reset signal and configures for writing command.
int gpio_init() {

    // Open gpio device
    gpio_fd = open(GPIO_DEV, O_RDONLY);
    if (gpio_fd < 0) {
        perror("gpio_init unable to open gpio device");
	return -1;
    }

    // Output configuration attributes
    struct gpio_v2_line_attribute attribute_output;
    memset(&attribute_output, 0, sizeof(attribute_output));
    attribute_output.id = GPIO_V2_LINE_ATTR_ID_FLAGS;
    attribute_output.flags = GPIO_V2_LINE_FLAG_OUTPUT;

    // Input configuration attributes
    struct gpio_v2_line_attribute attribute_input;
    memset(&attribute_input, 0, sizeof(attribute_input));
    attribute_input.id = GPIO_V2_LINE_ATTR_ID_FLAGS;
    attribute_input.flags = GPIO_V2_LINE_FLAG_INPUT;

    struct gpio_v2_line_config_attribute config_attr_output;
    memset(&config_attr_output, 0, sizeof(config_attr_output));
    config_attr_output.attr = attribute_output;
    config_attr_output.mask = (1<<0) | (1<<1); // The 0th and 1st index will be output

    struct gpio_v2_line_config_attribute config_attr_input;
    memset(&config_attr_input, 0, sizeof(config_attr_input));
    config_attr_input.attr = attribute_input;
    config_attr_input.mask = 1<<2;  // the 2nd index will be input

    // Configuration for line request
    struct gpio_v2_line_config config;
    memset(&config, 0, sizeof(config)); 
    config.flags = GPIO_V2_LINE_FLAG_OUTPUT;  // Default value is output (overridden in attributes)
    config.num_attrs = 2;
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
    (request.offsets)[0] = 17; // reset pin
    (request.offsets)[1] = 25; // Data/Command pin
    (request.offsets)[2] = 24; // Busy pin
    strncpy(request.consumer, "eInk Display", sizeof(request.consumer));
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
    rq_fd = request.fd;
    return 0;
}


int hardware_reset() {
    
    struct gpio_v2_line_values values;
    int ret;

    values.mask = 1<<0 | 1<<1;
    values.bits = 0; // set reset and D/C pin to 0
    ret = ioctl(rq_fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &values);
    if (ret < 0) {
        perror("Failed to set initial line values");
        close(rq_fd);
        return -1;
    }

    usleep(10 * 1000);

    // De activate reset pin - set to 1
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



// Sets used gpio pins to 0
int clean_gpio() {
    
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
int is_busy() {
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
int wait_busy() {
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


// Sets the D/C# pin to 1 for DATA and 0 for COMMAND
int set_data_command(int dataCommand) {

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