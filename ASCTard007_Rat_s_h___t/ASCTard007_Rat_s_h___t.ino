 /*  ============================================================
  *
  *  Program: ASCTard007 - Rat(s)h(*)t
  *
  *  Description: 
  *              Based very loosely on the buchla eardrill pendulum/ratchet module
  *              Tries to replicate the interesting outputs on the unit (A and B)
  *              Theres loads of ways tomake this better once the ardcore expander comes out so expect some serious additions then
  *              NEEDS A FAST CLOCK - theres so much division going on you need to feed it a fast clock to get useful results
  *
  *
  *  I/O Usage:
  *    A0: Internal Clock Division (ccw to 12 o'clock = count % division (1-128 so this can get VERY slow), 12 to CW  primes, fibonacci, Fermats Litle Theorum, Recaman )
  *    A1: Random Voltage Max
  *    A2: DO1 mode (ccw to 12 o'clock = count % division (1-64), 12 to CW  primes, fibonacci, Fermats Litle Theorum, Recaman )
  *    A3: DO2 mode (ccw to 12 o'clock = count % division (1-64), 12 to CW  primes, fibonacci, Fermats Litle Theorum, Recaman )
  *    Digital Out 1: trigger when internal clock matches DO1 mode value
  *    Digital Out 2: trigger when internal clock matches DO2 mode value
  *    Clock In: clock input
  *    Analog Out: Random Voltage on d1 out
  *
  *
  *  Created:  04/04/2013
  *
  */
  
  /* ARDCORE SETUP */

unsigned int Primes[]  =
{
    2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113,127
    
};

const int PrimesSize = sizeof(Primes) / sizeof(int);

unsigned int Fib[]  =
{
    0,1,2,3,5,8,13,21,34,55,89
    
};

const int FibSize = sizeof(Fib) / sizeof(int);

unsigned int FermatLittle[] =
{
    1, 6, 1, 8, 0, 3, 3, 9, 8, 8, 7, 4, 9, 8, 9, 4, 8, 4, 8, 2, 0, 4, 5, 8, 6, 8, 3, 4, 3, 6, 5, 6, 3, 8, 1, 1, 7, 7, 2, 0, 3, 0, 9, 1, 7, 9, 8, 0, 5, 7, 6, 2, 8, 6, 2, 1, 3, 5, 4, 4, 8, 6, 2, 2, 7, 0, 5, 2, 6, 0, 4, 6, 2, 8, 1, 8, 9, 0, 2, 4, 4, 9, 7, 0, 7, 2, 0, 7, 2, 0, 4, 1, 8, 9, 3, 9, 1, 1, 3, 7, 4, 8, 4, 7, 5
};

const int FermatSize = sizeof(FermatLittle) / sizeof(int);
int FermatCounter[2] = {0,0}; //counts where we are in the sequence

unsigned int Recaman[] =
{
 	0, 1, 3, 6, 2, 7, 13, 20, 12, 21, 11, 22, 10, 23, 9, 24, 8, 25, 43, 62, 42, 63, 41, 18, 42, 17, 43, 16, 44, 15, 45, 14, 46, 79, 113, 78, 114, 77, 39, 78, 38, 79, 37, 80, 36, 81, 35, 82, 34, 83, 33, 84, 32, 85, 31, 86, 30, 87, 29, 88, 28, 89, 27, 90, 26, 91
};

const int RecamanSize = sizeof(Recaman) / sizeof(int);



//  constants related to the Arduino Nano pin use
const int clkIn = 2;           // the digital (clock) input
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)
const int trigTime = 2;       // 25 ms trigger timing

//  variables for interrupt handling of the clock input
volatile int clkState = LOW;
volatile int clkState2 = LOW;
int clkDivide = 0;
int clkTime[2] = {0,0};
int burstRate = 0;
int burst = 0;

//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};        // start with both set low
unsigned long digMilli[2] = {0, 0};  // a place to store millis()


int inValue = 0;       // the input value
int a[4] = {0,0,0,0}; //input vals
int go[4] = {0,0,0,0}; //does something need updating
int tempval = 0;

int dmodestore[2] = {1, 1};

//output voltage stuff
int randomMax = 255; //max output
int randomOut = 0; //the current output


//val for the density value of the clock to be passed to the counter
int density = 1;
//the current clock count
int counter = 0;
//post divisor clock count
int divclock = 0;
int olddivclock = 0; //the old divclock value
//small counter for fermatlittle, value 0 for initial loop, value 1 for output loop
int littlecounter[2] = {0,0};

void setup(){
  
  /* Debug */
 // Serial.begin(9600);
 randomSeed(analogRead(4));
  /*Ardcore Setup */
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
  
  // set up an interrupt handler for the clock in.
  // Note: Interrupt 0 is for pin 2 (clkIn)
  attachInterrupt(0, isr, RISING);
  
}

void loop(){
  tempval = 0;
  olddivclock = divclock;
  //here we use the first clock and our routines to set up the second clock which
 if(clkState == HIGH){
   clkState = LOW;
   //update the counter
   if(counter <= 126){
    counter++;
  }else if(counter >= 127){
    counter = 0;
  }
 //check the count against the mode and do stuff
 if(density <= 127){
   if(IsDivisible(counter, density)){
     clkState2 = HIGH;
   }
 } else {
  //else we do some funky table stuff we have up to 256 to play with 
  
  //primes
  if(density > 127 && density < 160){
     tempval = PrimesCheck(counter);
    if(tempval == 2 || tempval == 1){
       //Serial.println("RESET RETURNED - reset divclock");
       clkState2 = HIGH;
    }
   }
   
   //fibonacci
  if(density >= 160 && density < 180){
     tempval = FibCheck(counter);
    if(tempval == 2 || tempval == 1){
      clkState2 = HIGH;
    }
   }
   
   //we do things a little differently in the functions for the fermat and Recaman as they have repeated numbers
   
   //Fermat
   if(density > 180 && density < 210){
     tempval = FermatCheck(counter , 0);
    if(tempval == 2 || tempval == 1) {
     clkState2 = HIGH;
    }
   }
   
   //Recaman
   if(density > 210 && density < 256){
     tempval = RecamanCheck(counter);
    if(tempval == 2 || tempval == 1) {
     clkState2 = HIGH;
    // Serial.println("RECAMAN 1 RETURNED - add to divclock");
     //Serial.println(divclock);
    }
   }
   
 

 
 } //end of setting initial clock divisions
 } //end of external clock high
 

//check if the internal clock is high
 if(clkState2 == HIGH){
clkState2 = LOW;
//change the random voltage
randomOut = random(randomMax);


//now do clocking stuff
if(divclock <= 126){
    divclock++;
  }else if(divclock >= 127){
    divclock = 0;
  }
 //for each  dmodestore value we do something
 for (int i=0; i<2; i++) {
    if(dmodestore[i] <= 34){
      //if the divclock is divisible by the mode number output is high
      if(IsDivisible(divclock, dmodestore[i])){
         digitalWrite(digPin[i], HIGH);
         digState[i] = HIGH;
         digMilli[i] = millis();
      }
    }
    //do it for primes
    if(dmodestore[i] > 34 && dmodestore[i] < 41){
     tempval = PrimesCheck(divclock);
      if(tempval == 1 || tempval == 2){
          digitalWrite(digPin[i], HIGH);
          digState[i] = HIGH;
          digMilli[i] = millis();
    }
   }
   
   //fibonacci
  if(dmodestore[i] >= 41 && dmodestore[i] < 48){
     tempval = FibCheck(divclock);
     if(tempval == 1 || tempval == 2){
          digitalWrite(digPin[i], HIGH);
          digState[i] = HIGH;
          digMilli[i] = millis();
    }
   }
   
   //we do things a little differently in the functions for the fermat and Recaman as they have repeated numbers
   
   //Fermat
   if(dmodestore[i] >= 48 && dmodestore[i] < 55){
     tempval = FermatCheck(divclock, 1);
     if(tempval == 1 || tempval == 2){
          digitalWrite(digPin[i], HIGH);
          digState[i] = HIGH;
          digMilli[i] = millis();
    }
   }
   
   //Recaman
   if(dmodestore[i] >= 55 && dmodestore[i] < 65){
     tempval = RecamanCheck(divclock);
 if(tempval == 1 || tempval == 2){
          digitalWrite(digPin[i], HIGH);
          digState[i] = HIGH;
          digMilli[i] = millis();
    }
   }
   
 if(digState[0] == HIGH){
  dacOutput(randomOut); 
 }
 
 
 } //end of each digi out
 } //end of if clk2 high


 
 
  //update all the knobs
  for(int i = 0; i <=3; i++){
  //get the density
  tempval = deJitter(analogRead(i), a[i]);
  if (tempval != a[i]) {
  a[i] = tempval;
  //set our go values to do stuff
  go[i] = 1;
  } else {
    go[i] = 0;
  }
  //reset our temp val to stop us having to declare it in the loop
  tempval = 0;
  }
  //if any of them are noted as changed update the values
   if(go[0] == 1){
    setDensity(a[0]);
   } 

   if(go[1] == 1){
    SetRandomMax(a[1]);
   } 
   
   if(go[2] == 1){
    dMode(0, a[2]);
   } 
   
   if(go[3] == 1){
    dMode(1, a[3]);
   } 
   
    // do we have to turn off any of the digital outputs?
  for (int i=0; i<2; i++) {
    if ((digState[i] == HIGH) && (millis() - digMilli[i] > trigTime)) {
      digState[i] = LOW;
      digitalWrite(digPin[i], LOW);
    }
  }

}

int setDensity(int x){
  //set the density amount
  density = (x>>2)+1;

 //some sanity checking for patterns
// if(density <= 127){
//     Serial.print("MASTER DO divclock % divmodestore[i] ");
//      Serial.println(density);
//   }
//  
//  //primes
//  if(density > 127 && density < 160){
//    Serial.println("MASTER Primes");
//   }
//  
//  if(density >= 160 && density < 180){
//    Serial.println("MASTER Fibbo");
//   }
//   
//   //Fermat
//   if(density >= 180 && density < 210){
//     Serial.println("MASTER Fermat");
//   }
//   
//   //Recaman
//   if(density >= 210 && density < 256){
//     Serial.println("MASTER Recaman");
//    }
   
  
}

int SetRandomMax(int x){
  randomMax = x>>2;
}

int dMode(int d, int x){
 //set the d mode here

  dmodestore[d] = (x>>4)+1;

//sanity check
// if(dmodestore[d] <= 34){
//      Serial.print("D");
//      Serial.print(d);
//      Serial.print(" divclock % divmodestore[i] ");
//      Serial.println(dmodestore[d]);
//    }
//    //do it for primes
//    if(dmodestore[d] > 34 && dmodestore[d] < 41){
//      Serial.print("D");
//      Serial.print(d);
//      Serial.println(" Primes");
//   }
//   
//   //fibonacci
//  if(dmodestore[d] >= 41 && dmodestore[d] < 48){
//      Serial.print("D");
//      Serial.print(d);    
//    Serial.println(" Fibbo");
//   }
//   
//   //we do things a little differently in the functions for the fermat and Recaman as they have repeated numbers
//   
//   //Fermat
//   if(dmodestore[d] >= 48 && dmodestore[d] < 55){
//           Serial.print("D");
//      Serial.print(d);   
//     Serial.println(" Fermat");
//
//   }
//   
//   //Recaman
//   if(dmodestore[d] >= 55 && dmodestore[d] < 65){
//                Serial.print("D");
//      Serial.print(d);   
//     Serial.println(" Racaman");
//
//   }
}

//all these functions return states 0, 1, 2
//0 - no change, 1 - change, 2 - reset

int PrimesCheck(int counter){
  
      for(int i = 0; i <= PrimesSize; i++){
       if(Primes[i] == counter && divclock <=126){
         //advance the counter
         return 1;
       } else if (Primes[i] == counter && divclock >= 127){
         //reset the counter
         return 2;
       }
     }
    return 0; 
  
}

int FibCheck(int counter){
      for(int i = 0; i <= FibSize; i++){
       if(Fib[i] == counter && divclock <=126){
         //advance the counter
         return 1;
       } else if (Fib[i] == counter && divclock >= 127){
         //reset the counter
         return 2;
       }
     }
    return 0; 
  
}

int FermatCheck(int counter, int mode){
    //if mode 0 we are using in initial clock loop, if 1 in output
    
  //as the fermat list has repeated number we do things a bit differently, 
  //the fermat array has its own counter that we advance each time and check against that item in the array
  //also this array only has numbers between 0 - 10 so we use smallcounter to cycle through this
     if(FermatLittle[FermatCounter[mode]] == littlecounter[mode] && littlecounter[mode] < 10){
       //if the count = the step we are looking at we advance the count and more to the next array step
       FermatCountUpdate(mode);
       LittleCountUpdate(mode);
       return 1;
     } else if(FermatLittle[FermatCounter[mode]] == littlecounter[mode] && littlecounter[mode] >= 10){
    LittleCountUpdate(mode);
    return 2;
    }
    LittleCountUpdate(mode);
    return 0;
}

int FermatCountUpdate(int mode){
  if(FermatCounter[mode] < FermatSize){
    FermatCounter[mode]++;
  } else if(FermatCounter[mode] == FermatSize){
   FermatCounter[mode] = 0;
  }
}

int LittleCountUpdate(int mode){
  if(littlecounter[mode] < 10){
    littlecounter[mode]++;
  } else if(littlecounter[mode] >= 10){
   littlecounter[mode] = 0;
  }
}

int RecamanCheck(int counter){
  
      for(int i = 0; i <= PrimesSize; i++){
       if(Recaman[i] == counter && divclock <=126){
         //advance the counter
         return 1;
       } else if (Recaman[i] == counter && divclock >= 127){
         //reset the counter
         return 2;
       }
     }
    return 0; 
  
}

void isr()
{
  clkState = HIGH;
}

int IsDivisible(int x, int n)
{
    return (x % n) == 0;
}


int deJitter(int v, int test)
{
  if (abs(v - test) > 3) {
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
