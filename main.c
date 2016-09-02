/*
 * File:   main.c
 * Author: eug
 *
 * Created on 02 September 2016, 11:28 AM
 */

//configuration bits will go here

#include <xc.h>
#include <stdio.h>
#include <stdarg.h>

/*
 * setup routines come here.
 * e.g setting up the timers, adc, etc.
 */
void init_uart(void) {
    SPBRG = 0x19; // 9600 baud @ 4 MHz
    TXEN = 1; // enable transmitter
    BRGH = 1; // select high baud rate
    SPEN = 1; // enable serial port
    CREN = 1; // enable continuous operation
}
void init_adc(void) {
    
}
void init_counter(void) {
    
}
/*
 * Overwrite putch, so that calling
 * printf() will print to the USART
 */
void putch(unsigned char data) {
    while(!TXIF) //check buffer
        continue; //wait till ready
    TXREG  =data; //send data
}

/*
 * main loop contains the loop that will repeat forever,
 * so all code goes in here.
*/
void mainloop(void) {
    printf("Hello, World!\n");
}

void main(void) {
    //call all init routines
    init_uart();
    init_adc();
    init_counter();
    //loop
    for(;;)
        mainloop();
    return;
}
