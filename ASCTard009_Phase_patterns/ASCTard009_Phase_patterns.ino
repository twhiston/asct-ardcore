///
/// @mainpage	Phase Patterns 
///
/// @details	phase patterns with external clock
/// @n 		
/// @n 
/// @n @a		Developed with [embedXcode+](http://embedXcode.weebly.com)
/// 
/// @author		Ascetic
/// @author		asctSoftware
/// @date		15/05/2014 12:23
/// @version	1.5
/// 
/// @copyright	(c) Ascetic, 2014
/// @copyright	GNU General Public License
///
/// External clock version of phase patterns sketch made for muffwiggler.com user makaton
/// A[0] - Trigger Length
/// A[1] - DAC out 'reich homage' transposition
/// A[2] - 0    = external clock
///        > 0  = internal clock
/// A[3] - Phase amount, be aware that with fast clocks if this is too high it will loose pulses, if its set to 0 you wont get any phase changes and pulses will stay at a contant amount apart
///
///
/// If D[0] isnt in sync with your external clock input unplug and replug the clock input, this should solve the issue


// Include application, user and local libraries
#include <TimedAction.h>

// Prototypes
void isr();
void clocker0();
void clocker1();

inline void portDon(int port = 3);
inline void portDoff(int port = 3);
inline void setClocks(int timeDiff);

byte quantNote(int v);
int vQuant(int v);
void dacOutput(byte v);




// Define variables and constants

//TRIGGER LENGTH
int trigTime = 25;//in millis
long clockon[2] = {0,0};//register clock on lengths for trig time

//TRANSPOSE AMOUNT
int transpose = 0;

//INT CLOCK VALS
int clockspeed = 207;
int oldclockspeed = 207;


//ISR / EXT CLOCK VALS
unsigned long timeNew = 0;
unsigned long timeOld = 0;
volatile unsigned long timeDiff = 0;
unsigned long timeDiffOld = 0;



//PHASE
unsigned int phaseDiff = 1;


//the resolution of the timer is in milliseconds, so 1000 - 1 second actions
TimedAction Clock0 = TimedAction(clockspeed,clocker0);
TimedAction Clock1 = TimedAction((clockspeed-phaseDiff),clocker1);



//melody related values

int playhead0 = 0;
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




//Functions

void setup() {
    // put your setup code here, to run once:
    //set up the inputs and outputs and write them low using port manipulation
    DDRD = DDRD | B11111000;
    DDRB = B111111;
    PORTD = B00000000;
    PORTB = B00000000;
    //Serial.begin(9600);
    attachInterrupt(0, isr, RISING);
    
    
}

int restartFlag = 0;

void isr()
{
    timeOld = timeNew;
    timeNew = millis();
    timeDiff = timeNew-timeOld;
    setClocks(timeDiff);
    if (restartFlag == 1) {
        Clock0.enable();
        Clock1.enable();
        restartFlag = 0;
    }
}

void clocker0()
{
    clockon[0] = millis();
    if(playhead0 < 11  ){
        int out = quantNote(melody[playhead0]);
        dacOutput(out);
        playhead0++;
    } else {
        int out = quantNote(melody[playhead0]);
        dacOutput(out);
        playhead0 = 0;
    }
    portDon(0);
}

void loop()
{
    // put your main code here, to run repeatedly:
    Clock0.check();
    Clock1.check();
    
    //clocking
    
    //if A[2] is on 0 then we are in EXTERNAL clock mode, if A[2] is above that then set the internal clock speed
    static int clockTest = 0;
    static int clockTestOld = -1;
    
    clockTest = analogRead(2);
    
    //if the input is 0 now and the old one was not detatch the interrupt
    if (clockTest == 0 && clockTestOld != 0) {
        //detachInterrupt(0);
        Clock0.disable();
        Clock1.disable();
    } else if (clockTest > 0 && clockTest < 250 && clockTestOld >= 250 ){
        //if the clock is greater than 0 but less than 100 and the old clock is also not in this range
        //enable interrupts
        attachInterrupt(0, isr, RISING);
        restartFlag = 1;
    }else if (clockTest > 0 && clockTest < 250 && clockTestOld == 0 ){
        //if the clock is greater than 0 but less than 100 and the old clock is also not in this range
        //enable interrupts
        restartFlag = 1;
    }else if (clockTest >= 250 && clockTestOld < 250){
        //if it was under 100 but is now over detatch interrupts
        detachInterrupt(0);
    }else if (clockTest >= 250 && clockTestOld >= 250){
        //if its greater than 100 or equal then set the clocks
        if (clockTestOld != clockTest) {
            setClocks(1024-(clockTest-100));
        }
    }

    clockTestOld = clockTest;
    
    
    //Update the phase amount
    static int phaseTest;
    static int phaseTestOld;
    
    phaseTest = analogRead(3) >> 3;
    if(phaseTest != phaseTestOld){
        phaseDiff = phaseTest;
        Clock1.setInterval(timeDiff+phaseDiff);
        phaseTestOld = phaseTest;
    }
    
    //turn off digital outs if needed
    for(int i = 0; i< 2; i++){
        if( millis()-clockon[i]  > trigTime ){
            portDoff(i);
        }
    }
    
    
    //update the trig time
    trigTime = (analogRead(0)>>3)+1;
    
    
    //Look up the transpose, we store the value temporarily to avoid always doing a costly division
    static int tranCheck = 0;
    static int tranCheckOld = 0;
    
    tranCheck = analogRead(1);
    
    //if the value has changed we have to do a division
    if (tranCheck != tranCheckOld) {
        tranCheckOld = tranCheck;
        transpose = tranCheck / 86;
    }
}


void clocker1()
{
    clockon[1] = millis();
    portDon(1);
}

void portDon(int port){
    port = port+3;
    PORTD |= (1<< port);
    // digitalWrite(port, HIGH);
}

void portDoff(int port){
    port = port+3;
    PORTD &= ~(1<< port);
    // digitalWrite(port, LOW);
}

void setClocks(int timeDiff)
{
    
    if (timeDiff != timeDiffOld && timeDiff > 4) {
        Clock0.setInterval(timeDiff);
        Clock1.setInterval(timeDiff+phaseDiff);
        timeDiffOld = timeDiff;
    }
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



