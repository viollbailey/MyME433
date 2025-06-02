#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "font.h"
#include "ssd1306.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9

#define HEARTBEAT 25

void drawLetter(int startx, int starty, char letter);
void drawMessage(int startx, int starty, char *message);

int main()
{
    stdio_init_all();

    gpio_init(HEARTBEAT);
    gpio_set_dir(HEARTBEAT, GPIO_OUT);

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    ssd1306_setup(); // initialize the display
    ssd1306_clear();
    ssd1306_update(); // Just to clear the screen

    adc_init();
    adc_gpio_init(26);
    adc_select_input(0); // reading from the ADC0 pin

    while (true) {
        char message [50];

        gpio_put(HEARTBEAT, 1); // Turns the LED on
        ssd1306_drawPixel(3,5,1);
        ssd1306_update();
        sleep_ms(500);
        gpio_put(HEARTBEAT, 0); // Turns the LED off;
        ssd1306_drawPixel(3,5,0);
        ssd1306_update();
        sleep_ms(500);

        uint16_t adcval = adc_read();

        // For example
        unsigned int t1 = to_us_since_boot(get_absolute_time());  // read the time
        sprintf(message, "%f", adcval);
        drawMessage(3, 12, message);
        ssd1306_update();
        unsigned int t2 = to_us_since_boot(get_absolute_time());  // read the time
        unsigned int tdiff = t2 - t1; // us / frame
        
        // finding the FPS and then 
        float fps = (float) 1000000 / tdiff; 
        sprintf(message, "%.02f", fps);
        drawMessage(3, 20, message);


    }
}

void drawLetter(int x, int y, char c){
    // Need to loop through ASCII[][] to find our values
    int row, col;
    row = c - 0x20; // Our ASCII table starts at 0x20
    col = 0;
    for (col = 0; col < 5; col++){ // Looping through the bytes of the letter
        char byte = ASCII[row][col];

        for (int i = 0; i < 8; i++){ // Looping through the bits of each byte
            char ink = (byte>>i) & 0b1; // Is our pixel on or off?

            ssd1306_drawPixel(x + col,y + i,ink); // set the pixel
        }
    }

}

void drawMessage(int x, int y, char * m){

    int i = 0;
    
    // Note that strings stick in one extra null character, so we can just say we're done if
    // the message char is the null character 0
    while(m[i] != 0){
        drawLetter(x + i * 5, y, m[i]); // just move by 5 each time
        i++;
    }
}
