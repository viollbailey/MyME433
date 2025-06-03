/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define FLAG_VALUE 123

#define LIGHT 15

static float adcval = 0;

void core1_entry() {

    /*
    multicore_fifo_push_blocking(FLAG_VALUE);

    uint32_t g = multicore_fifo_pop_blocking();

    if (g != FLAG_VALUE)
        printf("Hmm, that's not right on core 1!\n");
    else
        printf("Its all gone well on core 1!\n");

     Useless garbage
    while (1){
        tight_loop_contents();
    }
    */ 

    // Initializing the LED on pin 15
    gpio_init(LIGHT);
    gpio_set_dir(LIGHT, GPIO_OUT);

    //Initializing the adcval on pin26
    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);

    while(true){
    
        uint32_t g = multicore_fifo_pop_blocking();

        if (g == 1){
            gpio_put(LIGHT, 1); //switches the LED on
            multicore_fifo_push_blocking(g);
        }

        if (g == 2){
            gpio_put(LIGHT, 0); //switches the LED off
            multicore_fifo_push_blocking(g);
        }

        if (g == 0){
            adcval = adc_read();
            adcval = 3.3 * adcval / 1023;     
            multicore_fifo_push_blocking(g);
        }
        else{
            multicore_fifo_push_blocking(g);
        }
    
    }
    //Enable g

}

int main() {
    stdio_init_all();


    // Lets you wait until screen is open (useful, omg)
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    printf("Hello, multicore!\n");

    /// \tag::setup_multicore[]

    multicore_launch_core1(core1_entry);
    //enabling core 1 and setting its function

    // Wait for it to start up

    /*
    // Core 0 is waiting for core 1 to send something into the fifo
    uint32_t g = multicore_fifo_pop_blocking();
    if (g != FLAG_VALUE)
        printf("Hmm, that's not right on core 0!\n");
    else {
        multicore_fifo_push_blocking(FLAG_VALUE);
        printf("It's all gone well on core 0!\n");
    }
    */
    int message = 0;
    while(1){
        printf("Waiting for command: (0: Read ADC0, 1: Turn LED On, 2: Turn LED Off)\n");

        message = getchar();
        sleep_ms(20);
        printf("%d\n", message - 48);

        if (message == 49){
            multicore_fifo_push_blocking(1);
            printf("LED turned on!\n");
        }
        if (message == 50){
            multicore_fifo_push_blocking(2);
            printf("LED turned off!\n");
        }
        else if (message == 48){
            multicore_fifo_push_blocking(0);
            printf("ADC0 is %0.2fV\n", adcval);
        }
        else{
            multicore_fifo_push_blocking(0);
            printf("Invalid Command\n");
        }
        uint32_t g = multicore_fifo_pop_blocking();
    }


    /// \end::setup_multicore[]
}
