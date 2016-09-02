/*
 * File:   main.c
 * Author: eug
 *
 * Created on 02 September 2016, 11:28 AM
 */

//configuration bits will go here

#include <xc.h>

//function headers
void setup(void); //set up is in here
void mainloop(void);

void main(void) {
    setup();
    for(;;)
        mainloop();
    return;
}

/*
 * setup should contain all the setup/configuration code
 * such as setting up the timers, adc, etc.
 */
void setup(void) {
    
}
/*
 * main loop contains the loop that will repeat forever,
 * so all code goes in here.
*/
void mainloop(void) {
    
}