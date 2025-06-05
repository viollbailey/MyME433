#include <stdio.h>
#include "pico/stdlib.h"
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
        char m[10]; //Python testing
        scanf("%s",m); //Python testing

        setSaveImage(1);
        while(getSaveImage()==1){}
        convertImage();
        int com_upper = findLine(IMAGESIZEY/4); // calculate the position of the center of the ine
        //int com_lower = findLine(3 * IMAGESIZEY/4); // calculate the position of the center of the ine
        setPixel(IMAGESIZEY/4,com_upper,0,255,0); // draw the center so you can see it in python
        //setPixel(3* IMAGESIZEY/4,com_lower,0,255,0); // draw the center so you can see it in python
        printImage(); // PythonTesting
        //printf("%d\r\n",com); // comment this when testing with python
    }
}

