#ifndef GPIO_TOOLS
#define GPIO_TOOLS

#define GPIO_DEV "/dev/gpiochip0"
#define DATA 1
#define COMMAND 0
#define BUSY 0
#define FREE 1

extern int hardware_reset();
extern int gpio_init();
extern int wait_busy();
extern int clean_gpio();
extern int set_data_command(int data_command);

#endif //GPIO_TOOLS