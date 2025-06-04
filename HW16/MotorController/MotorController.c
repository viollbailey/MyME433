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


//defining the motor pins
#define LEFTMOTOR_PWM 18
#define LEFTMOTOR_PHASE 19
#define RIGHTMOTOR_PWM 20
#define RIGHTMOTOR_PHASE 21

static int LeftDuty = 0; // Just defining duty cycle up here cause it's easier
static int RightDuty = 0;
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

    /* PWM controls (Keep commented out)
    // Setting up PWM control for the RC motor
    gpio_set_function(PWMPin, GPIO_FUNC_PWM); // Set the LED Pin to be PWM
    uint slice_num = pwm_gpio_to_slice_num(PWMPin); // Get PWM slice number
    float div = 50; // must be between 1-255
    pwm_set_clkdiv(slice_num, div); // divider set
    uint16_t wrap = 60000; // when to rollover, must be less than 65535
    // Sys clock is 150MHz, so PWM freq = 150 MHz / wrap / divider
    // in our case we want a 50Hz PWM signal for the servo so 150 MHz/ (50 * 60000) = 50
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true); // turn on the PWM
  
    pwm_set_gpio_level(PWMPin, wrap / 40); // Set duty cycle as percentage of the wrap
    //Duty cycle needs to be between 2.5% and 12.5% for the servo
    */

   /* Phase controls (Keep commented out)
    gpio_init(LEDOUTPIN_NUM);
    gpio_set_dir(LEDOUTPIN_NUM, GPIO_OUT);
    gpio_put(PIN_NUM, 1); for output pin on
    */

    // Initializing the motor pins
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

    // Wait until the screen program is open
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Hello, Motors!\n");

    while(1){
        
        // Communicating with the computer
        printf("Current Duty Cycle\n");
        printf("Left Duty: %d\n", LeftDuty);
        printf("Right Duty: %d\n", RightDuty);

        printf("Enter Values (L_PWM R_PWM)");
        scanf("%d %d", &LeftDuty, &RightDuty);
        

        set_motors(wrap);

    }
    /* ADC stff
    adc_init(); // init the adc module
    adc_gpio_init(ADCPIN_NUM); // Set ADC0 as adc input, note that it's GP# NOT pin#
    So 26 is ADC0 (right above the run pin)
    adc_select_input(0); // Select to read ADC0
    */


    return 0;
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
