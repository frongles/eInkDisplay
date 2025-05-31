/**Tools for interfacing with the linux SPI device driver for use with 
 * the 2.13inch e-Paper display from waveshare
 */

#ifndef SPI_TOOLS
#define SPI_TOOLS

#define SPI_DEV "/dev/spidev0.0"
#define SPI_MODE SPI_MODE_0
#define SPI_SPEED 20000000
#define SPI_BITS_PER_WORD 8

// Initialise the SPI driver - must be done first
int spi_init();

// Write an array of commands to the device
int write_spi(uint8_t* commands, int length);

#endif 