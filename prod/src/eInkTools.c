/** Tools for writing to e-Ink display
 * Fraser Crumpler
 * 12/12/2024
 * 
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>

#include "../include/stb_truetype.h"
#include "../include/eInkTools.h"
#include "../include/gpioTools.h"
#include "../include/spiTools.h"

static uint8_t display[HEIGHT][(WIDTH + 8) / 8];

// Writes a byte as a command to the display
int write_command(uint8_t command) {
    uint8_t commands[1];
    commands[0] = command;
    set_data_command(COMMAND);
    write_spi(commands, 1);

    return 0;
}


// Writes a byte as data to the display
int write_data(uint8_t command) {
    uint8_t commands[1];
    commands[0] = command;
    set_data_command(DATA);
    write_spi(commands, 1);

    return 0;
}


int init_display() {
    // Open drivers
    gpio_init();
    spi_init();

    hardware_reset();

    write_command(0x12); // SW reset
    usleep(10 * 1000);
    wait_busy();

    write_command(0x01); // Driver output control
    write_data(0xF9); // Gate lines settings - 249 + 1
    write_data(0x00);
    write_data(0x00); // First output gate, in order 0,1,2.. from 0 - 250

    write_command(0x11); // data entry mode
    write_data(0x03); // Update address in X direction, with X increment and Y increment
    //write_data(0x07);  // Update address in Y direction, with X and Y increment

    write_command(0x44); // X RAM
    write_data(0x00);
    write_data(((WIDTH - 1) >> 3) & 0xFF);

    write_command(0x45); // Y RAM
    write_data(0x00);
    write_data(0x00);
    write_data((HEIGHT- 1) & 0xFF);
    write_data(((HEIGHT - 1) >> 8) & 0xFF);

    write_command(0x4E); // Initial X
    write_data(0x00);

    write_command(0x4F); // Initial Y
    write_data(0x00);
    write_data(0x00);

    write_command(0x3C); // BorderWaveForm
    write_data(0x05); // GS transition, VSH1, follow LUT, LUT0

    write_command(0x21); // Display update control 1
    write_data(0x00); // Normal mode
    write_data(0x80); // Available source from S8 to S167

    write_command(0x18); // Read built-in temperature sensor...
    write_data(0x80);
    wait_busy();
    return 0;
}

int activate_display() {
    write_command(0x24);
    for (int j = 0; j < HEIGHT; j++) {
        for (int i = 0; i < (WIDTH + 8) / 8; i++) {
            write_data(display[j][i]);
        }
    }

    write_command(0x22); // Display update control 2

    // Enable analog
    // Load temp value
    // display with display mode 1
    // disable analog
    // disable OSC
    write_data(0xF7);

    write_command(0x20); // Activate display update sequence
    wait_busy();
    return 0;
}

int clear_display() {

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < (WIDTH + 8) / 8; j++) {
            display[i][j] = 0xFF;
        }
    }

    activate_display();
    return 0;
}

// creates some lines on the screen. For testing. 
int pattern_display() {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < (WIDTH + 8 ) / 8; j++) {
            if ((i % 16 == 0) || ((i + 1) % 16 == 0)) {
                display[i][j] = 0xFF;
            }
            else {
                display[i][j] = 0x00;
            }
        }
    }
    activate_display();
    return 0;
}

int sleep_display() {
    write_command(0x10);
    write_data(0x03);
    return 0;
}

int cleanup() {
    clear_display();
    sleep_display();
    return 0;
}
/*
int write_char(char* font, int fontsize, int x, int y, int character) {

    // Get font file and filesize
    FILE *file = fopen(font, "r");
    assert(file != NULL);
    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char *data = malloc(file_size);
    fread(data, file_size, 1, file);
    fclose(file);
    
    // Get font map
    // The font map comes in bytes. The byte location in the sequence corresponds to the pixel, 
    // with the top left corner being the start. The byte's intensity corresponds to the brightness of the 
    // pixel.
    stbtt_fontinfo fontInfo;
    stbtt_InitFont(&fontInfo, data, 0);
    float scale = stbtt_ScaleForPixelHeight(&fontInfo, fontsize);
    int width, height, xoff, yoff;
    unsigned char* bitmap = stbtt_GetCodepointBitmap(&fontInfo, scale, scale, character, &width, &height, &xoff, &yoff);


    x = x + xoff;
    y = y + yoff;
    // Convert the font byte map, to a bit map compatible with the e-ink display
    // i.e. an array of bytes whose bits correspond to active or inactive pixels
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            if (bitmap[j * width + i] > 255/2) {
                write_pixel(x + i, y + j);
            }
        }

    }
    free(data);

    return 0;
}
*/

// Write pixel function from jim crumpler
int write_pixel(int x, int y) {
    int byteX = x / 8;
    int byteY = y;
    int bit_position = 7 - x % 8;

    uint8_t value = display[byteY][byteX];
    value = value & ~(1<<bit_position);
    display[byteY][byteX] = value;
    return 0;
}