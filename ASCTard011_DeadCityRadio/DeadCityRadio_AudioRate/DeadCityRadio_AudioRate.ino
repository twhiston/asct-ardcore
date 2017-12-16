/**
 * Dead City Radio
 * White noise generator at audio rate
 * A[0] Volume
 * A[1] Pitch
 * A[2] Fine Pitch
 * A[3] smoothing (0-3)
 *
 * If you want an modulation speed version of this see the other sketch in the Dead City Radio folder
 *
 */
 
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

const int bufferSize = 128;
float noizBuffer[bufferSize];
int bufferPointer = 0;
boolean buffFlag = false;
float noizLevel = 1.0;


int interp;



 ISR(TIMER1_COMPA_vect) {
   
   int v;
      v = (noizBuffer[bufferPointer]+1)*127;
 

   PORTB = (PORTB & B11100000) | (v >> 3);
   PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
   
   (bufferPointer < bufferSize-1)?bufferPointer++:bufferPointer = 0;
   if(bufferPointer == 0)
     buffFlag = true;
   
 }
 
 
 void interruptSetup() {
    // initialize Timer1
    cli(); // disable global interrupts
    TCCR1A = 0; // set entire TCCR1A register to 0
    TCCR1B = 0;
    // Phase and Freq correct PWM, top = OCR1A
    OCR1A = 200;
    
    TCCR1A |= (1 << WGM10);
    TCCR1B |= (1 << WGM13);
    TCCR1B |= (1 << CS11);//prescale 8
    TIMSK1 |= (1 << OCIE1A);
    //external interrupt 0 on rising edge
    sbi(EICRA, ISC00);
    sbi(EICRA, ISC01);
    //enable external interrupt on pin2 
   //attachInterrupt(0, isr, RISING);
    // enable global interrupts:
    sei();
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

void dacOutput(byte v)
{
  PORTB = (PORTB & B11100000) | (v >> 3);
  PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
}

void setup()
{
  Serial.begin(9600);
  DDRD = DDRD | B11111000;
  DDRB = B111111;
  PORTD = B00000000;
  PORTB = B000000;
  whitenoise(noizBuffer,bufferSize, 1.0); //generate the first noise buffer
  interruptSetup();
}

void loop()
{
  //our pitch vals
  static unsigned long pitchL;
  static int pitchF;
  
  if(buffFlag == true){
    whitenoise(noizBuffer,bufferSize, noizLevel);
    buffFlag == false;
  }
  
  noizLevel = analogRead(0)/1024.0;
  //theres a bit of a balancing act stopping these values increasing to the point where it freezes the ardcore
  //sorry for the magic numbers!
  pitchL = (8184 - (analogRead(1)<<3) + 527);
  pitchF = (256 - (analogRead(2)>>2) -512);
  long pitchcalc = (pitchL + pitchF)*2;
  pitchcalc = (pitchcalc < 20)? 20: pitchcalc;
  OCR1A = pitchcalc;
  
  interp = analogRead(3)>>8;
  
  //we need to write an interp function, maybe it can store a current position, work out the values around it for the required routine and then do them
  //for example if its set to 4 it gets the current index then calculates the next 2 index's and the previous 2
  //then add them all together divide by 5 and do the normal scaling before  output
  
  
  
}
