/**
 * Dead City Radio
 * White noise generator at modulation rates
 * A[0] Volume
 * A[1] Pitch
 * A[2] Fine Pitch
 * A[3] smoothing (0-3)
 *
 * If you want an audio speed version of this see the other sketch in the Dead City Radio folder
 *
 * Ardcore expander users: This patch produces some REALLY nice gate patterns out of the dac bits, try it :D
 */
 
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

const int bufferSize = 128;
float noizBuffer[bufferSize];
int bufferPointer = 0;
boolean buffFlag = false;
float noizLevel = 1.0;

#define SPEEDMULTCOUNT 2


void setup()
{
  Serial.begin(9600);
  DDRD = DDRD | B11111000;
  DDRB = B111111;
  PORTD = B00000000;
  PORTB = B000000;
  whitenoise(noizBuffer,bufferSize, 1.0); //generate the first noise buffer
}

int interp;

void loop()
{
  //our pitch vals
  static unsigned long pitchL;
  static int pitchF;
  static unsigned long speedcalc;
  static unsigned long speedcount;
  static unsigned long speedmult;
  if(buffFlag == true){
    whitenoise(noizBuffer,bufferSize, noizLevel);
    buffFlag == false;
  }
  
  noizLevel = analogRead(0)/1024.0;
  //these ranges need improving
  pitchL = (16384-(analogRead(1)<<4))+32;
  pitchF = ((64 -(analogRead(2)>>4)) - 32);
  speedcalc = (pitchL + pitchF);

  interp = analogRead(3)>>8;


  if(speedcount >= speedcalc){
     speedcount = doOutput(speedcalc);
    }else {
      speedcount++;
    }

}

int doOutput(unsigned long speedcal)
{
    //knob range is too big at the moment, and too fast at the top end
    static int multCount = 0;
    int v;
    v = (noizBuffer[bufferPointer]+1)*127;
 
    PORTB = (PORTB & B11100000) | (v >> 3);
    PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
   
    (bufferPointer < bufferSize-1)?bufferPointer++:bufferPointer = 0;
    if(bufferPointer == 0)
      buffFlag = true;
    return 0;
}

long deJitter(long v, long test)
{
  if (abs(v - test) > 1) {
    return v;
  }
  return test;
}

 
 



//make the white noise buffer
float g_fScale = 2.0f / 0xffffffff;
long int g_x1 = 0x67452301;
long int g_x2 = 0xefcdab89;

void whitenoise(
  float* _fpDstBuffer, // Pointer to buffer
  unsigned int _uiBufferSize, // Size of buffer
  float _fLevel ) // Noiselevel (0.0 ... 1.0)
{
  _fLevel *= g_fScale;
  unsigned int bufferMax;
  while( _uiBufferSize-- )
  {
    g_x1 ^= g_x2;
    //do min max stuff
    unsigned int minbuff, maxbuff, minbuff1, maxbuff1;
    if(_uiBufferSize == 0){
        minbuff = bufferMax; 
      } else {
        minbuff = _uiBufferSize - 1;
      }
    if(minbuff == 0){
      minbuff1 = bufferMax;
    } else {
      minbuff1 = minbuff-1;
    }

    if(_uiBufferSize == bufferMax){
      maxbuff = 0;
    } else {
      maxbuff = _uiBufferSize+1;
    }
    if(maxbuff == bufferMax){
      maxbuff1 = 0;
    } else {
      maxbuff1 = maxbuff+1;
    }


    if(interp == 1){
      *_fpDstBuffer = ((g_x2 * _fLevel) + noizBuffer[_uiBufferSize])/2;
    } else if (interp ==2 ){   
      float out =  ((g_x2 * _fLevel) + noizBuffer[_uiBufferSize] + noizBuffer[maxbuff] + noizBuffer[minbuff])/3;
      if(out > 1)
        out = 1;
      if(out < -1)
        out = -1;
      *_fpDstBuffer = out;
    } else if (interp ==3 ){   
       float out = ((g_x2 * _fLevel) + noizBuffer[_uiBufferSize] + noizBuffer[maxbuff] + noizBuffer[minbuff]+ noizBuffer[maxbuff1] + noizBuffer[minbuff1])/4;
      if(out > 1)
        out = 1;
      if(out < -1)
        out = -1;
      *_fpDstBuffer = out;
    } else {
      *_fpDstBuffer = (g_x2 * _fLevel);
    }
    
    g_x2 += g_x1;
    *_fpDstBuffer++;
  }
}

