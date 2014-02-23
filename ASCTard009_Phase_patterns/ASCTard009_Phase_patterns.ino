/*
* ASCTard010 - Phase Patterns
* "This reich will last a thousand beers, oh ya"(http://www.youtube.com/watch?v=I7JY2V_GEuE)
*
* Just a bit of fun to test timed actions, infinite steve reich, uses the intervals from phase patterns which is outputs at the DAC and phased triggers on d0 & d2
* speed on a[2], transpose on a[3]
*
* You will need the timedAction library to use this http://playground.arduino.cc/Code/TimedAction
*
* You probably also need to update the timedaction library to include arduino.h rather than wprogram.h
* plenty on the net about this. e.g http://forum.arduino.cc/index.php?topic=147680.0
*
* 
*/


#include <TimedAction.h>


int playhead0 = 0;
int playhead1 = 0;
long clockon[2] = {0,0};

int trigTime = 25;

int transpose = 0;
int clockdelay = 1;

int clockspeed = 207; 
int oldclockspeed = 207;

TimedAction Clock0 = TimedAction(clockspeed,clocker0);
TimedAction Clock1 = TimedAction((clockspeed-clockdelay),clocker1);



          // { E4, F#4, B4, C#5, D5, F#4, E4, C#5, B4, F#4, D5, C#5 }
int melody[12] = { 0, 26, 111, 145, 162, 26, 0, 145, 111, 26, 162, 145 };

//int melody[6] = { 0, 26, 111, 145, 162, 26 };
const int qArray[61] = {
  0,   9,   26,  43,  60,  77,  94,  111, 128, 145, 162, 180, 
  197, 214, 231, 248, 265, 282, 299, 316, 333, 350, 367, 384, 
  401, 418, 435, 452, 469, 486, 503, 521, 538, 555, 572, 589, 
  606, 623, 640, 657, 674, 691, 708, 725, 742, 759, 776, 793, 
  810, 827, 844, 862, 879, 896, 913, 930, 947, 964, 981, 998, 
  1015};

void portDon(int port = 3){
  port = port+3;
  PORTD |= (1<< port);
 // digitalWrite(port, HIGH);
}

void portDoff(int port = 3){
  port = port+3;
  PORTD &= ~(1<< port); 
 // digitalWrite(port, LOW);
}

void setup() {
  // put your setup code here, to run once:
  //set up the inputs and outputs and write them low using port manipulation
  DDRD = DDRD | B11111000;
  DDRB = B111111;
  PORTD = B00000000;
  PORTB = B00000000;
//Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:
  Clock0.check(); 
  Clock1.check();
  
  transpose = analogRead(3) / 86;
  clockspeed = analogRead(2) >> 2;
  if(clockspeed != oldclockspeed){
    Clock0.setInterval(clockspeed);
    Clock1.setInterval(clockspeed-clockdelay);
    oldclockspeed = clockspeed ;
  }
  
  for(int i = 0; i< 2; i++){
    if( millis()-clockon[i]  > trigTime ){
      portDoff(i);  
    }
  }
}

void clocker0(){
 clockon[0] = millis();
 if(playhead0 < 11  ){
   int out = quantNote(melody[playhead0]);
   dacOutput(out);
   playhead0++;
   
 } else {
   int out = quantNote(melody[playhead0]);
   dacOutput(out);
   dacOutput(out);
  playhead0 = 0; 
 }
 portDon(0);

}

void clocker1(){
 clockon[1] = millis();
 portDon(1);
}

byte quantNote(int v)
{
  int tempVal = vQuant(v);  // decrease the value to 0-64 - ~ a 5 volt range
  tempVal += transpose;      // add the transposition
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


void dacOutput(byte v)
{
  PORTB = (PORTB & B11100000) | (v >> 3);
  PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
}


