// Gets gpio chip info
// from https://blog.lxsang.me/post/id/33

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/gpio.h>
#include <stdio.h>


#define GPIO_DEV "/dev/gpiochip0"

int main() {
    
    // Open gpio device
    int gpio_fd = open(GPIO_DEV, O_RDONLY);
    if (gpio_fd < 0) {
        perror("Unable to open gpio device");
	return -1;
    }
    // Get info from device
    struct gpiochip_info info;
    int ret = ioctl(gpio_fd, GPIO_GET_CHIPINFO_IOCTL, &info);
    if (ret < 0) {
        perror("Unable to get gpio chip info");
        close(gpio_fd);
        return -1;
    }

    printf("Chip name: %s\n", info.name);
    printf("Chip label: %s\n", info.label);
    printf("Number of lines: %d\n", info.lines);

    return 0;
}
