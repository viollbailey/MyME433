#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9

#define CHIP_ADDRESS 0b0100000 // 7 bit address of our chip

#define HEARTBEAT 25 // HEARTBEAT pin
/* add this code in every infinite loop
    gpio_put(HEARTBEAT, 1); // Turns the LED on
    sleep_ms(500);
    gpio_put(HEARTBEAT, 0); // Turns the LED off
*/

void setPin(unsigned char address, unsigned char register, unsigned char value);
unsigned char readPin(unsigned char address, unsigned char register);
void chip_init(unsigned char address);

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


    unsigned char address = CHIP_ADDRESS;
    chip_init(address); // Setting up our GP7 as output and GP0 as input
    // Also setting everything else to input just cause

    while (true) {
        gpio_put(HEARTBEAT, 1); // Turns the LED on
        sleep_ms(500);
        gpio_put(HEARTBEAT, 0); // Turns the LED off;
        /*
        if (!(readPin() & 0b00000001)){ // Looking at the OLAT for

        }
        */
    }
    return 0;
}


void setPin(unsigned char address, unsigned char reg, unsigned char value){
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = value;
    i2c_write_blocking(i2c_default, address, buf, 2, false);

}

unsigned char readPin(unsigned char address, unsigned char reg){
    unsigned char buf;
    i2c_write_blocking(i2c_default, address, &reg, 1, true);  // true to keep master control of bus
    i2c_read_blocking(i2c_default, address, &buf, 1, false);  // false - finished with bus
    return buf;
}

void chip_init(unsigned char address){
    // Our hard-coded address is just 000
    setPin(address, 0x00, 0b01111111); // Need to write to IODIR,
    // IODIR register is 0x00
    // Set GP7 to output with 0, everything else can be input with 1
    setPin(address, 0x0A, 0b00000000);
    // Setting the OLAT (register 0x0A) to be low by default
}

