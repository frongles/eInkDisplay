#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <stdio.h>
#include <stdint.h>

#include "../include/spiTools.h"

static int spi_fd = -1;

// Access the SPI driver and returns the open file descriptor
int spi_init() {

    // opens SPI_STREAM for O_RDWR read and write
    spi_fd = open(SPI_DEV, O_RDWR);
    if (spi_fd < 0) {
        perror("Failed to open SPI device");
        return -1;
    }

    // Writes (SPI_IOC_WR_MODE) the mode on the device
    // To read the mode on the device, use SPI_IOC_RD_MODE)
    uint8_t mode = SPI_MODE;
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        perror("Failed to set mode on SPI device");
        close(spi_fd);
        return -1;
    }

    // Assign 'maximum' clock speed in Hertz.
    uint32_t speed = SPI_SPEED;
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        perror("Failed to set clock speed on SPI device");
        close(spi_fd);
        return -1;
    }

    uint8_t bitsPerWord = SPI_BITS_PER_WORD;
    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bitsPerWord) < 0){
        perror("Failed to set bits per word on SPI device");
        close(spi_fd);
        return -1;
    }

    return 0;
}

// Writes a list of commands to SPI driver
int write_spi(uint8_t* commands, int length) {
    uint8_t *write = commands;
    uint8_t read[length];
    memset(read, 0, length);
    struct spi_ioc_transfer ts;
    memset(&ts, 0, sizeof(ts));
    ts.tx_buf = (unsigned long)write; // Buffer to write to SPI device
    ts.rx_buf = (unsigned long)read; // Buffer to read from SPI device
    ts.len = length; // Temporarily change word read size from default
    ts.bits_per_word = SPI_BITS_PER_WORD;
    ts.delay_usecs = 0;
    ts.cs_change = 0;
    ts.word_delay_usecs = 0;


    if(ioctl(spi_fd, SPI_IOC_MESSAGE(1), &ts) < 0) {
        perror("Failed to perform SPI transaction");
        close(spi_fd);
        return -1;
    }
    return 0;
}