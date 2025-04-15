#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h" // For calling adc functions on the module, remember adding to the makelist linked libraries!

#define ADCPIN_NUM 26 //GP#s for the pins
#define LEDOUTPIN_NUM 10
#define BUTTONPIN_NUM 20

int buttonVal = 0; //For reading the value of the input pin

int main()
{
    stdio_init_all();

    gpio_init(LEDOUTPIN_NUM);
    gpio_init(BUTTONPIN_NUM);

    gpio_set_dir(LEDOUTPIN_NUM, GPIO_OUT);
    gpio_set_dir(BUTTONPIN_NUM, GPIO_IN);

    /*
        gpio_get(PIN_NUM); for getting input pin
        gpio_put(PIN_NUM, 0 or 1); for output pin
    */

    adc_init(); // init the adc module
    adc_gpio_init(ADCPIN_NUM); // Set ADC0 as adc input, note that it's GP# NOT pin#
    adc_select_input(0); // Select to read

    // uint16_t result = acd_read(); For reading the result of the 12 bit ADC input

    while (!stdio_usb_connected()){ // Can be used to wait until the port is open (in our case via screen)
        sleep_ms(100);
    }

    printf("Start!\n");

    gpio_put(LEDOUTPIN_NUM, 1); // Turns the LED on

    printf("LED has been turned on\n");
    int buttonVal = 0;

    while(!buttonVal){
        buttonVal = gpio_get(BUTTONPIN_NUM); // Waiting until a button has been pressed
    }

    printf("LED has been turned off\n");
    gpio_put(LEDOUTPIN_NUM, 0); // Turn LED off

    // Loop for asking for samples
    int sampleNumber = 0;
    float voltValue = 0;

    while (true) {
        printf("Enter a number of analog samples to take (between 1 and 100):\n");
        scanf("%d", &sampleNumber);
        printf("%d\n", sampleNumber);
        for (int i = 0; i < sampleNumber; i++){
            uint16_t result = adc_read(); // Reading ADC as 12 bit number
            voltValue = (float) 3.3 * result / (4095); // Converting to voltage
            printf("Sample %d: %.2f V\r\n", i, voltValue);
            sleep_ms(10); // Wait 10 ms for a frequency of 1/0.01 = 100 Hz
        }
        sleep_ms(1000);
    }
}
