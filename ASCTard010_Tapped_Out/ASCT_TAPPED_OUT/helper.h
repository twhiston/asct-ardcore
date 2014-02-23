/* 
 * File:   helpers.h
 * Author: asctSoftware
 *
 * Created on 18 February 2014, 1:12 AM
 * Version 1.0
 *
 * Helper functions, settings structures and setup
 */

#ifndef HELPERS_H
#define	HELPERS_H

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
//8 bit integer division routines for cycle saving funtimes
#define U8REC1(A, M, S) (uint8_t)( (((uint16_t)(A) * (uint16_t)(M)) >> 8u) >> (S) )
#define U8REC2(A, M, S) (uint8_t)( (((((uint16_t)(A) * (uint16_t)(M)) >> 8u) + (A)) >> 1u) >> (S) )

//This is our settings structure, it stores all the data we need as a 'preset'
typedef struct Settings {
    unsigned char sVal[4]; //settings values: output select, knob layer, master tempo, clk mode
    unsigned char aVal[10][2][2]; //ten outputs, 2 layer dependant layers of settings (pattern and speed/shuffle settings), 2 knobs per layer(pattern,pattern speed/shuffle freq, shuffle amount)
    unsigned char pVal[2]; //preset values
    unsigned char gChan; //channel to push user defined modulation to
    unsigned char gMod; //the modulation value
    unsigned char gTemp; //temp val we need for the global modulation
    unsigned long *P[10]; //our array of patterns, each one is a POINTER to a long in the pattern table
} Settings;

static Settings sets; //create a settings structure in the heap

/*
 * This sets up some default settings for your currently loaded preset structure.
 * It is called if you define SDEFAULTS or EEPROMreset in the user_settings file
 * It returns a settings stuct as defined above
 */
struct Settings defaultSettings() {
    Settings a;
    //settings values
    a.sVal[0] = 0; //output select
    a.sVal[1] = 0; //knob layer
    a.sVal[2] = 127; //master tempo
    a.sVal[3] = 0; //clkmode
    //outputs channels settings
    for (uint8_t i = 10; i--;) {
	   a.aVal[i][0][0] = 0; //pattern select
	   a.aVal[i][0][1] = 13; //pattern speed 
	   a.aVal[i][1][0] = 0; //shuffle freq
	   a.aVal[i][1][1] = 0; //shuffle amount
    }
    a.pVal[0] = 0; //selected preset
    a.pVal[1] = 1; //preset action (in this case do nothing)
    //global modifiers
    a.gChan = 4;
#ifdef user_PATTERN
    a.gTemp = 0;
#endif    
#ifdef user_SPEED
    a.gTemp = 13;
#endif
    //reset all active patterns to pointer to pattern 0
    for (uint8_t i = 0; i < 10; i++) {
	   a.P[i] = &patterns[0];
    }
    return a;
}

/*
* You wont see these used in the sketch but if you want to print out a 32bit int in a binary form these functions are very useful
* Left in incase they might be useful to you
*/
//void printBits(unsigned long b, int maxi) {
//    for (int a = maxi - 1; a--;) {
//	Serial.print(bitRead(b, a));
//    }
//    Serial.println();
//}
//
//void printBitsF(unsigned long b, int maxi) {
//    Serial.println("Play ordered: ");
//    for (int a = 0; a <= maxi - 1; a++) {
//	Serial.print(bitRead(b, a));
//    }
//    Serial.println();
//}

#endif	/* HELPERS_H */

