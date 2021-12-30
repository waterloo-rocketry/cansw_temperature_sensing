#include "setup.h"

void pin_init(void){
    // LEDs
    TRISA3 = 0; // set C5 as an output for the white LED
    ANSELA3 = 0; // Enable digital input buffer (Useful for reading the LED state)
    LATA3 = 1; // turn the white LED off

    TRISA4 = 0; // set C6 as an output for the blue LED
    ANSELA4 = 0; // Enable digital input buffer
    LATA4 = 1; // turn the blue LED off

    TRISA5 = 0; // set C7 as an output for the red LED
    ANSELA5 = 0; // Enable digital input buffer
    LATA5 = 1; // turn the red LED off

    // conversion complete pins
    // board 1 interrupt is B3
    TRISB3 = 1; // set as input
    ANSELB3 = 0; // enable digital input buffer

    // board 2 interrupt is B2
    TRISB2 = 1; // set as input
    ANSELB2 = 0; // enable digital input buffer

    // board 3 interrupt is B5
    TRISB5 = 1; // set as input
    ANSELB5 = 0; // enable digital input buffer

    // CS pins
    // board 1 on C4
    TRISC4 = 0; // set C4 as an output
    ANSELC4 = 0; // Enable digital input buffer
    LATC4 = 1; // set high (since CS uses inverted logic)

    // board 2 on C6
    TRISC6 = 0; // set C6 as an output
    ANSELC6 = 0; // Enable digital input buffer
    LATC6 = 1; // set high (since CS uses inverted logic)

    // board 3 on B0
    TRISB0 = 0; // set B0 as an output
    ANSELB0 = 0; // Enable digital input buffer
    LATB0 = 1; // set high (since CS uses inverted logic)
}

void osc_init(void){
    // Select external oscillator with PLL of 1:1
    OSCCON1 = 0b01110000;
    // wait until the clock switch has happened
    while (OSCCON3bits.ORDY == 0)  {}
    // if the currently active clock (CON2) isn't the selected clock (CON1)
    if (OSCCON2 != 0b01110000) {
        // something is broken, maybe if we turn it off and on again?
        RESET();
    }
}