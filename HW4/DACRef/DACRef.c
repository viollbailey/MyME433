#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h" // not included
#include "hardware/spi.h"
#include "math.h"

void writeDAC(int, float); // channel, voltage

static inline void cs_select(uint cs_pin) { // inline forces the program to copy and paste the function, speeding up at cost of memory
    asm volatile("nop \n nop \n nop"); // Assembly code to skip clock cycles, keeps us from flipping the pin too quickly
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop"); // 
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // 
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop"); // 
}

int main()
{
    stdio_init_all();

    // Make sure the defaults are defined properly
    #if !defined(spi_default) || !defined(PICO_DEFAULT_SPI_SCK_PIN) || !defined(PICO_DEFAULT_SPI_TX_PIN) || !defined(PICO_DEFAULT_SPI_RX_PIN) || !defined(PICO_DEFAULT_SPI_CSN_PIN)
#warning spi/spi_flash example requires a board with SPI pins
    puts("Default SPI pins were not defined");
#else
   
    // Initializing the SPI
    spi_init(spi_default, 12000); // the baud, or bits per second. Want it to be slow for debugging
                                    // 12kHz = slow
                                    // Maximum is bandwidth of nScope (100kHz)
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    // Above pins are default pins on the bottom right of the PICO

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
    gpio_set_dir(PICO_DEFAULT_SPI_CSN_PIN, GPIO_OUT);
   
    float t = 0; // Time tracker
    float v = 0; // Voltage tracker

    while(1){
        printf("I'm doing stuff");
        sleep_ms(500);
        
        /* Main block for writing
        for(int i = 0; i < 100; i++){
            sleep_ms(10); // Update rate needs to be 50 times faster than freq, so 100 Hz should work (10ms update time)
            t = t + 0.1;
            v = 3.3 * (sin(t) + 1) / 1024;
            writeDAC(0,v);
        }
        */
       writeDAC(0,0); // Hard code testing 
    }

    return 0;
#endif
}


void writeDAC(int channel, float voltage){
    uint8_t data[2];
    int len = 2;

    // Data should look like 0bc111vvvv and 0bvvvvvv00;
    // c is the Channel (0 = A, 1 = B)
    // vvv... is the 10 bit unsigned int representing the voltage

    /*
    v = v >>
    data[0] = 0;
    data[0] = data[0] | (channel<<7);
    data[0] = data[0] | (0b111<<4);
    data[0] = data[0] | ();
    data[1] = 255;
    */
    data[0] = 0b01010101;
    data[1] = 255;
    cs_select(PICO_DEFAULT_SPI_CSN_PIN); //Makes the chip select pin go low, activating it
    spi_write_blocking(spi_default, data, len); // where data is a uint8_t array with length len
    cs_deselect(PICO_DEFAULT_SPI_CSN_PIN); // Makes the chip select pin go high, disabling it
}