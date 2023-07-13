#pragma once


// from LEDs are awesome fb group:
// The LED current on WS2815 (and equivalent 12V pixels of this architecture) is around 15mA. 
// Overhead for powering the control chip is a bit under 2mA. So total current consumption is 
// less than 20mA but it's not a bad number to use for planning purposes here.

// 12v LEDs draw ~0.02A at full power, but they also draw power equal to the max of the 3 channels.
// instead of the sum.
// at full brightness (0.02A/ LED), 500 LEDs consume 10A.
// that means about 1.5k will draw 30A.
// we can probably hit 70-80% load safely since this isn't a bottom of the barrel power supply.
// so maybe 1-1.2k LEDs lit all the way.
// we got 1100 so we're good. 1200 if i ever get that last strip.
// anecdotally, people in that group tend to do about 1200 on one of these power supplies. 

// we're now up to ~2100 LEDS though. and usually running on a LiFePO4 battery.
// well we're drawing 2400 pixels but i actually own about 2100 at this point.
//#define NUM_LEDS 100
#define NUM_LEDS 300
#define NUM_PINS 8
#define ALL_LEDS NUM_LEDS * NUM_PINS

#define BRIGHTNESS 255

#define FRAMES_PER_SECOND 60

// we can also use this functionality as a backup. note that 24 amps would drain one of the big boys in about 4 hours.
#define VOLTS 12
#define MAX_MA 24000

// dunno what this really means 
#define DATA_RATE 12


#define ADVANCE_RATE_HZ 20
#define FADE_RATE 13

#define PALETTE_DURATION_S 42

// pattern stuff



// some support size
#define COMPACT_WIDTH 10

// movement speed for slide compact palette. higher is slower.
#define SLOWNESS 3


// number of trails per pin
// we've basically doubled the number of LEDs so i knocked this down from 8 to 4
#define NUM_TRAILS 4

// number of random windows per pin
#define NUM_WINDOWS 4


#define LO_PAUSE 0.5 * FRAMES_PER_SECOND
#define HI_PAUSE 3 * FRAMES_PER_SECOND

#define MAX_WIDTH 10
