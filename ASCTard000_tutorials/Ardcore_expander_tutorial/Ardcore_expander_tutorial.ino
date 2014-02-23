 /*  ============================================================
  *
  *  Program: ASCTard011 - Expander Tutorial
  *
  *  Description: 
  *               Quick tutorial/demos for the snazzyFX ardcore expander.
  *               This makes no sound and does nothing useful but should help explain it a bit
  *               It is STRONGLY recommended to put the outputs into a scope/o'tool so you can see whats going on
  *
  *
  *
  *  Last Update: 19/11/13
  *
  */
  
  /* ARDCORE SETUP */

//  constants related to the Arduino Nano pin use
const int clkIn = 2;           // the digital (clock) input
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)
int trigTime = 500;       // 25 ms trigger timing
byte in;
//  variables for interrupt handling of the clock input
volatile int clkState = LOW;

//  variables used to control the current DIO output states
int writeState = LOW;        // start with both set low
unsigned long writeMilli = 0;  // a place to store millis()

int tempval = 0;               // the input value
int oldin[4] = {0,0,0,0}; //to dejitter we need to hold our old input values in an array, values correspond to A2,A3,A4,A5

//IMPORTANT - THESE ARE REQUIRED IF YOU USE THE TIMER2 FUNCTION TO SEND DC TO PIN11
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

void setup(){
  
  /* Debug */
  Serial.begin(9600);
 
  /*Ardcore Setup */
  // set up the digital (clock) input
  pinMode(clkIn, INPUT);
  pinMode(13, OUTPUT);
  
    // set up the digital outputs
  for (int i=0; i<2; i++) {
    pinMode(digPin[i], OUTPUT);
    digitalWrite(digPin[i], LOW);
  }
  
  
  attachInterrupt(0, isr, CHANGE);
  
  
  // set up the 8-bit DAC output pins
  for (int i=0; i<8; i++) {
    pinMode(pinOffset+i, OUTPUT);
    digitalWrite(pinOffset+i, LOW);
  }
  
  //this is a new routine that you will need to successfully analog write DC to pin11 of the ardcore expander.
  // If you analog write without the prescaling etc.... you'll still get a pulse train but the RC filter is optimized for audio
  //Massive credit to Dan Snazelle for this function!!!!
  Setup_timer2();
  
  
}

/*
* Uncomment the lines that correspond to the feature you want to test
*
*/
void loop(){
  /*
  * New analog ins return different values as they accept bipolar inputs. 
  * No input returns 522
  * -5v returns 0
  * 5v returns 1023
  * 
  * On these inputs the knob attenuate only they DO NOT provide a fixed voltage as per ins a0-a3, 
  * thus a4 a5 input will be 522 constantly with no external in
  */
  newanalogins();
  
  
  /*
  * Bits Light Show shows the new trigger/gate outs
  * A0 controls delay between values, start with this about 9 o'clock
  * This outputs a ramp on the dac and lights up the corresponding bits out on the expander
  * This should hopefully tell you that if you use the dac out you cant use the bits out as seperate digital outs
  * So if lots of gate/trigs are needed and also a cv its good to output what would go to the dac on pin 11 (or bitbang pin 13)
  * and then you can uses bits 0-5 and 7 (arduino outs 5-10 and 12) as extra digital outs.
  * You CANNOT use bit6 (arduino out 11) as a digital out as this is the SAME output as pin 11!!!!
  */
  //bitslightshow();
  
  
  /*
  * Incerement outs addresses each of the bits of the dac with a port manipulation to set them high directly, as opposed to the above
  * which takes its output from writing an analog value.
  * Using this method we can call each of the outputs individually
  * if you dont want to use port manipulation you could equally do digitalWrite(0-13, HIGH/LOW);
  */
 // incrementouts();
  
  /*
  * PIN11 OUT
  * Pin11 is an analog out so you can use analogwrite(11, x) to write to it
  * This test takes the value of A2 and echoes it to pin11 out 
  * check out the difference between taking this output from 11 and bit6! (10v p-p on 6, 5 on 11)
  */
 // pineleven();

  /*
  * Bitbang13
  * This shows you how to bitbang a port out to provide a cv, IMPORTANT you need to put this through a slew to get a smooth value out!!!!
  */
 // bitbang13();
}

void newanalogins(){
   //get the new analog ins and print them if changed
  int a4 = analogRead(4); //value 2 of the old in array is pin 4
//  int a5 = deJitter(analogRead(5), oldin[3]); //value 3 of the old in array is pin 5
//  if(a4 != oldin[2]){ //if the new input does not equal the old one print the output
  Serial.print("a4: ");
  Serial.println(a4);
 // oldin[2] = a4; //set our old value as it has changed
 // }
 // if(a5 != oldin[3]){//if the new input does not equal the old one print the output
//  Serial.print("a5: "); 
//  Serial.println(a5); 
//  oldin[3] = a5; //set our old value as it has changed
//  }  
  
  
}

/*this shows what happens to the digital outs when you just send a value to the DAC, you can see how the bits correspond to outputs going high */
void bitslightshow(){
  for(int i = 0; i< 256; i++){
   dacOutput(i); 
   delay(analogRead(2)); 
  }
}

//see the port manipulation tutorial for how this works
void incrementouts(){
  for(int i = 0; i<10;i++){
  (i < 5) ? PORTD |= (1<< i+3) : PORTB |= (1<<i-5); //write the output high
  delay(30);
    }

  for(int i = 0; i<10;i++){
  (i < 5) ? PORTD &= ~(1<< i+3) : PORTB &= ~(1<<i-5); //write output low
  delay(30);
  }
  
}

/*this echos the input at a2 on pin11 */
void pineleven(){
  int a2 = deJitter(analogRead(2), oldin[0]); //value 0 of the old in array is pin 2
  analogWrite(11, a2/4);
}

//this is basically how to bitbang a port out. If you connect output 13 to a slew limiter or lpf you'll see a steady cv value out that echos a[2] in
//obviously because of the delays used this will mess up your timing. To use this in a patch think about using something like Timed Actions http://playground.arduino.cc/Code/TimedAction to turn the pins on and off
void bitbang13(){
 int freq =  deJitter(analogRead(2), oldin[0]);
 freq = freq+1; //no 0 here or its on all the time
 digitalWrite(13, HIGH);
 delayMicroseconds(freq); // Approximately 10% duty cycle @ 1KHz
 digitalWrite(13, LOW);
 delayMicroseconds(1025 - freq);
  
}


//this dejitter only has a value of 2 so we get the most results returned for testing, standard patches have this around 8
int deJitter(int v, int test)
{
  if (abs(v - test) > 2) {
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

void isr()
{
  clkState = !clkState;
}

//******************************************************************
// timer2 setup
// set prscaler to 1, PWM mode to phase correct PWM,  16000000/510 = 31372.55 Hz clock
void Setup_timer2() {

// Timer2 Clock Prescaler to : 1
  sbi (TCCR2B, CS20);
  cbi (TCCR2B, CS21);
  cbi (TCCR2B, CS22);

  // Timer2 PWM Mode set to Phase Correct PWM
  cbi (TCCR2A, COM2A0);  // clear Compare Match
  sbi (TCCR2A, COM2A1);

  sbi (TCCR2A, WGM20);  // Mode 1  / Phase Correct PWM
  cbi (TCCR2A, WGM21);
  cbi (TCCR2B, WGM22);
}
