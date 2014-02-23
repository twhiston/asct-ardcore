/* 
 * File:   adc.h
 * Author: asctSoftware
 *
 * Created on 18 February 2014, 1:12 AM
 * Version 1.0
 *
 * 8-bit reading direct from mux without analogRead();
 * Handles reading inputs, assigning them to correct parts of the settings array and applying those values
 * A proportion of this code is decided at compile time so if you want to alter it in any way make sure your clear what options are defined in user_settings.h
 * 
 */
 
#ifndef ADC_H
#define	ADC_H
    
unsigned int locks = 0; //we use this to hold a lock bit for each control. If locks are not defined then this stays at 0 and it all still works
byte aIn = 0; //selected analog in, changed at the end of each pass through the knob read function
byte readIn; //clkin vector count

// Set up all the adc bits, called in setup()
void adcSetup() {
	//clear ADCSRA and ADCSRB registers
	ADCSRA = 0; //we set this up again lower down
	ADCSRB = 0; //bit6 turns on multiplex the rest turn on ref signal, this disables everything
	sbi(ADMUX, REFS0); //0 1 AREF, Internal Vref AVCC
	sbi(ADMUX, ADLAR); //Bit 5 left align the ADC value- so we can read highest 8 bits from ADCH register only
	sbi(ADCSRA, ADPS2); //set ADC clock with 128 prescaler- 16mHz/128=125kHz
	sbi(ADCSRA, ADPS1); //^
	sbi(ADCSRA, ADPS0); //^
	sbi(ADCSRA, ADIE);
	sbi(ADCSRA, ADATE); //Bit 5 – ADATE: ADC Auto Trigger Enable When this bit is written to one, Auto Triggering of the ADC is enabled. The ADC will start a con- version on a positive edge of the selected trigger signal. The trigger source is selected by setting the ADC Trigger Select bits, ADTS in ADCSRB.
	sbi(ADCSRA, ADEN); //Bit 7 Writing this bit to one enables the ADC.
	sei();
	sbi(ADCSRA, ADSC); //Bit 6 In Free Running mode, write this bit to one to start the first conversion.
}

//Interrupt vector for clk in, advances the read count
ISR(ADC_vect) {
	readIn++;
}


void muxChange(){
              //This is the bit where we change the input knob that we are reading, note that this is still inside the settle time condition
	    (aIn < 5) ? ADMUX++ : ADMUX &= 0xF8; //change the ADMUX channel
	    (aIn < 5) ? aIn++ : aIn = 0;
	    readIn = 0;
            return;
}

/*
 * we go through this each time the main loop executes, each time it runs it changes the analog in that it reads, as long as the settleTime is passed
 * Its basically a bigt decision tree to assign the value to the right part of the currently loaded settings array
 * User controlled modifier set at compile time
 */
void readKnob() {
	if (readIn > settleTime) { //necessary because of settling time of adc, allows us to not use a delay
	    unsigned char temp = ADCH;
	    //static controls
	    if (aIn == 0) {
			int tdiv = U8REC2(temp, 0x25u, 4u); //equivalent to temp/28 but takes 8micros instead of 20!!!!
			if (tdiv != sets.sVal[0]) {
		    	sets.sVal[0] = tdiv; //set the output
		    	sets.pVal[1] = 1; //set the preset loader to neutral
#ifdef PICKUP
        		locks = 0xff;//lock all the controls for pickup
#endif
			} //0-9 for output select
                        muxChange();
                        return;
	    } else if (aIn == 1) {
			if (temp >> 6 != sets.sVal[1]) {
		    	  sets.sVal[1] = temp >> 6; //we do not need to do anything else 
		    	  sets.pVal[1] = 1;
#ifdef PICKUP
            	          locks = 0xff;//lock all the controls for pickup
#endif
			}//0-3 for layer
                        muxChange();
                        return;
	    } else if (aIn == 4) {
            if (temp != sets.gChan) {
                int tdiv = U8REC2(temp, 0x25u, 4u);
#ifdef user_PATTERN
                if (sets.aVal[sets.gChan][0][0] != sets.gTemp){
                  sets.aVal[sets.gChan][0][0] = sets.gTemp;
                  sets.P[sets.gChan] = &patterns[sets.gTemp];
                }
#endif
#ifdef user_SPEED
                sets.aVal[sets.gChan][0][1] = sets.gTemp;
#endif
                sets.gChan = tdiv; //set the mod channel
#ifdef user_PATTERN
                if (sets.aVal[sets.gChan][0][0] != sets.gTemp)
                  sets.gTemp = sets.aVal[sets.gChan][0][0];
#endif
#ifdef user_SPEED
                sets.gTemp = sets.aVal[sets.gChan][0][1];
#endif
            }
            muxChange();
            return;
        } else if ( aIn == 5 ){ 
        	//this decision is made at compile time
#ifdef user_PATTERN
        if (sets.sVal[0] != sets.gChan){
            signed int sAdd = (temp-A5middle);
            if(sAdd != 0){
               sAdd += sets.gTemp;
               if(sAdd  > 255)
                 sAdd  = 255;
               if(sAdd < 0)
                 sAdd = 0;  
              if(sets.aVal[sets.gChan][0][0] != sAdd){
                sets.aVal[sets.gChan][0][0] = sAdd;
                sets.P[sets.gChan] = &patterns[sAdd];
              }
            }
        }
        muxChange();
        return;
#endif
#ifdef user_SPEED
        if (sets.sVal[0] != sets.gChan){
              signed int sAdd = (temp-A5middle)>>3;
              sAdd += sets.gTemp;
              if(sAdd  > 23)
                sAdd  = 23;
              if(sAdd < 0)
                sAdd = 0;   
              sets.aVal[sets.gChan][0][1] = sAdd ;
        }
        muxChange();
        return;
#endif
#ifdef user_SHUFFLE_FREQ
            sets.aVal[sets.gChan][1][0] = temp >> 3;
            muxChange();
            return;
#endif
#ifdef user_SHUFFLE_AMT
            sets.aVal[sets.gChan][1][1] = temp;
            muxChange();
            return;
#endif
#ifdef user_NONE
#endif                    
		//global cv controls
	    } else {
		//ten outputs, 3 layers, 2 knobs
#ifdef PICKUP
		if (sets.sVal[1] == 0 && aIn == 2 && ((locks >> 6) & 0x01) == 1 && temp == sets.aVal[sets.sVal[0]][0][0])
		    (locks) &= ~(1UL << (6));
#endif
		if (sets.sVal[1] == 0 && aIn == 2 && ((locks >> 6) & 0x01) == 0) {
		    if (temp != sets.aVal[sets.sVal[0]][0][0]) {
				sets.aVal[sets.sVal[0]][0][0] = temp;
				sets.P[sets.sVal[0]] = &patterns[temp];
#ifdef user_PATTERN
                if (sets.sVal[0] == sets.gChan)
                                sets.gTemp = temp;
#endif
		    }
                    muxChange();
                    return;
		} //set the pattern 
#ifdef PICKUP
		if (sets.sVal[1] == 0 && aIn == 3 && ((locks >> 7) & 0x01) == 1 && temp >> 3 == sets.aVal[sets.sVal[0]][0][1])
		    (locks) &= ~(1UL << (7));
#endif
		if (sets.sVal[1] == 0 && aIn == 3 && ((locks >> 7) & 0x01) == 0) {
		    if(temp >> 3!= sets.aVal[sets.sVal[0]][0][1]){
                                unsigned char inDiv = temp / 11;
				sets.aVal[sets.sVal[0]][0][1] = inDiv;
                    
#ifdef user_SPEED
                if (sets.sVal[0] == sets.gChan)
                                sets.gTemp = inDiv;
#endif
                     }
                    muxChange();
                    return;
		} //set the pattern speed 
#ifdef PICKUP
		if (sets.sVal[1] == 1 && aIn == 2 && ((locks >> 4) & 0x01) == 1 && temp >> 3 == sets.aVal[sets.sVal[0]][1][0])
		    (locks) &= ~(1UL << (4));
#endif
		if (sets.sVal[1] == 1 && aIn == 2 && ((locks >> 4) & 0x01) == 0) {
		    if (temp >> 3 != sets.aVal[sets.sVal[0]][1][0])
				sets.aVal[sets.sVal[0]][1][0] = temp >> 3;
                    muxChange();
                    return;
		}//shuffle frequency
#ifdef PICKUP
		if (sets.sVal[1] == 1 && aIn == 3 && ((locks >> 5) & 0x01) == 1 && temp == sets.aVal[sets.sVal[0]][1][1])
		    (locks) &= ~(1UL << (5));
#endif
		if (sets.sVal[1] == 1 && aIn == 3 && ((locks >> 5) & 0x01) == 0) {
		    if (temp != sets.aVal[sets.sVal[0]][1][1])
				sets.aVal[sets.sVal[0]][1][1] = temp;
                  muxChange();
                  return;
		}//shuffle amount
#ifdef PICKUP
		if (sets.sVal[1] == 2 && aIn == 2 && ((locks >> 2) & 0x01) == 1 && temp >> 5 == sets.pVal[0])
		    (locks) &= ~(1UL << (2));
#endif
		if (sets.sVal[1] == 2 && aIn == 2 && ((locks >> 2) & 0x01) == 0 && temp >> 5 != sets.pVal[0]) {
                    temp = temp >> 5;
		    if (temp != sets.pVal[0]) {
				sets.pVal[0] = temp;

#ifdef LOADONLY
                    if(sets.pVal[1] == 0 || sets.pVal[1] == 1){
                      sets.pVal[1] = 0;
                      psLOCKset(false);
                    }
#endif
		    }
                  muxChange();
                  return;
		}//preset select
#ifdef PICKUP
		if (sets.sVal[1] == 2 && aIn == 3 && ((locks >> 3) & 0x01) == 1 && temp >> 6 == sets.pVal[1])
		    (locks) &= ~(1UL << (3));
#endif
		if (sets.sVal[1] == 2 && aIn == 3 && ((locks >> 3) & 0x01) == 0) {
		    if (temp >> 6 != sets.pVal[1]) {
 
				sets.pVal[1] = temp >> 6;
#ifdef LOADONLY
                                if(sets.pVal[1] > 1)
                                sets.pVal[1] = 1;
#endif

                                if(sets.pVal[1] != 1){
				  psLOCKset(false);
                                }
		    }
                  muxChange();
                  return;
		}//preset save/load
#ifdef PICKUP
		if (sets.sVal[1] == 3 && aIn == 2 && ((locks >> 0) & 0x01) == 1 && temp == sets.sVal[2])
		    (locks) &= ~(1UL << (0));
#endif
		if (sets.sVal[1] == 3 && aIn == 2 && ((locks >> 0) & 0x01) == 0) {
		    if(temp != sets.sVal[2]){
				sets.sVal[2] = temp;
				mTempoSt(temp);
		    }
                  muxChange();
                  return;
		}//set master tempo
#ifdef PICKUP
		if (sets.sVal[1] == 3 && aIn == 3 && ((locks >> 1) & 0x01) == 1 && temp >> 7 == sets.sVal[3])
		    (locks) &= ~(1UL << (1));
#endif
		if (sets.sVal[1] == 3 && aIn == 3 && ((locks >> 1) & 0x01) == 0) {
		    if (sets.sVal[3] != temp >> 7) {
				sets.sVal[3] = temp >> 7;
				if (sets.sVal[3] == 0)
			    	resetSyncVals();
		    }//set clock in mode reset/sync
                  muxChange();
                  return;
		}
	    }
          muxChange();
          return;
	}
}


#endif	/* ADC_H */
