 /*  ============================================================
  *
  *  Program: ASCTard008 - Changnesia
  *
  *  Description: 
  *               Remembers the last cv in, dO1 on change ,d02 on same
  *
  *
  *
  *  I/O Usage:
  *    A0: dejitter val (turn this up to get a more stable reading, down for more chaos)
  *    A1: 
  *    A3: cv in
  *    A4: trigger length
  *    Digital Out 1: HIGH on change 
  *    Digital Out 2: HIGH on same
  *    Clock In:
  *    Analog Out: 
  *
  *  Created:  16/03/2013
  *
  */
  
  /* ARDCORE SETUP */


//  constants related to the Arduino Nano pin use
const int clkIn = 2;           // the digital (clock) input
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)
int trigTime = 25;       // 25 ms trigger timing

//  variables for interrupt handling of the clock input
volatile int clkState = LOW;

//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};        // start with both set low
unsigned long digMilli[2] = {0, 0};  // a place to store millis()

int tempval = 0;               // the input value
int a[4] = {0,0,0,0}; //input vals

int chang = 0; //the last value




void setup(){
  
  /* Debug */
 // Serial.begin(9600);
 
  /*Ardcore Setup */
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

void loop(){
  tempval = 0;
  
  
    //set the dejitter value
     tempval = analogRead(0)>>6;  
     if(tempval != a[0]){
      a[0] = tempval; 
     }
    
        //set the trigtime value
     tempval = analogRead(3);  
     if(tempval != a[3]){
      a[3] = tempval; 
      trigTime = a[3];
     }
    
    
    //get the value at a[2] and if its not the same as before output at dO[1]
    tempval = deJitter(analogRead(2), a[2]);
    if(tempval != a[2]){
      //output our old value
      dacOutput(a[2]>>2);
      a[2] = tempval;
      digState[0] = HIGH;
      digMilli[0] = millis();
      digitalWrite(digPin[0], HIGH);
      digState[1] = LOW;
      digMilli[1] = millis();
      digitalWrite(digPin[1], LOW);
    } else {
      if ((digState[1] == LOW) && (millis() - digMilli[1] > trigTime)) {
      digState[1] = HIGH;
      digitalWrite(digPin[1], HIGH);
      }
    }
    

    if ((digState[0] == HIGH) && (millis() - digMilli[0] > trigTime)) {
      digState[0] = LOW;
      digitalWrite(digPin[0], LOW);
    }
  
}



int deJitter(int v, int test)
{
  if (abs(v - test) > a[0]) {
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
