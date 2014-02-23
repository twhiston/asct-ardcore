//  ============================================================
//
//  Program: ASCTard002 
//
//  Description: Really strightforward cv scaler, set a min and a max with 
//  a0 and a1, cv in at a2 and it will scale it to your values. Dont forget that
//  the ardcore IGNORES NEGATIVE CV. so shift it before input.
//
//  TIP: try feeding an lfo into it and then into a quantizer and to a vco, nice way to generate melodic patterns
//
//
//  if min and max cross it inverts
//
//  I/O Usage:
//    Knob 1: minimum
//    Knob 2: maxixux
//    Analog In 1: CV to scale
//    Analog In 2: 
//    Digital Out 1: 
//    Digital Out 2: 
//    Clock In: 
//    Analog Out: Scaled output
//
//  Created:  04/02/2013
//
//  ============================================================
//
//  ================= start of global section ==================

//  constants related to the Arduino Nano pin use
const int clkIn = 2;           // the digital (clock) input
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)

float l_limit = 0; //lower limit on a0
float h_limit = 1023; //upper limit on a1
int input = 0; //input on a2
float output = 0;
int cast_out = 0;

//  ==================== start of setup() ======================

void setup() {

  //for debugging
  // Serial.begin(9600);      // open the serial port at 9600 bps:    

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
}

void loop()
{
  //get the input
  if(input != analogRead(2) >> 2){
  input = analogRead(2) >> 2;
  //a0 and a1 set lower and upper limits
  l_limit = analogRead(0) >> 2;
  h_limit = analogRead(1) >> 2;
  //do tha math 
  output = (input / ((255 - 0) / (h_limit - l_limit))) + l_limit;
  //cast our float to an int
  cast_out = static_cast<int>(output);
  //output
  dacOutput(cast_out);
  }
  
}

//  =================== convenience routines ===================


//  dacOutput(byte) - deal with the DAC output
//  -----------------------------------------
void dacOutput(byte v)
{
  PORTB = (PORTB & B11100000) | (v >> 3);
	PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
}



//  ===================== end of program =======================
