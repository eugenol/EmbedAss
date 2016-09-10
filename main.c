/*
 * File:   main.c
 * Author: eug
 *
 * Created on 02 September 2016, 11:28 AM
 */

// PIC16F690 Configuration Bit Settings
// 'C' source line config statements
// CONFIG
#pragma config FOSC = INTRCIO   // Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA4/OSC2/CLKOUT pin, I/O function on RA5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select bit (MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown-out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.



//DEFINES
/* Using Internal Clock of 4 Mhz */
#define FOSC 4000000L
#define _XTAL_FREQ FOSC
#define testbit(var, bit) ((var) & (1 <<(bit)))
#define setbit(var, bit) ((var) |= (1 << (bit)))
#define clrbit(var, bit) ((var) &= ~(1 << (bit)))

//INCLUDES

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
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

void init_adcs(void) {
  //set the ADC on to RA2
    setbit(TRISA,2);    //set TRISA:2 (input)
    setbit (ANSEL,2);   //make it analogue
  //set the 555 on RC6
    setbit(TRISC,6);    //set TRISC:6 (input)
    clrbit(ANSELH,2);   //make it inputs digital on RC6
    ADCON0 = 0b00001001; // Analog channel select @ AN2, ADC enabled
    __delay_ms(1); //delay after starting the ADC
}

int get_adc(void){
    setbit(ADCON0,1);
    __delay_ms(1);
    while(testbit(ADCON0,1));//wait until test is done
    return ADRESH;//returns the upper 8 bits of the ADC reading
}
int get_555(void){//returns the number of ms that the system was low for
    long int counter = 0;
    while(!(testbit(PORTC,6)));//loop while port is low
    while(testbit(PORTC,6)){//wait until port goes high again
        __delay_ms(1);//delay 1 ms
        counter++;
    }
    return counter;
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
    int int_adc;
    float int_voltage;
    float ext_time;
    int ext_adc;
    int_adc = get_adc();
    ext_adc = get_555();//returns number of msec
    int_voltage = 256 * 5 / (float)int_adc; //get value as a voltage
    char time_spent[4];
    char voltage[3];
    sprintf(time_spent, "%f", int_voltage);
    sprintf(voltage, "%d", ext_adc);
    printf(time_spent);
    printf(ext_adc);
}

void main(void) {
    //call all init routines
    init_uart();
    init_adcs();
    //loop
    for(;;){
        mainloop();
        int i;
        for(i=0;0<100;i++)
            __delay_ms(10);// loop about every second
    }
    return;
}
