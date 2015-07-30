/*  ============================================================
 *
 *  Program: ASCTard013 - midi2gates
 *
 *  Description:
 *               Simple midi to gates converter for the ardcore + expander.
 *               Uses software serial and a midi to minijack cable to enable gates from the expander bit outputs
 *
 *  I/O Usage:
 *    A0: Channel
 *    A1: Clock Gate Length
 *    A2: Base note, this is the note that will trigger output on bit0, each bit output will be triggered by +1 semitone from the last
 *    A3: Clock Division value, handy clock divider to use with the clock output
 *    Digital Out 1: Clock Output
 *    Digital Out 2: Start/Stop/Continue Output
 *    Clock In: Midi In - use a midi to minijack cable
 *    Analog Out: Gates out via expander
 *
 *  Created:  29/07/2015
 *
 */


#include "Arduino.h"
#include <SoftwareSerial.h>
#include "MidiMessage.h"


/*
 * USER EDITABLE VALUES
 */

//Start/Stop/Continue gate times, in MICROS
int msgTime = 1000;//start/stop/continue message length

/*
 * END OF USER EDITABLE VARIABLES
 */

// Prototypes
midiReturnCode processInput(midiMessage *buffer, const int extracted);
int deJitter(int v, int test);

// Variables and constants
int baseNote = 0;//base note midi input is tested against
int baseChannel = 0;//base channel midi input is tested againsg


// Pins
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)

// MIDI
SoftwareSerial sMidi(2, 3); // RX, TX - though we define 3 as the rx pin we never send anything from it so we can safely use it as a digital out instead
midiMessage buffer;
int byteCount = 0;
midiReturnCode r = DONOTHING;

// Clock and Divider
int clockHigh = 0;//time clock went high
int clockDivideCount = 0;//current clock count
int clockDivideValue = 0;//divide clock by this
int clockTime = 500;//time for a clock high pulse, this needs to be short or it'll just be on all the time

// Play messages
int playHigh = 0;


void setup()
{
    for (int i=0; i<2; i++) {
        pinMode(digPin[i], OUTPUT);
        digitalWrite(digPin[i], LOW);
    }
    // set up the 8-bit DAC output pins
    for (int i=0; i<8; i++) {
        pinMode(pinOffset+i, OUTPUT);
        digitalWrite(pinOffset+i, LOW);
    }
    
    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);
    
    sMidi.begin(31250);
 
}


void loop()
{

    int timeNow = micros(); //we need to query the time now in various places so we grab it once at the start
    
    if (sMidi.available()){
        const int extracted = sMidi.read();
        r = processInput(&buffer, extracted);
        if (r == SENDTOCOMPONENT) {
            if ( (buffer.channel == baseChannel) && (buffer.mType == NoteOn) && buffer.value >= baseNote && buffer.value <= baseNote+7 && buffer.data != 0) {
                int i = buffer.value-baseNote+2;
                (i < 5) ? PORTD |= (1<< i+3) : PORTB |= (1<<i-5); //write the output high
            } else if ( (buffer.channel == baseChannel) && (buffer.mType == NoteOff || (buffer.mType == NoteOn && buffer.data == 0) ) && buffer.value >= baseNote && buffer.value <= baseNote+7) {
                int i = buffer.value-baseNote+2;
                (i < 5) ? PORTD &= ~(1<< i+3) : PORTB &= ~(1<<i-5); //write output low
            }
        } else if (r == PLAYCONTROL)
        {
            if (buffer.mType == Clock) {
                ++clockDivideCount;
                if (clockDivideCount >= clockDivideValue) {
                    PORTD |= (1<< 3);
                    clockHigh = timeNow;
                    clockDivideCount = 0;
                }
            } else {
                clockDivideCount = 0;
                PORTD |= (1<< 4);
                playHigh = timeNow;
            }
        }
    }
    
    
    //clock and play message off checking
    if (timeNow-clockHigh > clockTime && clockHigh != 0) {
        clockHigh = 0;
        PORTD &= ~(1<<3);
    }
    if (timeNow-playHigh > msgTime && playHigh != 0) {
        playHigh = 0;
        PORTD &= ~(1<<4);
    }
    
    //Analog reads to set various values
    uint16_t a = analogRead(A0)>>6;
    if(a != baseChannel)
    {
        baseChannel = a;
    }
    
    a = analogRead(A1) << 4;
    if(a != clockTime)
    {
        clockTime = a;
    }
    
    a = analogRead(A2)>>3;
    if(a != baseNote)
    {
        baseNote = a;
    }
    
    a = analogRead(A3)>>2;
    if (a != clockDivideValue) {
        clockDivideValue = a;
    }
    
}

//this dejitter only has a value of 2 so we get the most results returned for testing, standard patches have this around 8
int deJitter(int v, int test)
{
    if (abs(v - test) > 2) {
        return v;
    }
    return test;
}

/*
 * Very limited midi parser, just deals with notes and play control messages (start/stop/continue/clock)
 */
midiReturnCode processInput(midiMessage *buffer, const int extracted)
{

    //we test on the top bit to see if its a status bit or not
    //if its a status bit we need to start a new message
    if ((extracted & 0x80)>>7) {
        
        //get the midi status byte without the channel offset, ie blank the bottom byte
        MidiType status = (extracted >= 0xF0)?(MidiType)extracted:(MidiType)(extracted & ~0xF);
        
        //switch on what type of message we got
        switch (status) {
            case NoteOn:
            case NoteOff:
                //channel dependant midi messages
                buffer->mType = status;
                buffer->channel = extracted & 0xF;//get the channel data
                buffer->value = 0;
                buffer->data = 0;
                byteCount = 0;
                return DONOTHING;
            case Start:
            case Continue:
            case Stop:
            case Clock:
                //Single Byte midi message about time
                buffer->mType = status;
                buffer->length = 1;
                byteCount = 0;
                return PLAYCONTROL;
            default:
                return DONOTHING;
        }
    } else {
        //if we get to here we are not a status bit so we need to append or do something
        switch (buffer->mType) {
            case NoteOn:
            case NoteOff:
                //2 data bit messages
                if (byteCount == 0) {
                    //add the value to the midi data but return a null array as info not finished
                    buffer->value = extracted;
                    ++byteCount;
                    return DONOTHING;
                } else {
                    //attach the velocity or cc data and output the buffer as the message is complete
                    buffer->data = extracted;
                    buffer->length = 3;
                    byteCount = 0;
                    return SENDTOCOMPONENT;
                }
                break;
            default:
                break;
        }
        
    }
    return DONOTHING;
}

