/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

 #include <stdio.h>
 #include "pico/stdlib.h"

 
 int main() {
     stdio_init_all();

     // Wait forever
     while (1){
        printf("Hello World!");
        sleep_ms(1000);
     };
}
 