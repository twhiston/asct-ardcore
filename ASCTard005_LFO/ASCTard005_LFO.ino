   /*  ============================================================
  *
  *  Program: ASCTard005 - Experimental LFO 
  *
  *  Description:  Experimental LFO using Mozzi's Oscil object to hold and playback wavetables
  *                Its called experimental because it definitely isnt perfect, I dont have a hardware scope to test the outputs
  *                so this means the phMod function code is probably not correct, but none the less what this will do is let
  *                you create some unique lfo shapes.
  *
  *                One potential issue is that as it uses the Oscil object it pulls in the mozzi guts which means that it sets the
  *                Mozzi DAC pins, this means the digital outs on the arduino will output part of the wave and so arnt usable for other output
  *                This could be worked around by building a custom oscil.h file object but for an experimental patch this is too time consuming
  *                It wont break your ardcore as it is now its just a bit annoying as EOR/EOC triggers would be nice on the digi outs
  *
  *
  *
  *  YOU MUST HAVE THE MOZZI LIBRARY INSTALLED FOR THIS TO WORK
  *  http://sensorium.github.com/Mozzi/
  *
  *  I/O Usage:
  *    A0: lfo range coarse
  *    A1: lfo shape (sin, cos, saw - thats all you get at 2048 in mozzi)
  *    A3: Lfo speed
  *    A4: LFO fm using Oscils phMod function
  *    Digital Out 1: 
  *    Digital Out 2: 
  *    Clock In: Reset Lfo cycle on HIGH
  *    Analog Out: Lfo out
  *
  *  Created:  16/03/2013
  *
  */

//Set up the stuff for Oscil

#include <Oscil.h> // oscillator template
#include <tables/sin2048_int8.h> // sine table
#include <tables/saw2048_int8.h> // saw table
#include <tables/cos2048_int8.h> // cos table
// use: Oscil <table_size, update_rate> oscilName (wavetable)
Oscil <2048, AUDIO_RATE> aSin(SIN2048_DATA);
// use #define for CONTROL_RATE, not a constant
#define CONTROL_RATE 128 // powers of 2 please
#define AUDIO_PWM_RESOLUTION   488 


int fmIn = 1;
 
/* ARDCORE SETUP */


//  constants related to the Arduino Nano pin use
const int clkIn = 2;           // the digital (clock) input
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)
const int trigTime = 2;       // 25 ms trigger timing

//  variables for interrupt handling of the clock input
volatile int clkState = LOW;
int clkDivide = 0;

//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};        // start with both set low
unsigned long digMilli[2] = {0, 0};  // a place to store millis()

int inValue = 0;               // the input value
int a0 = 0; //a0 value
int a1 = 0; //a1 value

Q15n16 range = 0; //range for lfo

//the last bit of the sample
int lastbit = 0;

int nextval = 0;
int shiftedval = 0; //our shifted output
int output = 0;

//values for the main loop
int tempval = 0; //holds our input readings
float outval; //value to pass to Oscil


void setup(){
  
  /* Debug */
 // Serial.begin(9600);
   
  /* MOZZI SETUP */
  aSin.setFreq(1u); // set the frequency with an unsigned int or a float
  
  
  /*Ardcore Setup */
  // set up the digital (clock) input
  pinMode(clkIn, INPUT);
  // set up the 8-bit DAC output pins
  for (int i=0; i<8; i++) {
    pinMode(pinOffset+i, OUTPUT);
    digitalWrite(pinOffset+i, LOW);
  }
  
  // set up an interrupt handler for the clock in.
  // Note: Interrupt 0 is for pin 2 (clkIn)
  attachInterrupt(0, isr, RISING);
  
  
}


void updateControl(){
  // put changing controls in here
   // test for input change

}


int updateAudio(){

}


void loop(){
  
  //Do a reset if the clock state is high
   if (clkState == HIGH) {
     clkState = LOW;
     aSin.setPhase(0);
   }
  
  //reset our texp val   
  tempval = 0;
   
  /* set the frequency */
  tempval = deJitter(analogRead(2), inValue);
  if (tempval != inValue) {
    inValue = tempval;
    //cast into to float
    outval = (float)tempval;
    //divide by range
    outval = outval/range;
    //set the Oscil freq
    aSin.setFreq(outval);
  }
  
  /* FM or not */
  tempval = deJitter(analogRead(3), fmIn);
  if(tempval != fmIn){
    fmIn = tempval;
    //we need to make the input a val between -1 and 1
    int fm2 = tempval >> 2;
    Q15n16 fmOut = fmIn >> 8;  
    //set the val
    nextval = aSin.phMod(fmIn);
  } else {
    //if no fm input just play the next value
    nextval = aSin.next();
  }
  
  //add 128 as oscil works -128 to +128 and we want 0 to 255. If you want the output to be only the 'top half' of the wave comment (and thus have no output for half a cycle) this line out
  nextval = nextval+128;

  //no overload
  if(nextval > 255){
   nextval = 255 ;
   }

  dacOutput(nextval);
  
  //set the wavtable
  tempval = deJitter(analogRead(1), a1);
  tempval = tempval/160;
  if (tempval != a1) {
    a1 = tempval;
    if(a1 < 3){
       aSin.setTable(SIN2048_DATA);
    } else if(a1 > 2 && a1 < 5) {
      aSin.setTable(COS2048_DATA);
    } else {
      aSin.setTable(SAW2048_DATA);
    }
}

  //set the speed offset
   tempval = deJitter(analogRead(0), a0);
   if (tempval != a0) {
      a0 = tempval; 
      range = a0 >> 5;
      range++;
      inValue = 0;
 }
 
  
}

void isr()
{
  // Note: you don't want to spend a lot of time here, because
  // it interrupts the activity of the rest of your program.
  // In most cases, you just want to set a variable and get
  // out.
  clkState = HIGH;
}

int deJitter(int v, int test)
{
  // this routine just make sure we have a significant value
  // change before we bother implementing it. This is useful
  // for cleaning up jittery analog inputs.
  if (abs(v - test) > 3) {
    return v;
  }
  return test;
}

//  dacOutput(byte) - deal with the DAC output
//  -----------------------------------------
void dacOutput(byte v)
{
  //Serial.println("CALLED"); 
  PORTB = (PORTB & B11100000) | (v >> 3);
  PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
}



