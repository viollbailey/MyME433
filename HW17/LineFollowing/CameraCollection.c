#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math.h"

#include "pico/stdlib.h"
#include "pico/sem.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#include "cam.h"

int main()
{
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Hello, camera!\n");

    init_camera_pins();
 
    while (true) {
        // uncomment these and printImage() when testing with python 
        // char m[10]; //Python testing
        // scanf("%s",m); //Python testing

        setSaveImage(1);
        while(getSaveImage()==1){}
        convertImage();
        int com_lower = findLine(9 * IMAGESIZEY/10); // calculate the position of the center of the ine
        int com_upper = findLine(IMAGESIZEY/10); // calculate the position of the center of the ine

        // For testing with python or screen
        // setPixel(IMAGESIZEY/2,com,0,255,0); // draw the center so you can see it in python
        // Not necessary if not testing it with python, but then we need to add some delay
        sleep_ms(50);

        // printImage(); // PythonTesting (Comment if not using python)
        printf("%d %d\r\n",com_lower, com_upper); // comment this when testing with python
    }
}

