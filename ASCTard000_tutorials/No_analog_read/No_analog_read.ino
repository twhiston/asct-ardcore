/*
* Fast Analog Read!!!!!
* 8-bit reading direct from mux without analogRead(); also shows how to change value being read by changing the ADMUX bits
*/
 
 
#define DEBUG //for now leave this defined so you can read the output when the clock switches the input to pin 3, disable for speed tests



volatile int clkState = LOW;

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

void setup(){

  //ins and outs
  DDRD = DDRD | B11111000;
  DDRB = B111111;
  PORTD = B00000000;
  PORTB = B00000000;


  //clear ADCSRA and ADCSRB registers
  ADCSRA = 0;
  ADCSRB = 0;


  sbi(ADMUX, REFS0); //set reference voltage
  sbi(ADMUX, ADLAR);//left align the ADC value- so we can read highest 8 bits from ADCH register only
  sbi(ADMUX,1); //this is input 2 which is bit 1, this is what we read from

  sbi(ADCSRA, ADPS2); //set ADC clock with 32 prescaler- 16mHz/32=500kHz
  sbi(ADCSRA, ADPS0); //set ADC clock with 32 prescaler- 16mHz/32=500kHz
  sbi(ADCSRA, ADATE); //enabble auto trigger
  sbi(ADCSRA, ADEN); //enable ADC
  sbi(ADCSRA, ADSC); //start ADC measurements

  //if you want to add other things to setup(), do it here
  #ifdef DEBUG
    Serial.begin(9600);
  #endif

  attachInterrupt(0, isr, RISING);
}

void loop(){
  //if the clock goes high get a reading from a different pin - a[3]
  if(clkState == HIGH){

    sbi(ADMUX,0); //if we set pin 0 to the current config we are reading pin 3 (10 | 01 = 11)
    delay(1); //if we dont have a delay of 1 we get a reading from pin2 not 3, must be a way around this
    int tempdata = ADCH; //read the value
    #ifdef DEBUG
      Serial.println(tempdata); //print out the value from our second analog in
    #endif
    cbi(ADMUX,0); //unset the 0 bit so we are back to pin 2
    clkState = LOW;

  }

  int data = ADCH; 
  dacOutput(data); //our output is only between 0-255 because we are only reading at 8 bit -  sbi(ADMUX, ADLAR)
}

void dacOutput(byte v)
{
  PORTB = (PORTB & B11100000) | (v >> 3);
  PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
}

void isr()
{
  clkState = HIGH;
}



