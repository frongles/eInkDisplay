#ifndef EINKTOOLS
#define EINKTOOLS

#define HEIGHT 250
#define WIDTH 120
#include <stdint.h>

int write_command(uint8_t command);
int write_data(uint8_t data);
int init_display();
int activate_display();
int clear_display();
int sleep();


#endif