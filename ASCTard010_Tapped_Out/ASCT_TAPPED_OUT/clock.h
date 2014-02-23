/* 
 * File:   clock.h
 * Author: asctSoftware
 *
 * Created on 18 February 2014, 1:12 AM
 * Version 1.0
 *
 * This holds most of the clock related functions, setup ISR's etc.....
 *
 */

#ifndef CLOCK_H
#define	CLOCK_H

#include <avr/interrupt.h>
#include <Arduino.h>  // for type definitions

#define sBSize 4
#define targetPPQ 24

//Functional Declarations
void interruptSetup();
void bufferSetup();
void clkHandler();
void syncHandler();
void bufferSetup();
void bSmooth();
void bCalculate();
void mTempoTicks(long input);
void mTempoSt(int input = 255);
void resetSyncVals();

unsigned long rBuff; //the current rollover
unsigned int volatile clockCount = 6144; //our current count
unsigned int volatile pPointer[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0}; //current play position of each pattern
unsigned int volatile shuffle[10]; //hold our shuffle count
volatile long cRollover = 0; //this is how many ticks between first tick and next

//output values
byte pB = B0; //holding bay port B
byte pD = B0; //port D
byte output = B0; // temp binary output value

//FLAGS
volatile boolean cReset = LOW;
volatile boolean cSync = LOW;
boolean csStarted = LOW; //is it recording ticks now?
boolean sMark = LOW; //not volatile as not accessed outside ISR
boolean volatile clk = LOW;//high on internal clock interrupt

//our buffer
typedef struct Buffer {
    float period;
    unsigned long buff[sBSize];
    unsigned int pos;
    unsigned long count;
} Buffer;
static Buffer buff;

void interruptSetup() {
    // initialize Timer1
    cli(); // disable global interrupts
    TCCR1A = 0; // set entire TCCR1A register to 0
    TCCR1B = 0;
    // Phase and Freq correct PWM, top = OCR1A
    //value for OCR1A is set via mTempoSt() in sketch setup
    // TCCR1B |= (1 << WGM12);
    TCCR1A |= (1 << WGM10);
    //  TCCR1A |= (1 << WGM11);
    TCCR1B |= (1 << WGM13);
    // Set CS11 for 8 prescaler:
    TCCR1B |= (1 << CS11);
    // enable timer compare interrupt:
    TIMSK1 |= (1 << OCIE1A);
    //external interrupt 0 on rising edge
    sbi(EICRA, ISC00);
    sbi(EICRA, ISC01);
    //enable external interrupt on pin2 
    sbi(EIMSK, INT0);
    // enable global interrupts:
    sei();
}

void bufferSetup() {
    mTempoSt(60); //set start tempo 120bpm (60bpm+60bpm's = 120bpm)
    for (int i = 0; i < sBSize; i++) {
        buff.buff[i] = 48;
    }
}

//internal interrupt for our master clock
ISR(TIMER1_COMPA_vect) {
    //clock count and sync
#ifdef HARDSYNC
 if(sets.sVal[3] == 1)
    return;
#endif
    if (csStarted == HIGH)
	   cRollover++; //deal with sync
    clk = HIGH;
}

//external interrupt for sync or reset actions
ISR(INT0_vect) {
#ifdef HARDSYNC
  unsigned int static hDivCount;
    if(sets.sVal[3] == 1){
        if(hDivCount == hDiv-1){
            clk = HIGH; 
            hDivCount = 0;
        } else {
            hDivCount++;
        }
        return;
    }
#endif
    (sets.sVal[3] == 0) ? cReset = HIGH : cSync = HIGH;
}

void clkHandler() {
    (clockCount > 1) ? clockCount-- : clockCount = 6144; //increase the clock unless its at min then reset 0-95 = 96ppq
    (clockCount & 0x1) ? pB &= ~(1UL << 5) : pB |= 1UL << 5; //%2 equivalent for output 13 - 24ppq
    //channel output builder
    for (int i = 10; i--;) {
	if (shuffle[i] >= sets.aVal[i][1][0] && sets.aVal[i][1][0] != 0) {
	    if (clockCount % (clkVals[sets.aVal[i][0][1]]+(sets.aVal[i][1][1])) == 0) {
		  output = 1;
		  sMark = HIGH;
	    } else {
		  output = 0;
	    }
	} else {
	    (clockCount % clkVals[sets.aVal[i][0][1]] == 0) ? output = 1 : output = 0;
	}
	if (output == 1) {
	    //if output is 1 then move on in our pattern 
	    (pPointer[i] == 0) ? pPointer[i] = 31 : pPointer[i]--;
	    output = (*sets.P[i] >> pPointer[i]) & 0x01;
	    if (output == 1 && sMark == LOW) {
		  shuffle[i]++;
	    } else if (output == 1 && sMark == HIGH) {
		  shuffle[i] = 0;
		  sMark = LOW;
	    }
	}
	(i < 5) ? (output ? (pD) |= (1UL << (i + 3)) : (pD) &= ~(1UL << (i + 3))) : (output ? (pB) |= (1UL << (i - 5)) : (pB) &= ~(1UL << (i - 5)));
	//equivalent= (i<5)?bitWrite(pD, i+3, output):bitWrite(pB, i-5, output);
    }
    //write the outputs
    PORTD = pD;
    PORTB = pB;
    clk = LOW;
}

void syncHandler() {
    if (cReset) {
    	cli();
    	for (int i = 10; i--;) {
    	    pPointer[i ] = 0;
    	    shuffle[i] = 0;
    	}
    	clockCount = 0;
    	cReset = LOW;
    	sei();
    	return;
    }//do counter reset - WORKING
    if (cSync) {
    	//save the rollover and put it back to 0 asap for the counter to fill
    	rBuff = cRollover;
    	cRollover = 0;
    	//if the input is 48 pulses then its accurate to the current clock
    	if (csStarted == LOW) {
    	    csStarted = HIGH;
    	    for (int i = 0; i < sBSize; i++) {
    		  buff.buff[i] = 48;
    	    }
    	} else if (csStarted == HIGH) {
    	    //write to the buffer
    	    buff.buff[buff.pos] = rBuff;
    	    bSmooth();
    	    bCalculate();
    	    (buff.pos < sBSize - 1) ? buff.pos++ : buff.pos = 0;
    	}
    	cSync = LOW;
    }//sync the clock
}

//some smoothing for the sync calculations
void bSmooth() {
    //if the last value was a lot smaller than the current one reset the array to this value
    int lastpos = 0;
    (buff.pos == 0) ? lastpos = sBSize - 1 : lastpos = buff.pos - 1;
    if (buff.buff[buff.pos] > buff.buff[lastpos] + 10 || buff.buff[buff.pos] > buff.buff[lastpos] - 10) {
    	for (int i = sBSize; i--;) {
    	    buff.buff[i] = buff.buff[buff.pos];
    	}
    }
    int adder = 0;
    for (int i = sBSize; i--;) {
	   adder += buff.buff[i];
    }
    int divo = adder / sBSize;
    buff.buff[buff.pos] = divo;
    return;
}

/*
 * This does some sync calculations for the non hardsync sync implementation. Its still not ideal but if theres a better way to sync
 * whilst keeping the patterns in time let me know your thoughts!
 */
void bCalculate() {
    int static varianceCount;
    float total = 0;

    for (int i = sBSize; i--;) {
	   total += buff.buff[i];
    }

    float totaldiv = 0;
    totaldiv = total / (float) sBSize;
    //if our div is already correct exit
    if (floor(totaldiv + 0.5) == floor(buff.period + 0.5))
	   return;

    buff.period = totaldiv;
    float divVal = sBSize * (float) (targetPPQ * 2);
    total = total / divVal; //no way to optimize this line!!!!
    unsigned int cExtTemp = floor((buff.count * total) + 0.5);

    if (cExtTemp == buff.count)
	   return;

    if (varianceCount < varianceMax) {
    	if (cExtTemp > buff.count - variance && cExtTemp < buff.count) {
    	    varianceCount++;
    	    return;
    	}
    	if (cExtTemp < buff.count + variance && cExtTemp > buff.count) {
    	    varianceCount++;
    	    return;
    	}
    } else {
    	//this is a reset to account for variance
    	buff.count = cExtTemp;
    	cli();
    	OCR1A = buff.count;
    	clockCount = 0;
    	sei();
    	varianceCount =  0;
    	return;
    }
    //this is a reset where the input has changed significantly
    buff.count = cExtTemp;
    cli();
    OCR1A = buff.count;
    clockCount = 0;
    sei();
    varianceCount =  0; //if we get this far its changes sufficiently to reset the variance
    return;
}

/*
 * Set the master tempo
 * input is how many bpm to add to the root, which is 60
 * 
 *if 0 = 60bpm & knob range is 0-255
 * 
 * 16000000 / 8 / (x + 1) / 2 / 2 =  128.04 HZ
 *  
 * 255 = 1hz + (255*0.017)
 * 255 = 1hz + 4.335hz
 * 255 = 5.335hz
 * 
 * 5.335*24 = 128.04
 * 
 * 500000/128.04 = 3905.02967822555451
 * 
 * x+1 = 3905
 * x = 3904
 *
 * we can verify with knob at 255 128.35/6/7 is produced on the 24ppq output 13
 * so there is a margin of error around 0.3hz  
 */
void mTempoSt(int input) {
   // one bpm = 0.017hz
    double newcount = (1 + ((double) input * 0.017))*24;
    newcount = (500000 / newcount) - 1;
    buff.count = floor(newcount + 0.5); //do some sensible scaling rounding
    cli();
    OCR1A = buff.count;
    sei();
}

/*
 * Set the Tempo in raw ticks & update sync buffer to reflect this
 */
void mTempoTicks(long input) {
    if (input < 2)
	   return;
    OCR1A = input;
    buff.count = input;
}

/*
 * Resets all the values necessary to do a full clock reset
 */
void resetSyncVals() {
    cSync = LOW; //sync value low
    csStarted = LOW; //if we change out of sync mode stop it recording rollover
    cRollover = 0; //reset the rollover counter so when we switch back it works
}

#endif	/* CLOCK_H */
