/** Program to open a device connected through the SPI terminal and feed some data through.
 * Fraser Crumpler
 * With help from chatGPT
 * 12/12/2024
 * 
 */


#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define SPI_DEV "/dev/spidev0.0"
#define SPI_MODE SPI_MODE_0
#define SPI_SPEED
#define SPI_BITS_PER_WORD 8
#define GPIO_DEV "/dev/gpiochip0"

int main() {
    //E ink display must be connected and have sufficient source voltage before beginning
    spi_connect();
    gpio_init();
    


int spi_connect() {

    // opens SPI_STREAM for O_RDWR read and write
    int spi_fd = open(SPI_DEV, O_RDWR);
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
    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &SPI_BITS_PER_WORD) < 0){
        perror("Failed to set bits per word on SPI device");
        close(spi_fd);
        return -1;
    }

    // Specify transfer data and settings
    uint8_t write[] =;
    uint8_t read[] = ;
    struct spi_ioc_transfer transferStructure = {
        .tx_buf = (unsigned long)write; // Buffer to write to SPI device
        .rx_buf = (unsigned long)read; // Buffer to read from SPI device
        .len = sizeof(write); // Temporarily change word read size from default
    };

    if(ioctl(spi_fd, SPI_IO_MESSAGE(1), &transferStructure) < 0) {
        perror("Failed to perform SPI transaction");
        close(spi_fd);
        return -1;
    }

    // Transaction complete


    return 0;
}


int gpioInit() {
    // Open gpio device
    int gpio_fd = open(GPIO_DEV, O_RDONLY);
    if (gpio_fd < 0) {
        perror("Unable to open gpio device");
    }
