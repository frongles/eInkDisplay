#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <linux/spi/spidev.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "spiTools.h"
#include "log.h"

static int spi_fd = -1;

// Access the SPI driver and returns the open file descriptor
int spi_init() {
    log_msg(LOG_INFO, "Initialising SPI");
    // opens SPI_STREAM for O_RDWR read and write
    spi_fd = open(SPI_DEV, O_RDWR);
    if (spi_fd < 0) {
        log_msg(LOG_ERROR, "Failed to open SPI_DEV");
        exit(EXIT_FAILURE);
    }

    // Writes (SPI_IOC_WR_MODE) the mode on the device
    // To read the mode on the device, use SPI_IOC_RD_MODE)
    log_msg(LOG_INFO, "Setting SPI mode");
    uint8_t mode = SPI_MODE;
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        log_msg(LOG_ERROR, "Failed to set SPI mode");
        exit(EXIT_FAILURE);
    }

    // Assign 'maximum' clock speed in Hertz.
    uint32_t speed = SPI_SPEED;
    log_msg(LOG_INFO, "Setting SPI speed");
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        log_msg(LOG_ERROR, "Failed to set SPI speed");
        exit(EXIT_FAILURE);
    }

    uint8_t bitsPerWord = SPI_BITS_PER_WORD;
    log_msg(LOG_INFO, "Setting SPI bits per word");
    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bitsPerWord) < 0){
        log_msg(LOG_ERROR, "Failed to set SPI bits per word");
        exit(EXIT_FAILURE);
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
        log_msg(LOG_ERROR, "Failed to send SPI message");
        exit(EXIT_FAILURE);
    }
    return 0;
}