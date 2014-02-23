/* 
 * File:   ASCT_Tapped_Out.ino
 * Author: asctSoftware
 *
 * Created on 18 February 2014, 1:12 AM
 * Version 1.0
 * 
 * Tapped out is an ardcore pattern/clock module. See manual for details.
 * See user_settings.h for user adjustable variables
 *
 */

// SETUP STUFF
#include <avr/io.h>
#include "user_settings.h"
#include "helper.h"
#include "clock.h"
#include "EEPROM_readwrite.h"
#include "adc.h"




void setup() {
    //ins and outs
    DDRD = DDRD | B11111000;
    DDRB = B111111;
    PORTD = B00000000;
    PORTB = B000000;

    interruptSetup(); //in clock.h
    adcSetup(); //in adc.h
    bufferSetup();//in clock.h

#ifdef EEPROMreset
    sets = defaultSettings(); //fill the settings array with some default values
    EEPROM_reset(); //reset the eeprom with the default values
#else
#ifdef SDEFAULTS
    sets = defaultSettings();
#else
    int pos = 0 + EEPROMoffset; //where to read
    eeprom_read_block((void*) &sets, (void*) pos, sizeof (sets));//read the preset into sets
#endif
#endif

#ifdef LOADONLY
    sets.pVal[1] = 1;
#endif
    locks = 0xff;//lock our controls to start
    
}

void loop() {
    if (clk)
        clkHandler(); //do clock handling when clock high
    readKnob(); //get our knob inputs 
    if (cReset || cSync)
        syncHandler();//handle the sync
    //do we need to read or write to the EEPROM
    if (psLOCKcheck() == false)
	   EEPROM_handler();
}
