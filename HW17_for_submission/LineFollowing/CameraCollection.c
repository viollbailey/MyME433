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

//defining the motor pins
#define LEFTMOTOR_PWM 18
#define LEFTMOTOR_PHASE 19
#define RIGHTMOTOR_PWM 20
#define RIGHTMOTOR_PHASE 21

static float LeftDuty = 40; // Just defining duty cycle up here cause it's easier
static float RightDuty = 40; // 0 = 0%, 100 = 100%

static float min_speed = 20; // Minimum duty cycle ONCE IN MOTION (starting min is 40)
static float max_speed = 60; // Uck
#define ERROR_MARGIN 15
#define DEADZONE 5

/*
Note that the robot motors will only definitely start turning at 40%
Didn't test exactly when they stop, but probably don't go below 20% if
we want them to keep going
*/

// I wanna clean up the main function a bit so UNFORTUNATELY I'm gonna be making other
// functions.
// I'm still gonna initialize in the main though just cause it's easier to me

void set_motors(uint16_t wrap);


int main()
{
    stdio_init_all();

    gpio_set_function(LEFTMOTOR_PWM, GPIO_FUNC_PWM);
    gpio_set_function(RIGHTMOTOR_PWM, GPIO_FUNC_PWM);
    gpio_init(LEFTMOTOR_PHASE);
    gpio_set_dir(LEFTMOTOR_PHASE, GPIO_OUT);
    gpio_init(RIGHTMOTOR_PHASE);
    gpio_set_dir(RIGHTMOTOR_PHASE, GPIO_OUT);

    // Lets just run at 20kHz PWM since that was what it was for ME333
    uint slice_num_left = pwm_gpio_to_slice_num(LEFTMOTOR_PWM);
    uint slice_num_right = pwm_gpio_to_slice_num(RIGHTMOTOR_PWM);
    float div = 1; // must be between 1-255
    pwm_set_clkdiv(slice_num_left, div); // divider set
    pwm_set_clkdiv(slice_num_right, div); // divider set
    uint16_t wrap = 7500; // when to rollover, must be less than 65535
    // Sys clock is 150MHz, so PWM freq = 150 MHz / wrap / divider
    // want 20kHz so 150MHz / 7500 = 20kHz
    pwm_set_wrap(slice_num_left, wrap);
    pwm_set_enabled(slice_num_left, true); // turn on the PWM
    pwm_set_wrap(slice_num_right, wrap);
    pwm_set_enabled(slice_num_right, true); // turn on the PWM

    pwm_set_gpio_level(LEFTMOTOR_PWM, 0); // Set duty cycle as percentage of the wrap
    pwm_set_gpio_level(RIGHTMOTOR_PWM, 0); // Set duty cycle as percentage of the wrap
    gpio_put(LEFTMOTOR_PHASE, 0);
    gpio_put(RIGHTMOTOR_PHASE, 0);

    // Wait until the screen program is open
    
    
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Hello, Robot!\n");
    
    

    init_camera_pins();
 
    /* printf("Set min of the error\n");
    scanf("%d, %d", &max_error_left, &max_error_right);

    */
    unsigned int t1; //For keeping track of time
    unsigned int t2; //For keeping track of time

    pwm_set_gpio_level(LEFTMOTOR_PWM, 40); // Set duty to 40 to get the car started
    pwm_set_gpio_level(RIGHTMOTOR_PWM, 40);


    int history = 40;
    int mode = 0;
    while (true) {
        // uncomment these and printImage() when testing with python 
        //char m[10]; //Python testing
        //scanf("%s",m); //Python testing

        setSaveImage(1);
        while(getSaveImage()==1){}
        convertImage();
        int com_lower = (findLine(3 * IMAGESIZEY/4) - 40); // calculate the position of the center of the line
        int com_upper = (findLine(1 * IMAGESIZEY/4) - 40); // calculate the position of the center of the line
        // int com_middle = findLine(IMAGESIZEY/2); // calculate the position of the center of the line
        
        /*
        com_middle = (((com_middle - 40) + 2 * (history - 40)) / 3 ) + 40;
        history = com_middle;
        */
        float com_avg = (com_lower + com_upper) / 2; // avg of the center

        // For testing with python or screen
        // setPixel(IMAGESIZEY/2,com,0,255,0); // draw the center so you can see it in python
        // Not necessary if not testing it with python, but then we need to add some delay
        // if we're testing with screen
        // sleep_ms(50);
        // printImage(); // PythonTesting (Comment if not using python)

        float turn_speed = min_speed;

        // Version with the upper and lower
        float com_dir = com_upper - com_lower; // >0 means right, <0 means left
        mode = 0;
        if ((abs(com_dir) > DEADZONE)){
            mode = 1;
            if (com_dir < 0){
                if (-com_dir > ERROR_MARGIN){
                    RightDuty = max_speed;
                    LeftDuty = -max_speed;
                }
                else{
                    turn_speed = (min_speed + (max_speed - min_speed) * ((-(com_dir + DEADZONE)) / (ERROR_MARGIN - DEADZONE)));
                    RightDuty = max_speed;
                    LeftDuty = -turn_speed;                
                }
            }
            if (com_dir > 0){
                if (com_dir > ERROR_MARGIN){
                    LeftDuty = max_speed;
                    RightDuty = -max_speed;
                }
                else{
                    turn_speed = (min_speed + (max_speed - min_speed) * ((com_dir - DEADZONE) / (ERROR_MARGIN - DEADZONE)));
                    LeftDuty = max_speed;
                    RightDuty = -turn_speed;     
                }
            }
        }
        else if (abs(com_avg) > DEADZONE){
            mode = 2;
            if (com_avg < 0){
                if ((-com_avg) > ERROR_MARGIN){
                    RightDuty = max_speed;
                    LeftDuty = min_speed;
                }
                else{
                    turn_speed = (max_speed - (max_speed - min_speed) * ((DEADZONE - com_avg) / (ERROR_MARGIN + DEADZONE)));
                    RightDuty = max_speed;
                    LeftDuty = turn_speed;                
                }
            }
            if (com_avg > 0){
                if (com_avg > ERROR_MARGIN){
                    LeftDuty = max_speed;
                    RightDuty = min_speed;
                }
                else{
                    turn_speed = (max_speed - (max_speed - min_speed) * ((com_avg - DEADZONE) / (ERROR_MARGIN + DEADZONE)));
                    LeftDuty = max_speed;
                    RightDuty = turn_speed;     
                }
            }
        }
        else {
            mode = 3;
            LeftDuty = max_speed;
            RightDuty = max_speed;
        }

        // Version with just a middle one
        /*
        if (com_middle < (40 - DEADZONE)){
            t1 = to_us_since_boot(get_absolute_time());
            if (com_middle < max_error_left){
                LeftDuty = -max_speed;
                RightDuty = max_speed;
            }
            else{
                turn_speed = (min_speed + (max_speed - min_speed) * ((40 - com_middle) / ERROR_MARGIN));
                RightDuty = turn_speed;
                LeftDuty = -turn_speed;
            }
        }
        else if (com_middle > (40 + DEADZONE)){
            t1 = to_us_since_boot(get_absolute_time());
            if (com_middle > max_error_right){
                RightDuty = - max_speed;
                LeftDuty = max_speed;
            }
            else{
                turn_speed = (min_speed + (max_speed - min_speed) * ((com_middle - 40) / ERROR_MARGIN));
                LeftDuty = turn_speed;
                RightDuty = -turn_speed;
            }
        }
        else {
            t2 = to_us_since_boot(get_absolute_time());
            if (t2 - t1 > 4000000){ // However long the loops take
                LeftDuty = max_speed;
                RightDuty = max_speed;
            }
            else{
                LeftDuty = 20 + (max_speed - 20) * ((t2 - t1) / 4000000);
                RightDuty = 20 + (max_speed - 20) * ((t2 - t1) / 4000000);
            }
        }
        */


        
        printf("Line COMs (dir, avg): %0.1f %0.1f\r\n",com_dir, com_avg); // comment this when testing with python
        printf("Line COMS (lower, upper): %d %d\n", com_lower, com_upper);
        printf("Mode: %d\n", mode);
        printf("Current Duty Cycle\n");
        printf("Left Duty: %0.1f\n", LeftDuty);
        printf("Right Duty: %0.1f\n", RightDuty);
        

        //set_motors(wrap);

    }
}


void set_motors(uint16_t wrap){

    if (LeftDuty < 0){
        pwm_set_gpio_level(LEFTMOTOR_PWM, wrap * (-LeftDuty) / 100); // Set duty cycle as percentage of the wrap
        gpio_put(LEFTMOTOR_PHASE, 1);
    }
    else {
        pwm_set_gpio_level(LEFTMOTOR_PWM, wrap * LeftDuty / 100); 
        gpio_put(LEFTMOTOR_PHASE, 0);
    }
    if (RightDuty < 0){
        pwm_set_gpio_level(RIGHTMOTOR_PWM, wrap * (-RightDuty) / 100); 
        gpio_put(RIGHTMOTOR_PHASE, 1);
    }
    else {
        pwm_set_gpio_level(RIGHTMOTOR_PWM, wrap * RightDuty / 100); 
        gpio_put(RIGHTMOTOR_PHASE, 0);
    }
}

