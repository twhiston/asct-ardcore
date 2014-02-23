 /*  ============================================================
  *
  *  Program: ASCTard006 - Burst Generator
  *
  *  Description: 
  *               Clock in for timing. a3 high to do burst
  *
  *
  *
  *  I/O Usage:
  *    A0: 
  *    A1: 
  *    A3: burst length 0-32
  *    A4: trigger burst (does burst on high)
  *    Digital Out 1: 
  *    Digital Out 2: 
  *    Clock In: clock input for burst out timing (try using a vulcan or other complex mod source for fun bursts)
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
const int trigTime = 2;       // 25 ms trigger timing

//  variables for interrupt handling of the clock input
volatile int clkState = LOW;
int clkDivide = 0;
int clkTime[2] = {0,0};
int burstRate = 0;
int burst = 0;

//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};        // start with both set low
unsigned long digMilli[2] = {0, 0};  // a place to store millis()

int inValue = 0;               // the input value
int a[4] = {0,0,0,0}; //input vals
int aShift[4] = {0,0,0,0}; //shifted vals {,,Burst Length, Do Burst}



void setup(){
  
  /* Debug */
  //Serial.begin(9600);
 
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
  
  // set up an interrupt handler for the clock in.
  // Note: Interrupt 0 is for pin 2 (clkIn)
  attachInterrupt(0, isr, RISING);
  
}

void loop(){
  int tempval = 0;
  
  //Do a reset if the clock state is high

   

    
   if (aShift[3] == 0) {
       //if we are not doing a burst test to see if we need to
     tempval = analogRead(3)>>9;  
     if (tempval == 1 && a[3] != 1) {
      a[3] = tempval;
      aShift[3] = 1;
      } else if (tempval == 0 && a[3] == 1){
       a[3] = tempval;
      } 
     }
  
  

 
  
  //get how many we do each burst
  tempval = deJitter(analogRead(2), a[2]);

  if (tempval>>5 != aShift[2]) {
    a[2] = tempval;
    //make it a 1-32 range
    aShift[2] = (tempval>>5);
    Serial.println("Burst Length (A2) Shifted");
  Serial.println(aShift[2]);
  }
   
    // do we have to turn off any of the digital outputs?
  for (int i=0; i<2; i++) {
    if ((digState[i] == HIGH) && (millis() - digMilli[i] > trigTime)) {
      digState[i] = LOW;
      digitalWrite(digPin[i], LOW);
    }
  }
}


void isr()
{
  clkState = HIGH;
    //do the burst
  if(aShift[3] == 1){

    //count up to our burst max and do an output for each
    if(burst <= aShift[2]){
      
        clkState = LOW;
        clkTime[0] = millis();
        digState[0] = HIGH;
        digMilli[0] = millis();
        digitalWrite(digPin[0], HIGH);
        clkTime[1] = millis();
        digState[1] = HIGH;
        digMilli[1] = millis();
        digitalWrite(digPin[1], HIGH);
        burst++;
       
      } else if (burst > aShift[2]){
        burst = 0;
        aShift[3] = 0;
      }
  }
}


int deJitter(int v, int test)
{
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
