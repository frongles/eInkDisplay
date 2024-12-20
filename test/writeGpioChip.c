// Gets gpio chip info
// Adapted from https://blog.lxsang.me/post/id/33 and updated to V2

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/gpio.h>
#include <stdio.h>
#include <string.h>


#define GPIO_DEV "/dev/gpiochip0"

int main() {
    
    // Open gpio device
    int gpio_fd = open(GPIO_DEV, O_RDONLY);
    if (gpio_fd < 0) {
        perror("Unable to open gpio device");
	return -1;
    }
    // Configuration for line request
    struct gpio_v2_line_config config;
    memset(&config, 0, sizeof(config));
    config.flags = GPIO_V2_LINE_FLAG_OUTPUT;  // indicate that the line will be used for output
    config.num_attrs = 0; // No extra attributes; the config will apply to all requests

    // Line request info
    struct gpio_v2_line_request request;
    memset(&request, 0, sizeof(request));
    request.offsets[0] = 17;
    strncpy(request.consumer, "LED breadboard", sizeof(request.consumer));
    request.num_lines = 1; 
    request.event_buffer_size = 0;
    request.config = config;

    // Make request
    int ret = ioctl(gpio_fd, GPIO_V2_GET_LINE_IOCTL, &request);
    close(gpio_fd);
    if (ret < 0) {
        perror("Unable to get line from ioctl");
        return -1;
    }

    // Specify values and lines to be considered
    struct gpio_v2_line_values values;
    for (int i = 0; i < 10; i++) {
        values.mask = 1<<0; // Actives the 0th index of request.offsets
        values.bits = 1<<0; // Sets the 0th line to high
        ret = ioctl(request.fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &values);
        if (ret < 0) {
            perror("Failed to set line values");
            close(request.fd);
            return -1;
        }
        sleep(1);
        values.bits = 0;
        ret = ioctl(request.fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &values);
        if (ret < 0) {
            perror("Failed to set line values");
            close(request.fd);
            return -1;
        }
        sleep(1);
    }

    close(request.fd);
    return 0;
}

