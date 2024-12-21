#ifndef SPI_TOOLS
#define SPI_TOOLS

#define SPI_DEV "/dev/spidev0.0"
#define SPI_MODE SPI_MODE_0
#define SPI_SPEED 20000000
#define SPI_BITS_PER_WORD 8

int spi_init();
int write_spi(uint8_t* commands, int length);

#endif 