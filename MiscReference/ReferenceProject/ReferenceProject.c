#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h" // For calling adc functions on the module, remember adding to the makelist linked libraries!

int main()
{
    stdio_init_all();
    adc_init(); // init the adc module
    adc_gpio_init(26); // Set ADC0 as adc input, note that it's GP# NOT pin#
    adc_

    while (!stdio_usb_connected()){ // Can be used to wait until the port is open (in our case via screen)
        sleep_ms(100);
    }

    printf("Start!\n");

    while (true) {
        char message[100];
        scanf("%s", message);
        printf("message: %s\r\n", message);
        sleep_ms(1000);
    }
}
