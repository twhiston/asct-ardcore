//  ============================================================
//
//  Program: Tone Rows
//
//  Description: Play some tone rows 
//  Big thanks to Darwin Grosse who's AC02_Quantizer patch is the basis for this
//
//  I/O Usage:
//    Knob 1: Transposition of note
//    Knob 2: Duration of output gate
//    Analog In 1: Input voltage to be quantized
//    Analog In 2: If HIGH, suppress quantization
//    Digital Out 1: Trigger on output
//    Digital Out 2: Gate on output
//    Clock In: External trigger of quantization
//    Analog Out: Quantizer output
//
//  Input Expander: unused
//  Output Expander: 8 bits of output exposed
//
//  Created:  14 Feb 2013
//  
//  ============================================================
//
//  ================= start of global section ==================

//  constants related to the Arduino Nano pin use
const int clkIn = 2;           // the digital (clock) input
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)

//  constant for actual 0-5V quantization (vs. >> 4)
const int qArray[61] = {
  0,   9,   26,  43,  60,  77,  94,  111, 128, 145, 162, 180, 
  197, 214, 231, 248, 265, 282, 299, 316, 333, 350, 367, 384, 
  401, 418, 435, 452, 469, 486, 503, 521, 538, 555, 572, 589, 
  606, 623, 640, 657, 674, 691, 708, 725, 742, 759, 776, 793, 
  810, 827, 844, 862, 879, 896, 913, 930, 947, 964, 981, 998, 
  1015};

//  variables for interrupt handling of the clock input
volatile int clkState = LOW;

//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};  // start with both set low
unsigned long digMilli[2] = {0, 0};     // the timing variables
unsigned long digTimes[2] = {10, 10};   // the timing settings

// variables of interest

int inValue = 0;               // the input value
byte outValue = 0;             // the DAC output value
int oldOut = -1;               // the last output value
int inshifted = 0;
int a2temp = 0;

int doQuant = 0;               // flag to do calcs
int gateDuration = 0;          // the duration of the output gate


//tonerow vals
int rowroot = 0;             // the row root
int originalrow[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
int *tclock = 0;
int rootnote = 0;

//  ==================== start of setup() ======================
//  Standard setup routine - see ArdCore_Template for info.

void setup() {
  Serial.begin(9600);
  
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

  // set up the clock input
  attachInterrupt(0, isr, RISING);
  
  //make the original row
  makeOriginalrow(rootnote);
  
  //set up a random number
  randomSeed(random(10000));
}

//  ==================== start of loop() =======================

void loop()
{
  int tempval = 0;
  
  // test for root note  change
  tempval = analogRead(0) / 86;
  if (tempval != rowroot) {
    rowroot = tempval;
    makeOriginalrow(rowroot);
   clockbuilder(originalrow);
    doQuant = 1;
  }

  // test for input change
  a2temp = deJitter(analogRead(2), inValue);
  if (a2temp != inValue) {
    inValue = a2temp;
    doQuant = 1;
  }
  

  // test for clock tick (even if suppressed, we quantize on clock)
  if (clkState == HIGH) {
    clkState = LOW;
    doQuant = 1;
  }
  
  // do the quantization
  if (doQuant) {

    // send the note
    outValue = quantNote(inValue);
    if (outValue != oldOut) {
      dacOutput(outValue);
      oldOut = outValue;

      
    Serial.print(rowroot);
    Serial.print('\t');
    Serial.print(inValue);
    Serial.print('\t');
    Serial.print(outValue);
    Serial.print('\t');    
    Serial.print(doQuant);
    Serial.println();
    
    
      // do the triggers and gates
      for (int i=0; i<2; i++) {
        digitalWrite(digPin[i], HIGH);
        digState[i] = HIGH;
        digMilli[i] = millis();
      }
    }
    
    doQuant = 0;
  }
  
  // get the current gate time  
  digTimes[1] = analogRead(1);
  
  // test for trigger and gate turnoff
  for (int i=0; i<2; i++) {
    if ((digState[i] == HIGH) && (millis() - digMilli[i] > digTimes[i])) {
      digitalWrite(digPin[i], LOW);
      digState[i] = LOW;
    }
  }
    
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

//  deJitter(int, int) - smooth jitter input
//  ----------------------------------------
int deJitter(int v, int test)
{
  // A musically useful range of deJittering for a quantizer
  // is an input wobble of +/- 8. If you are working with
  // continuous values, you might want to change this to a
  // smaller value, or simply uncomment the next line:
  //
  // return v;
  
  if (abs(v - test) > 2) {
    return v;
  }
  return test;
}

//  quantNote(int) - drop an incoming value to a note value
//  -------------------------------------------------------
byte quantNote(int v)
{
  int tempVal = vQuant(v);  // decrease the value to 0-64 - ~ a 5 volt range
 // tempVal += transpose;      // add the transposition
  return (tempVal << 2);     // increase it to the full 8-bit range
}

//  vQuant(int) - properly convert an ADC reading to a value
//  ---------------------------------------------------------
int vQuant(int v)
{
  int tmp = 0;
  
  for (int i=0; i<61; i++) {
    if (v >= qArray[i]) {
      tmp = i;
    }
  }
  
  return tmp;
}

// this makes a tone row through some random means
int makeOriginalrow(int root) {
  originalrow[0] = root;
  int arbitraryrow[13] = {1,2,3,4,5,6,7,8,9,10,11,0};
  for(int i = 1; i < 12;){
    int seed = random(0, 12);
   // Serial.println(seed);
    if((arbitraryrow[seed] != 666) && (seed != root) ){



      originalrow[i] = seed;
      arbitraryrow[seed] = 666;

      i++;
     // Serial.println(seed);
     // Serial.print("I ");
     // Serial.println(i);

    }
  }
    Serial.println("original Row");
    for(int i = 0; i < 12; i++){
    Serial.print(originalrow[i]);
    Serial.print('\t');
  }
      Serial.println();
      
      
//  originalrow

}

int clockbuilder(int *row){
 //set the root of the clock
 rowsort(row, 12);
  rowrotate(tclock);
 
  Serial.println("clock");
    for(int i = 0; i < 12; i++){
    Serial.print(tclock[i]);
    Serial.print('\t');
  }
      Serial.println();

}
int rowsort(int *a, int n)
{
  for (int i = 1; i < n; ++i)
  {
    int j = a[i];
    int k;
    for (k = i - 1; (k >= 0) && (j < a[k]); k--)
    {
      a[k + 1] = a[k];
    }
    a[k + 1] = j;
  }
  tclock = a;

}

int rowrotate(int *row){
  for(int a = 0; a < rowroot; a++){
 int temp = row[0]; //Grab the first number from the array

for(int i = 0; i < 12; i++)
{

row[i] = row[i + 1]; //Makes all the array indexes equal the number after them

}
row[11] = temp; //Store the first number in the last slot
  }
  tclock = row;
}
//  ===================== end of program =======================
