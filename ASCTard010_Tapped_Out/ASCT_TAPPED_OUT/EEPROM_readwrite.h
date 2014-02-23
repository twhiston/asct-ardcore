/* 
 * File:   EEPROM_readwrite.h
 * Author: asctSoftware
 *
 * Created on 18 February 2014, 1:12 AM
 * Version 1.0
 *
 * This reads and writes settings values structs to the EEPROM, it handles the size of the structs itself.
 * Saving causes a momentary pause in pattern output
 */

#ifndef EEPROM_READWRITE_H
#define	EEPROM_READWRITE_H

#include <avr/eeprom.h>

unsigned int const memSize = 1024; //max size of memory, so we dont write outside of real values
boolean psLOCK = true;

//check to see that we can write according to allowed writes and memsize
bool EEPROM_isWriteOk(unsigned int address) {
    unsigned int static writeCounts;//we use this to prevent writes after a certain number to save EEPROM life, number defined in user_setting.h
	writeCounts++;
	if (allowedWrites == 0 || writeCounts > allowedWrites)
	    return false;
	if (address > memSize)
	    return false;
	return true;
}

//check the read address is within the memory size
bool EEPROM_isReadOk(unsigned int address) {
  if (address > memSize) 
    return false;
  return true;
}

// check and set for write lock
void psLOCKset(boolean value) {
	psLOCK = value;
}

boolean psLOCKcheck() {
	return psLOCK;
}

//write or read the settings from the EEPROM
void EEPROM_handler() {
	if (sets.pVal[1] == 3) {
	    unsigned int pos = (sets.pVal[0] * sizeof (sets)) + EEPROMoffset; //where to write
	    if (EEPROM_isWriteOk(pos) == true) {
			eeprom_write_block((const void*) &sets, (void*) pos, sizeof (sets));
			psLOCKset(true);
	    } else {
			psLOCKset(true); //writes exceeded
	    }
	} else if (sets.pVal[1] == 0) {
	    unsigned int pos = (sets.pVal[0] * sizeof (sets)) + EEPROMoffset; //where to read
	    if (EEPROM_isReadOk(pos) == true) {
                        unsigned char tempp0 = sets.pVal[0];
                        unsigned char tempp1 = sets.pVal[1];
			eeprom_read_block((void*) &sets, (void*) pos, sizeof (sets));
                        sets.pVal[0] = tempp0;
                        sets.pVal[1] = tempp1;
	    }
	    psLOCKset(true);
	} else {
            //Serial.println("DO NOTHING AND LOCK");
	    psLOCKset(true); //if do nothing lock this out as we dont need to waste cycles
	}
}

//writes the arduino EEPROM to 0
void EEPROM_reset() {
	for (int i = 0; i < 8; i++) {
	    int a = (i * sizeof (sets)) + EEPROMoffset;
	    eeprom_write_block((const void*) &sets, (void*) a, sizeof (sets));
	}
}

#endif	/* EEPROM_READWRITE_H */

