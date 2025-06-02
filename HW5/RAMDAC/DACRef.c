#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h" // not included
#include "hardware/spi.h"
#include "math.h"

#define RAMCS 14 // Setting the CS pin for the RAM


void writeDAC(int, float); // channel, voltage
void spi_ram_init();
void spi_ram_write(uint16_t addr, float v);
float spi_ram_read(uint16_t addr);


union FloatInt {
    float f;
    uint32_t i;
};


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

    //set up the RAM CS pin
    gpio_init(RAMCS);
    gpio_put(RAMCS, 1);
    gpio_set_dir(RAMCS, GPIO_OUT);
   

    spi_ram_init(); // Initialize the ram in the write mode

    // for doing waves
    float t = 0; // Time tracker for sine wave
    // int x = 0; // Cycle tracker for triangle wave; not using right now
    float v1 = 0; // Voltage tracker for sine wave
    // float v2 = 0; // Voltage tracker triangle wave; Not using this here right now

    // For loading the RAM with the sin wave
    for(int i = 0; i < 1000; i++){
        uint16_t address = 0 + i;// Not quite sure about what address to use, I'm just gonna use 0 as the starting point
        v1 = sin(M_PI * 2 * t); // Loops once a second for 1 hz sine wave
        v1 = 1023 * (v1 + 1) / 2; // changes the -1 to 1 sin wave to a sin wave going from 0 to 1023

        t = t + 0.001; //Increment time by 1ms for the next loop   
        spi_ram_write(address, v1); // Write v to the RAM
    }

    while (1){
        t = 0;
        for(int i = 0; i < 1000; i++){
            uint16_t address = 0 + i; // Just the same start as the last one
            v1 = spi_ram_read(address); // Read from RAM at the address
            writeDAC(0,v1); // Write to the DAC
            sleep_ms(1); //delay 1ms for the next thing
        }
    }

    /* This is all the code for communicating with the DAC, we'll just do this later
    while(1){
        
        /* // Troubleshooting
        printf("I'm doing stuff\n");
        sleep_ms(500);
         

        // Main block for writing
        for(int i = 0; i < 99; i++){
            sleep_ms(10); // Update rate needs to be 50 times faster than freq, so 100 Hz should work (10ms update time)
            t = t + 0.01; //Increment time by time
            x++; // Increment cycle by 1
            
            // For Channel A, generating a 2Hz sin wave
            v1 = sin(2 * M_PI * 2 * t); // Loops twice a second
            v1 = 1023 * (v1 + 1) / 2; // changes the -1 to 1 sin wave to a sin wave going from 0 to 1023
            writeDAC(0,v1); // Writing to VOutA
            
            // For Channel B, generating a 1Hz triangle wave
            
            v2 = abs(x - 50); //Triangle wave with a freq. of 1 sec, going from 50 to 0 to 50
            v2 = 1023 * v2 / 50; // Scaling amplitude to 0 to 1023
            writeDAC(1,v2); // Writing to VOutB

        }
        t = 0; // Reset time and cycle trackers
        x = 0; 
       // writeDAC(0,0); // Hard code testing 
    }
    */
    return 0;
#endif
}

void spi_ram_init(){
    uint8_t buf[2];
    buf[0] = 0b00000001; //command. We want to write to the status
    buf[1] = 0b01000000; //value. We want to use use 01 for the sequential mode
    
    cs_deselect(RAMCS); // Makes the chip select pin go high, disabling it
    spi_write_blocking(spi_default, buf, 2);
    cs_deselect(RAMCS); // Makes the chip select pin go high, disabling it
}

void spi_ram_write(uint16_t addr, float v){
    uint8_t buf[7];
    buf[0] = 0b00000010; //command. We want to do normal writing
    buf[1] = addr >> 8; //Address high 8 bits
    buf[2] = addr & 0xff; // Address low 8 bits 

    // Use our union to do let us bitshift the float
    union FloatInt num;
    num.f = v;

    buf[3] = num.i >> 24; // float high 8 bits
    buf[4] = (num.i >> 16)& 0b11111111; // float mid high8 bits
    buf[5] = (num.i >> 8) & 0b11111111; // float mid low 8 bits
    buf[6] = num.i & 0b11111111; // float low 8 bits

    cs_select(RAMCS); //Makes the chip select pin go low, activating it
    spi_write_blocking(spi_default, buf, 7);
    cs_deselect(RAMCS); // Makes the chip select pin go high, disabling it
}

float spi_ram_read(uint16_t addr){
    
    //We have to use a slightly inefficient write_read function
    uint8_t write[7], read[7];
    write[0] = 0b00000011; //instruction to read
    write[1] = addr >> 8;//address high byte
    write[2] = addr & 0xff;//address low byte

    cs_select(RAMCS); //Makes the chip select pin go low, activating it
    spi_write_read_blocking(spi_default, write, read, 2);
    cs_deselect(RAMCS); // Makes the chip select pin go high, disabling it

    //read[0] up to read[2] is just nonsense
    //actual floats go here
    union FloatInt num;
    num.i = num.i | read[3] << 24;
    num.i = num.i | read[4] << 16;
    num.i = num.i | read[5] << 8;
    num.i = num.i | read[6];

    return num.f;

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