/**Tools for interacting with the linux gpio device driver, for
 * the 2.13inch e-Paper display from waveshare
 */

#ifndef GPIO_TOOLS
#define GPIO_TOOLS

#define GPIO_DEV "/dev/gpiochip0"
#define DATA 1
#define COMMAND 0
#define BUSY 0
#define FREE 1

// Initialise gpio device driver - must be done before using any other function
extern int gpio_init();

// Send hardware reset signal
extern int hardware_reset();

// Pauses the program until the display is not busy
extern int wait_busy();

// Obselete - need to get rid of
extern int clean_gpio();

// Sets the device ready for DATA or COMMAND
extern int set_data_command(int data_command);

#endif //GPIO_TOOLS