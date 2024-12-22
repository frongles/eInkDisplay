/**Tools for interfacing with the 2.13inch e-Paper display from waveshare
 */


#ifndef EINKTOOLS
#define EINKTOOLS

#define HEIGHT 250 // in pixels
#define WIDTH 122 // in pixels
#define MONACO "../fonts/Monaco.ttf"

// Initialise the display
// This must be run first
int init_display();

// Send a single command to the display
int write_command(uint8_t command);

// Send a single byte of data to the display
// The data read is small endian
int write_data(uint8_t data);

// Refreshes the display, writing any data in RAM to the pixels
int activate_display();

// Clears the display
int clear_display();

// Creates a pattern on the screen - debugging
int pattern_display();

// Put the display to sleep - low power mode
// The display should be left in sleep mode when not in use
int sleep_display();

// Clear the screen and put to sleep for storage/unplugging the device
int cleanup();

#endif