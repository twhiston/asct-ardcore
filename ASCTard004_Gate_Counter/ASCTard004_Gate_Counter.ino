//  ============================================================
//
//  Program: ASCTard004 
//
//  Description: Counter and Flip Flop
//  With Mode knobs CCW clock input is compared to a value set by a2 or a3 and triggers when value is reached
//  With mode knob CW clock creates a toggled on/off whos length is defined by the value of a2/a3
//
//  I/O Usage:
//    Knob 1: Mode 1 , ccw = count cw = toggle
//    Knob 2: Mode 2 , ccw = count cw = toggle
//    Analog In 1: Count max 1 
//    Analog In 2: Count Max 2
//    Digital Out 1: Out 1
//    Digital Out 2: Out 2
//    Clock In: Gate/Trigger In
//    Analog Out: Not Used
//
//  Created:  18/03/2013
//
//  ============================================================
//
//  ================= start of global section ==================

//  constants related to the Arduino Nano pin use
unsigned period = 0; 
const int clkIn = 2;           // the digital (clock) input
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)
const int trigTime = 25;       // 25 ms trigger timing
int tempval = 0;
int avalue[4] = {0,0,0,0}; //values for inputs
int counterval[2] = {0,0};
int toggleval[2] = {0, 0};
int countermax[2] = {0, 0};
int mode[2] = {0,0};

volatile int clkState = LOW;

//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};        // start with both set low
unsigned long digMilli[2] = {0, 0};  // a place to store millis()


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
  
  attachInterrupt(0, isr, RISING);
}

void loop()
{
  //update the count max
    /* set the count max */
  
   updatemax();
   updatemode();
  //get the input
    if (clkState == HIGH) {
      clkState = LOW;
      for (int i=0; i<2; i++) {
        if(mode[i] == 0){
        counter(i);
        } else {
        toggle(i);
        }
      }
    }
      
      // clear outs if in mode 0
      for (int i=0; i<2; i++) {
       if(mode[i] == 0){
         if ((digState[i] == HIGH) && (millis() - digMilli[i] > trigTime)) {
           digState[i] = LOW;
           digitalWrite(digPin[i], LOW);
         }
       }
     }
}


/* the counter routine */
int counter(int input){
  if(counterval[input] >= countermax[input]){
      counterval[input] = 0;
      digState[input] = HIGH;
      digMilli[input] = millis();
      digitalWrite(digPin[input], HIGH);
    } else {
     int count = counterval[input];
     count = count++;
     counterval[input] = count;
    }
}

/*the toggle routine */
int toggle(int input){

   if(toggleval[input] == 0 && counterval[input] >= countermax[input]){
      toggleval[input] = 1;
      counterval[input] = 0;
      digState[input] = HIGH;
      digMilli[input] = millis();
      digitalWrite(digPin[input], HIGH);
    } else if(toggleval[input] == 1 && counterval[input] >= countermax[input]){
      toggleval[input] = 0;
      counterval[input] = 0;
      digState[input] = LOW;
      digMilli[input] = millis();
      digitalWrite(digPin[input], LOW);  
    } else{
      int count = counterval[input];
      count = count++;
      counterval[input] = count;
    } 
  
  
}

/*update the max count */
int updatemax(){
  for (int i=2; i<4; i++) {
    tempval = deJitter(analogRead(i), avalue[i]);
    if (tempval != avalue[i]) {
      avalue[i] = tempval;
      int counterval = countermax[i];
      countermax[i-2] = avalue[i] >> 4;
    //  Serial.print(" Max ");
     // Serial.println(countermax[i-2]);
    }
  }
}

/* change the mode from count to toggle */
int updatemode(){
  for (int i=0; i<2; i++) {
   tempval = deJitter(analogRead(i), avalue[i]);
   tempval = tempval >> 8;
   if (tempval != avalue[i]) {
     avalue[i] = tempval; 
     if(avalue[i] < 2){
       mode[i] = 0; 
     } else {
       mode[i] = 1; 
     }
   }
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

void isr()
{
  clkState = HIGH;
}

int deJitter(int v, int test)
{
  if (abs(v - test) > 8) {
    return v;
  }
  return test;
}

//  ===================== end of program =======================
