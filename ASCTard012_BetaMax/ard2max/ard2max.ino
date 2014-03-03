/**
 * asctArdToMax
 *
 * Max/Msp and ardcore integration, comes with max/msp and puredata externals to deal with the serial message processing and to make communication easier
 * Uses no external libraries, the ard2max class inherets from the arduino Serial and Stream classes.
 * Designed to be as lightweight and fast as possible, in my tests its demonstrably faster than existing Firmata based communication solutions for the ardcore.
 *
 * Version 1.0
 */

#include "ard2maxClass.h"

ArdToMax ard; //initialize our ardcore object, this also initializes the dac because having it set up in the right way is a prerequisite to this sketch working
boolean clkState = LOW; //global clock variable

void setup()
{
   //Setup IO
   //This is just a fancy way of setting pins to input or output mode and then making sure they are all set to low to start, i prefer the brevity of this way of doing it
   DDRD = DDRD | B11111000;
   DDRB = B111111;
   PORTD = B00000000;
   PORTB = B000000;

   ard.begin(115200);       //start the communication, exactly like Serial.begin() because we inherit this method from that class! 115200 is the default and does not need to be specified, we do so for example purposes as you may specify any serial speed you want
   ard.establishContact();  //this sends a value (6) over the serial till it gets a byte in return from the max object, intercept this value if you need some kind of serial init bang in your max patch
   ard.setDejitter(true, 1);//true and 1 are the default values so you dont really need this line, its provided as an example to show you can change it, increase the value to increase the amount of dejitter
                            //its really just an interface to the classic deJitter in Darwin Grosse's patches, there is also an overloaded version of this function which takes only true or false. If set to true it will set the dejitter value to the default of 1
   ard.hasExpander(true);   /**
                            * you can comment out this line if you dont have an expander as the default is none
                            * when the expander mode is false (default) all it does is safeguard you against sending digital on/off messages to the dac pins.
                            * Theres not really any reason that you would want to set individual bits of the dac if you dont have an expander imo so this is provided as a convenient safeguard for you if needed
                            */   
   attachInterrupt(0, isr, RISING); //though we have a function to send out a clock message in the ard2max class you handle the invocation and execution by attaching an interrupt here and the basic isr() call below
}


/**
 * The main loop is very simple as all the work is done behind the scenes. Order of importance is clock, inputs, serial messages from max
 */
void loop()
{
  if(clkState){
      ard.clkOut();
      clkState = LOW;
  }
  while(ard.available())
    ard.read();
  ard.getInputs();
}


//Interrupt vector for clk in, sets the clock state to high.
void isr()
{
  clkState = HIGH;
}
