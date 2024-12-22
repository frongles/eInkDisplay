/** Tools for writing to e-Ink display
 * Fraser Crumpler
 * 12/12/2024
 * 
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#define STB_TRUETYPE_IMPLEMENTATION
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
    //clean_gpio();
    return 0;
}

int write_letter(char* font, int fontsize, int x, int y, int character) {

    // Get font file and filesize
    FILE *file = fopen(font, "r");
    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    const unsigned char *data = malloc(file_size);
    fread(data, file_size, 1, file);
    fclose(file);
    
    // Get font map
    // The font map comes in bytes. The byte location in the sequence corresponds to the pixel, 
    // with the top left corner being the start. The byte's intensity corresponds to the brightness of the 
    // pixel.
    stbtt_fontinfo font;
    stbtt_InitFont(&font, data, 0);
    float scale = stbtt_ScaleForPixelHeight(&font, fontsize);
    int width, height, xoff, yoff;
    unsigned char* intensityBitmap = stbtt_GetCodepointBitmap(&font, scale, scale, character, &width, &height, &xoff, &yoff);

    // Datamap width is the number of bits / 8 rounded up, in bytes
    int byte_width;
    if (width % 8 == 0) byte_width = width / 8;
    else byte_width = (width + 8) / 8;

    uint8_t* bitmap[height][byte_width];

    // Convert the font byte map, to a bit map compatible with the e-ink display
    // i.e. an array of bytes whose bits correspond to active or inactive pixels
    for (int j = 0; j < height; j++) {
        uint8_t bits = 0x00;
        for (int i = 0; i < width; i++) {

            // After shifting 8 bits, write to the data map, and reset
            if (i % 8 == 0 && i != 0) {
                bitmap[j][i / 8 - 1] = bits;
                bits = 0x00;
            }
            
            // Add a white pixel to the bits if the intensity is less than half
            if (intensityBitmap[j * width + i] < 255/2) {
                bits += 1; 
            }
            bits << 1;
        }
        // At the end of every row, pad the right side of the last byte with white pixels
        int padding = 8 - width % 8;
        bits = bits << padding | 0xFF >> (width % 8); // shift bits to left align, and fill the end with white
        bitmap[j][byte_width - 1] = bits;
    }

    x = x + xoff;
    y = y + yoff;
    // To do - might need to swap X and Y since the display X is the short direction. 
    // Also want to convert all this entries to entering into the big data array instead.
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < byte_width; i++) {
            write_data(bitmap[j][i]);
            display[j + y][i + x] = bitmap[j][i];
        }
    }

    return 0;
}


/*
    An example to visualise converting from a map of bytes to a map of bits

    X X 0       two bytes (of any intensity), then 
    X 0 X
    0 0 X

    [1100 000]
    [1010 000]
    [0010 000]

    width of 3 => ceil(3/8)
    height of 3 => 3

    pseudocode


    foreach byte {
        uint8_t bits = 0;
        for ( int i = 0; i < 8; i++) {
            if (byte intensity > 50%) {
                bits += 1;
            }
            bits = bits << 1;
        }
        outputbitmap[byte] = bits;
    }
    outputbitmap[lastbyte] = outputbitmap[lastbyte] << (8 - width % 8) // pads the last bit with white space at the end
    // 



*/

// Write pixel function from jim crumpler
int write_pixel(uint8_t x, uint8_t y) {
    int byteX = x / 8;
    int byteY = y / 8;
    int bit_position = 7 - x % 8;

    uint8_t value = display[byteY][byteX];
    value = value & !(1<<bit_position);
    display[byteY][byteX] = value;
    return 0;
}