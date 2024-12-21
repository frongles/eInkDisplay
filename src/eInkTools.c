/** Tools for writing to e-Ink display
 * Fraser Crumpler
 * 12/12/2024
 * 
 */

#include <stdio.h>
#include <stdint.h>

#include "../include/eInkTools.h"
#include "../include/gpioTools.h"
#include "../include/spiTools.h"

#define HEIGHT 250
#define WIDTH 120


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
    ret = write_command(0x12);
    if (ret < 0) return -1;
    wait_busy(rq_fd);
    usleep(10 * 1000);

    // Set gate driver output control
    printf("Gate driver output control\n");
    ret = write_command(0x01);
    if (ret < 0) return -1;
    wait_busy(rq_fd);

    
    printf("Write gate driver output data\n");
    write_data(0xF9);
    write_data(0x00);
    write_data(0x00);
    wait_busy(rq_fd);
    
    // Define data entry mode sequence
    printf("Data entry mode sequence\n");
    write_command(0x11);
    wait_busy(rq_fd);
    write_data(0x03);
    wait_busy(rq_fd);
    
    // Set ram Y address start and end
    printf("Set RAM X address start end\n");
    write_command(0x44);
    wait_busy(rq_fd);

    printf("Writing ram x address data\n");
    write_data(0x00);
    write_data((120 >> 3) & 0xFF);
    wait_busy(rq_fd);
    
    // Set ram Y address start and end
    printf("Set RAM Y address start end\n");    
    write_command(0x45);
    wait_busy(rq_fd);

    uint16_t height = 249;
    printf("Writing ram y address data\n");
    write_data(0x00);
    write_data(0x00);
    write_data(height & 0xFF);
    write_data((height >> 8) & 0xFF);
    wait_busy(rq_fd);

    // Set panel border
    printf("Set panel border\n");
    write_command(0x3C);
    write_data(0x05);
    wait_busy(rq_fd);

    // Display update control
    printf("display update control 1\n");
    write_command(0x21);
    write_data(0x00);
    write_data(0x80);
    wait_busy(rq_fd);
    
    
    // Sense temperature
    printf("Sense temperature\n");
    write_command(0x18);
    write_data(0x80);
    wait_busy(rq_fd);

    // Display update control optioning
    printf("Display update control options\n");
    write_command(0x22);
    wait_busy(rq_fd);

    write_data(0x91);
    wait_busy(rq_fd);


    // Set initial X address
    printf("Set initial X address\n");
    write_command(0x4E);
    wait_busy(rq_fd);

    write_data(0x00);
    wait_busy(rq_fd);

    // Set initial Y address
    printf("Set initial Y address\n");
    write_command(0x4F);
    
    write_data(0x00);
    write_data(0x00);
    

    // Write bits to display.
    printf("Write bits to display\n");
    write_command(0x24);


    for (int i = 0; i < 250; i++) {
        for (int j = 0; j < 16; j++) {
            if (i % 16) {
                write_data(0xFF);
            }
            else {
                write_data(0x00);
            }

        }
    }


    // Display update control optioning
    printf("Display update control optioning\n");
    write_command(0x22);
    write_data(0xF7);

    // Master activation - activate display update sequence
    printf("Master activation - activate display update sequence\n");
    write_command(0x20);
    wait_busy(rq_fd);

/*
    // Deep sleep command
    printf("Enter deep sleep\n");
    write_command(0x10);
    wait_busy(rq_fd);
    
    printf("Send deep sleep data\n");
    write_data(0x03);
    wait_busy(rq_fd); */
    
    clean_gpio(rq_fd);
    close(spi_fd);
    close(rq_fd);

    return 0;
}


// Writes a byte as a command to the display
int write_command(uint8_t command) {
    uint8_t commands[10];
    commands[0] = command;
    set_data_command(COMMAND);

    int ret = write_spi(commands, 1);
    if (ret < 0) return -1;
    return 0;
}


// Writes a byte as data to the display
int write_data(uint8_t command) {
    uint8_t commands[10];
    commands[0] = command;
    set_data_command(DATA);

    int ret = write_spi(commands, 1);
    if (ret < 0) return -1;
    return 0;
}


int init_display() {
    // Open drivers
    gpio_init();
    spi_init();

    hardware_reset();

    write_command(0x12); // SW reset
    wait_busy();

    write_command(0x01); // Driver output control
    write_data(0xF9);
    write_data(0x00);
    write_data(0x00);

    write_command(0x11); // data entry mode
    write_data(0x03);

    write_command(0x44); // X RAM
    write_data(0x00);
    write_data((WIDTH >> 3) & 0xFF);

    write_command(0x45); // Y RAM
    write_data(0x00);
    write_data(0x00);
    write_data(HEIGHT & 0xFF);
    write_data((HEIGHT >> 8) & 0xFF);

    write_command(0x4E); // Initial X
    write_data(0x00);

    write_command(0x4F); // Initial Y
    write_data(0x00);
    write_data(0x00);

    write_command(0x3C); // BorderWaveForm
    write_data(0x05);

    write_command(0x21); // Display update control 1
    write_data(0x00);
    write_data(0x80);

    write_command(0x18); // Read built-in temperature sensor...
    write_data(0x80);
    wait_busy();
    return 0;
}

int activate_display() {
    write_command(0x22); // Display update control 2
    write_data(0xF7);
    write_command(0x20); // Activate display update sequence
    wait_busy();
}

int clear_display() {
    write_command(0x24);
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH / 8; j++) {
            write_data(0xFF);
        }
    }

    activate_display();
    return 0;
}


int sleep() {
    write_command(0x10);
    write_data(0x03);
    return 0;
}