//
//  ard2max.cpp
//  
//
//

#include "ard2maxClass.h"
#include "Arduino.h"
#include "HardwareSerial.h"

//---------------------------------------
//INITIALIZATION FUNCTIONS
//---------------------------------------

/**
 * @class ArdToMax
 */
ArdToMax::ArdToMax()
{
  hasExpander(false);
  setupDac();
  aIn = 1;
  deJitterVal =  1;
}

/**
 * set the right bits for the dac
 * @method setupDac
 */
void ArdToMax::setupDac()
{
  ADCSRA = 0; //we set this up again lower down
  ADCSRB = 0; //bit6 turns on multiplex the rest turn on ref signal, this disables everything
  sbi(ADMUX, REFS0); //0 1 AREF, Internal Vref AVCC
  sbi(ADCSRA, ADATE); //Bit 5 – ADATE: ADC Auto Trigger Enable When this bit is written to one, Auto Triggering of the ADC is enabled. The ADC will start a conversion on a positive edge of the selected trigger signal. The trigger source is selected by setting the ADC Trigger Select bits, ADTS in ADCSRB.
  sbi(ADCSRA, ADEN); //Bit 7 Writing this bit to one enables the ADC.
  sei();
  sbi(ADCSRA, ADSC); //Bit 6 In Free Running mode, write this bit to one to start the first conversion.
}

//---------------------------------------
//SERIAL SEND FUNCTIONS
//---------------------------------------

/**
 * read the adc and output
 * unlike the analog read function in the arduino library this is non blocking because it doesnt have to wait for a conversion to be done 
 * This is because we are constantly polling this function, but it only replies after a predefined 'settle time' which allows the dac to get a new value
 * This means we can really tweak this for performance
 * @method getInputs
 */
void ArdToMax::getInputs()
{
    if(readIn > SETTLETIME){
      (aIn < inputsMax) ? aIn++ : aIn = 0;
      int low = ADCL;
      int high = ADCH;
      int temp;
      if(deJitState == true){
        temp = deJitter((high << 8) | low, data[aIn]);
      } else {
        temp = (high << 8) | low;
      }
      if(temp != data[aIn]){
        data[aIn] = temp;
        ArdStream->write(XON);//this is our transmission start id DC1/XON
        ArdStream->write(aIn);//this is a char to indicate which input it is 0-5
        sendValueAsTwo7bitBytes(temp);//send the adc int data as 2 bytes
        ArdStream->write(XOFF);//this is our transmission stop id DC3/XOFF
      }
      (aIn < inputsMax) ? ADMUX++ : ADMUX &= 0xF8; //change the ADMUX channel
      readIn = 0;
    } else {
      readIn++;
    }

}

//'classic' deJitter routine as seen in Darwin Grosse's 20Objects sketches
int ArdToMax::deJitter(int v, int test)
{
  if (abs(v - test) > deJitterVal) {
    return v;
  }
  return test;
}

/**
 * From the firmata library
 * @param value int value to break into bytes and pass to serial
 */
void ArdToMax::sendValueAsTwo7bitBytes(int value)
{
  ArdStream->write(value >> 7 & B01111111); // MSB
  ArdStream->write(value & B01111111); // LSB
}

void ArdToMax::clkOut()
{
    ArdStream->write(XON);
    ArdStream->write(CLKID);
    ArdStream->write(XOFF);
}


//---------------------------------------
//CONTROL FUNCTIONS
//---------------------------------------


/**
 * [ArdToMax::begin description]
 */
void ArdToMax::begin()
{
    begin(115200);
}
/**
 * @method begin
 * [ArdToMax::begin description]
 * @param i Serial Speed
 */
void ArdToMax::begin(long i)
{
    Serial.begin(i);
    ArdStream = &Serial;
    return;
}

int ArdToMax::establishContact() {
  pB = PORTB; //holding bay port B
  pD = PORTD; //port D
  while (ArdStream->available() <= 0){
    ArdStream->write(XON);
    ArdStream->write(HANDSHAKE);//acknowledgement val
    ArdStream->write(XOFF);
    delay(300);  
  }

}

int ArdToMax::available()
{
  return ArdStream->available();
}

/**
 * These two expander related functions stop the dac reading further than we need in getInputs
 * Also used in the Digiout to stop incorrect bits getting send to dac
 * @param has  [description]
 */
void ArdToMax::hasExpander(boolean has)
{
  (has == true)?expander = true:expander = false;
  setInputCount(has);
}
/**
 * Private function for setting the input count, is called by the hasExpander routine
 * @param nCount 0 for no expander 1 for expander
 */
void ArdToMax::setInputCount(int nCount)
{
  inputsMax = (nCount != 1)?3:5; 
}

void ArdToMax::setDejitter(boolean on, int amount)
{
  if(on == true){
    deJitState = true;
    deJitterVal = amount;
  } else {
    deJitState = false;
  }
}

void ArdToMax::setDejitter(int on)
{
  (on == 1)?deJitState = true:deJitState = false;
}


//---------------------------------------
//OUTPUT FUNCTIONS
//---------------------------------------

//we need to get bytes until we get a line end character
void ArdToMax::read()
{
  memset(input,0,sizeof(input));// Ensure no old data remains from previous loop
  //get the input till the terminator
  ArdStream->readBytesUntil(XOFF, (char *)input, 32);
  //do output
  outputRouter(input);
}

void ArdToMax::outputRouter(char *input)
{
    if(input[0] == 'd'){
      digiOut(input);
    } else if(input[0] == 'a'){
      dacOut(input);
    } else {
      ArdStream->println(-1);
    }
}

//chooses a port and outputs
void ArdToMax::digiOut(char *input)
{
  int channel = input[1]-'0';
  int output = input[2]-'0';
  if(expander == false && channel > 1)
    return;
  pB = PORTB;
  pD = PORTD;
  (channel < 5) ? (output ? (pD) |= (1UL << (channel + 3)) : (pD) &= ~(1UL << (channel + 3))) : (output ? (pB) |= (1UL << (channel - 5)) : (pB) &= ~(1UL << (channel - 5)));
  PORTB = pB;
  PORTD = pD;
}

void ArdToMax::dacOut(char *input)
{
    String final = input;
    final = final.substring(1);
    if(final.toInt() > MAXDACOUT)
      final = (String)MAXDACOUT;
    dacOutput(final.toInt());
}

void ArdToMax::dacOutput(byte v)
{
  PORTB = (PORTB & B11100000) | (v >> 3);
  PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
}
