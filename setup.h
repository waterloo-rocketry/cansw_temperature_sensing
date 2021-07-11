#ifndef LEDS_H
#define	LEDS_H

#include <xc.h>

// LED macros
#define WHITE_LED LATA3
#define BLUE_LED LATA4
#define RED_LED LATA5

// function to initialize the general inputs and outputs like LEDs
void pin_init(void);

// function to initialize the external oscilator
void osc_init(void);

#endif
