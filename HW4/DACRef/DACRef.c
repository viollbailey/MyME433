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
   
    float t = 0; // Time tracker for sine wave
    int x = 0; // Cycle tracker for triangle wave
    float v1 = 0; // Voltage tracker for sine wave
    float v2 = 0; // Voltage tracker triangle wave;

    while(1){
        printf("I'm doing stuff\n");
        sleep_ms(500);
        
        // Main block for writing
        for(int i = 0; i < 100; i++){
            sleep_ms(10); // Update rate needs to be 50 times faster than freq, so 100 Hz should work (10ms update time)
            t = t + 0.01; //Increment time by time
            x = x++; // Increment cycle by 1
            
            // For Channel A, generating a 2Hz sin wave
            v1 = sin(2 * M_PI * 2 * t); // Loops twice a second
            v1 = 1023 * (v1 + 1) / 2; // changes the -1 to 1 sin wave to a sin wave going from 0 to 1023
            writeDAC(0,v1); // Writing to VOutA
            
            // For Channel B, generating a 1Hz triangle wave
            
            v2 = (float) abs(x - 50); //Triangle wave with a freq. of 1 sec, going from 50 to 0 to 50
            v2 = 1023 * v2 / 50; // Scaling amplitude to 0 to 1023
            writeDAC(1,v2); // Writing to VOutB

        }
        t = 0; // Reset time and cycle trackers
        x = 0; 
       // writeDAC(0,0); // Hard code testing 
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
    // Note that voltage goes from 0 up to 1023 as the maximum


    uint volt = voltage; // Need to convert to an int to get the right bits

    data[0] = 0;
    data[0] = data[0] | (channel<<7); // Setting channel to the leftmost bit
    data[0] = data[0] | (0b111<<4); // Setting default 111 to next 3 bits
    data[0] = data[0] | (volt>>6); // Setting the last 4 bits in data[0] to the first 4 bits of voltage 
    
    data[1] = 0b11111111;
    data[1] = data[1] & (volt<<2); // Setting the first 6 bits of data to the last 6 bits of the voltage and the next 2 bits to 0


    /* Hard coding test
    data[0] = 0b01111000;
    data[1] = 0b00000000;
    */

    cs_select(PICO_DEFAULT_SPI_CSN_PIN); //Makes the chip select pin go low, activating it
    spi_write_blocking(spi_default, data, len); // where data is a uint8_t array with length len
    cs_deselect(PICO_DEFAULT_SPI_CSN_PIN); // Makes the chip select pin go high, disabling it
}