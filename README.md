# E Ink HAT display tools

Collection of code that uses the raspberry pi zero 2W with raspberyy pi OS, and the 2.13inch E-Ink Display HAT from Waveshare to display lines and text.

## Usage
To use, install stb_truetype.h as linked below, and fill in the paths to the fonts you want in display.c
Change display.c to print whatever you would like to the display.

## Dependencies

### Font Reading
To load fonts, I used this library. Just add the stb_truetype.h header in the /include folder.
https://github.com/nothings/stb

### Display
For this project I used a 2.13inch E-Ink Display HAT from waveshare. 
https://www.amazon.com.au/dp/B07Z1WYRQH?ref_=pe_19115062_429603572_302_E_DDE_dt_1

### Raspberry pi
https://www.raspberrypi.com/products/raspberry-pi-zero-2-w/

