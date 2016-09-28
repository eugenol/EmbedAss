/*
 * File:   main.c
 * Author: eugenol
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
#define _XTAL_FREQ 4000000L //FOSC

//INCLUDES
#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

//Global variables
unsigned long TMR1_counter=0;  // Count overflow from interrupt.
unsigned long TMR0_counter=0;  // Count overflow from interrupt.
// Flags - bit data type to save space
bit time_done = 0;              // Do last fraction of overflow count.
bit transmit_data = 0;          // data ready to send?
// Frequency Counter values.
unsigned char TMR1L_val;        // Store Timer 1 low byte.
unsigned char TMR1H_val;        // store Timer 1 high byte.
unsigned long TMR1_ovfl_val = 0; // store timer 1 overflows - multiples of 65536.

//function declarations
int get_adc();//returns a number between 0 and 1023

/*
 * setup routines come here.
 * e.g setting up the timers, adc, etc.
 */
void init_ports(void) {
    TRISA = 0xFF;   //set all digital I/O to inputs
    TRISB = 0xFF;
    TRISC = 0xFF;
  
    ANSEL = 0x00;   //disable all analog ports
    ANSELH = 0x00;

    TRISAbits.TRISA2 = 1;   //Disable the output driver for pin RA2/AN2
    ANSELbits.ANS2 = 1;     //set RA2/AN2 to analog mode
}

void init_uart(void) {
    SPBRG = 0x19;   // 9600 baud @ 4 MHz
    TXEN = 1;       // enable transmitter
    BRGH = 1;       // select high baud rate
    SPEN = 1;       // enable serial port
    CREN = 1;       // enable continuous operation
}

void init_adcs(void) {
   
    ADCON0bits.ADFM = 1;        //ADC result is right justified
    ADCON0bits.VCFG = 0;        //Vdd is the +ve reference
    ADCON1bits.ADCS = 0b001;    //Fosc/8 is the conversion clock
                                //This is selected because the conversion
                                //clock period (Tad) must be greater than 1.5us.
                                //With a Fosc of 4MHz, Fosc/8 results in a Tad
                                //of 2us.
    ADCON0bits.CHS = 2;         //select analog input, AN2
    ADCON0bits.ADON = 1;        //Turn on the ADC
}

void init_interrupts(void) {
    T1CONbits.TMR1ON = 1;       //Enable Timer 1
    T1CONbits.TMR1CS = 1;       //Choose clock source (External clock)
    T1CONbits.nT1SYNC = 1;      //Do not synchronise external clock input

    OPTION_REGbits.PSA = 1;
    OPTION_REGbits.T0CS = 0;
}

void start_count(void) {
    INTCONbits.GIE = 0;          // Disable All interrupts.

    TMR1H = 0;                   // Clear Timer1 high count.
    PIR1bits.TMR1IF = 0;         // Clear Interrupt Flag.
    PIE1bits.TMR1IE = 1;
    TMR1L = 0;                   // Clear Timer1 low count.

    TMR1_counter = 0;        // Clear overflow counter.
    TMR0_counter=0;         // Clear overflow counter.

    TMR0 = 0;               // Reset Timer0 value.
    INTCONbits.T0IF = 0;      // Clear Timer0 interrupt flag
    INTCONbits.T0IE = 1;      // Enable Timer0 interrupt.

    INTCONbits.PEIE = 1;        // Enable All Extended interrupt
    INTCONbits.GIE = 1;         // Enable All interrupts.
}

void interrupt isr() {
    // Timer/Counter 1 Interrupt
    if (PIR1bits.TMR1IF) {
        TMR1_counter++;         //increment overflow count
        PIR1bits.TMR1IF = 0;    //clear timer 1 interrupt flag
    }
    // Timer/counter 0 interrupt
    if (INTCONbits.T0IF) {
        TMR0_counter++;         //overflow, so increment count

        if(time_done) { // The last count is not a full overflow.

            TMR1L_val = TMR1L; // Capture counter values of low byte.
            TMR1H_val = TMR1H; // Capture counter values of low byte.
            TMR1_ovfl_val = TMR1_counter; // capture number of overflows
            transmit_data = 1;  // data ready to transmit
            time_done=0;        //clear flag

        } else {

           if(TMR0_counter==3906) { // 4MHz Capture the input count.
                TMR0 = 256-64+2; // 2 cycles lost when writing to TMR0 so add 2.
                time_done=1;     //  set flag
           }
        }
        INTCONbits.T0IF = 0; //clear timer 0 interrupt flag
    } // End TMR0IF
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

void main(void) {
    
    unsigned long freq = 0;  // display frequency value to use.
    int adc_val = 0;
    //call all init routines
    init_ports();
    init_uart();
    init_adcs();
    init_interrupts();

    start_count();
    
    // main loop
    for(;;){
        // check flag and transmit if ready
        if(transmit_data) {
            // store freq value in long
            freq = TMR1L_val+(TMR1H_val<<8)+(TMR1_ovfl_val<<16); 
            start_count(); // start freq counter again
            adc_val = get_adc(); // read adc

            printf("%lu,%d\r",freq,adc_val); //send data to serial port  
            transmit_data = 0;
        }
    }
}

int get_adc(void){
      __delay_us(5);              //Wait the acquisition time (about 5us).

    ADCON0bits.GO = 1;          //start the conversion
    while(ADCON0bits.GO==1);  //wait for the conversion to end
    
    return (ADRESH<<8)+ADRESL;//returns the ADC reading
}