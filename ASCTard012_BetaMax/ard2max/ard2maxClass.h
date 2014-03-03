//
//  ard2max.h
//  
//
//  Created by Ascetic on 24/02/2014.
//
//

#ifndef ____ard2max__
#define ____ard2max__

#include "Arduino.h"

//These are all ascii character codes we use for stuff
#define XON 17
#define XOFF 19
#define HANDSHAKE 6
#define CLKID 7
//----------------------
#define MAXDACOUT 255   //the dac goes to 255 max
#define BUFFSIZE 7      //the size of the input buffer
#define SETTLETIME 32   /**
                         *Default - 32, this is how long we need to leave the dac to get a proper conversion. 
                         *You can try making this faster but if its too fast all the channels results will be wrong and will jump around and the serial bus will get flooded
                         *32 seemed about right for a fast but stable result in testing, because its so liable to break the output if set wrongly there is no interface to this value
                         */

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))


class ArdToMax
{
public:
    ArdToMax();

    void getInputs();
    void clkOut();
    void begin();
    void begin(long i);
    int establishContact();
    int available();
    void hasExpander(boolean has = false);
    void setDejitter(boolean on = true, int amount = 1);
    void setDejitter(int on = 1);
    void read();

private:

    void setupDac();
    int deJitter(int v, int test);
    void sendValueAsTwo7bitBytes(int value);
    void setInputCount(int nCount = 0);
    void outputRouter(char *input);
    void digiOut(char *input);
    void dacOut(char *input);
    void dacOutput(byte v);
    
    Stream *ArdStream; //this is our output stream, write to this to send serial data
    boolean expander;
    char input[32];//CAN OPTIMIZE HERE, HOW SMALL CAN WE MAKE THIS WITHOUT MESSAGES CRASHING IT
    int data[BUFFSIZE]; //serial read buffer
    byte pB;
    byte pD;
    int readIn;
    char aIn;
    int inputsMax;
    boolean deJitState;
    int deJitterVal;
};

#endif /* defined(____ard2max__) */
