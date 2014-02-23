//  ============================================================
//
//  Program: ASCTard001 Analytic Geometry
//
//  Description: Records cv in and plays it back from co-ordinates it receives as cv,
//               See the readme.md in the folder for this sketch to learn a bit more about how it works
//
//  I/O Usage:
//    Knob 1: MODE
//    Knob 2: 
//    Analog In 1: CV record when in record mode / x position in play mode
//    Analog In 2: unused when in record mode / y position in play mode
//    (tip - try the dreamboat cv outs into here in play mode)
//    Digital Out 1: Trigger clock high/step output
//    Digital Out 2: Trigger clock high/step output
//    Clock In: External clock input
//    Analog Out: 8-bit output (straight through from input when in record mode, otherwise pattern playback)
//
//  Created:  31/01/2013
//
//  ============================================================
//
//  ================= start of global section ==================

//  constants related to the Arduino Nano pin use
const int clkIn = 2;           // the digital (clock) input
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)
const int trigTime = 25;       // the standard trigger time
const int pauseTime = 250;     // require a pause

//  variables for interrupt handling of the clock input
volatile int clkState = LOW;

//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};  // start with both set low
unsigned long digHIGH[2] = {0, 0};      // the times of the last HIGH

//  recording and playback variables
int seq_array[64] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63};
//these ints hold our current x/y position
int seq_x = 0;
int seq_y = 0;

//holds our mode value, taken from a0 knob
int mode = 1;
//current recording step
int recstep = 0;
//pattern length
int rec_step_max = 64;
//pattern playback variable
int auto_pat_pos = 0;

//  limit changes to the sequencer to require a 100ms pause
int lastPos = -1;
unsigned long lastPosMillis = 0;

//  ==================== start of setup() ======================

void setup() {

  //for debugging
   Serial.begin(9600);      // open the serial port at 9600 bps:    

  // set up the digital (clock) input
  pinMode(clkIn, INPUT);
  
  // set up the digital outputs
  for (int i=0; i<2; i++) {
    pinMode(digPin[i], OUTPUT);
    digitalWrite(digPin[i], LOW);
  }
  
  // set up the 8-bit DAC output pins
  for (int i=0; i<8; i++) {
    pinMode(pinOffset+i, OUTPUT);
    digitalWrite(pinOffset+i, LOW);
  }
  
  attachInterrupt(0, isr, RISING);
}
//  ==================== start of loop() =======================

void loop()
{
  //if we have a clock in we have some action
  if (clkState == HIGH) {
     //get our x/y values for grid lookup
     //we make our input between 0 and 7 so we have 8 steps in each direction to choose from

  //if we are in record
  if (mode < 3){
        //reset clock state
      clkState = LOW;
      //get the input value from a2
      int recval = quantNote(analogRead(2));
      //then record value at the step we are at
      seq_array[recstep]=recval;
      //pass our input through the output
      dacOutput(recval);
      //increase record step unless step is at 64 in which case reset to 0
      if (recstep == 0) {
        //if sequence is at start red light comes on
        digitalWrite(digPin[1], HIGH);
        recstep++;
      } else if(recstep < 63){
       //green light for step
      digitalWrite(digPin[0], HIGH);
      //set digstates
      digState[0] = HIGH;
      digHIGH[0] = millis();
      //increase the step
      recstep++;
      } else {
        //if the recstep is > 63
       recstep = 0;
      }

    } else if (mode >= 3 && mode < 5) {
      //this mode mixes in some of the original when both inputs hit 0
       //get our x/y values for grid lookup
      int temp_x = analogRead(2) >> 7;
      int temp_y = analogRead(3) >> 7;
      clkState = LOW;
      if(temp_x == 0 && temp_y == 0){
        dacOutput(seq_array[auto_pat_pos]);
        //set our digital outs high for triggers
        digitalWrite(digPin[0], HIGH);
        digitalWrite(digPin[1], HIGH);
        //set the info in our array
        digState[0] = HIGH;
        digHIGH[0] = millis();
        //go through the sequence till the end then reset
        if(auto_pat_pos < 63){
          auto_pat_pos++;
        } else {
          auto_pat_pos = 0;
        }
      } else{
        
        //write the output to the dac which is the grid position we are in from the x/y vals at cv in
        //set our digital outs high for triggers
        digitalWrite(digPin[0], HIGH);
        digitalWrite(digPin[1], HIGH);
        //set the info in our array
        digState[0] = HIGH;
        digHIGH[0] = millis();
      int x_mult = temp_x*8;
      int grid_pos = x_mult+temp_y;
      dacOutput(seq_array[grid_pos]);
      }
  }else if (mode >= 5) {
      //This is Playback mode
      //get our x/y values for grid lookup
      int temp_x = analogRead(2) >> 7;
      int temp_y = analogRead(3) >> 7;
      //set the clock low
      clkState = LOW;
  
      //write the output to the dac which is the grid position we are in from the x/y vals at cv in
      int x_mult = temp_x*8;
      int grid_pos = x_mult+temp_y;
      dacOutput(seq_array[grid_pos]);
      
      //set our digital outs high for triggers
      digitalWrite(digPin[0], HIGH);
      digitalWrite(digPin[1], HIGH);
      //set the info in our array
      digState[0] = HIGH;
      digHIGH[0] = millis();
    }
   //end of modes routine 
  } 
//check to see if we should turn off the digital outs
   if ((millis() - digHIGH[0] > trigTime) && (digState[0] == HIGH)) {
      digState[0] = LOW;
      digitalWrite(digPin[0], LOW);
      digitalWrite(digPin[1], LOW);
   }
  
    //get our mode value from a0
    mode = analogRead(0) >> 7;

  
}


//  =================== convenience routines ===================

//  isr() - quickly handle interrupts from the clock input
//  ------------------------------------------------------
void isr()
{
  clkState = HIGH;
}

//  dacOutput(byte) - deal with the DAC output
//  -----------------------------------------
void dacOutput(byte v)
{
  PORTB = (PORTB & B11100000) | (v >> 3);
	PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
}

//  quantNote(int) - drop an incoming value to a note value
//  -------------------------------------------------------
int quantNote(int v)
{
  // feed this routine the input from one of the analog inputs
  // and it will return the value in a 0-64 range.
  return (v >> 4) << 2;
}

//  ===================== end of program =======================
