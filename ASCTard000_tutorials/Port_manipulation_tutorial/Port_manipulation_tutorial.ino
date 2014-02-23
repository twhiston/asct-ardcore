  
/*
* PORT MANIPULATION
*
* How to write directly to a port
*
* This uses a ternary statement to output a value to a pin which turns the lights on and off in sequence, see code for explanation
*
*/

  
void setup() {

//pin 0 of a register is on the far right of the string below, so this write port D to be output for pins 3-7. Port D corresponds to b2,b1,b0,d1,d0,clock in,tx,rx (tx and rx should not be changed)
DDRD = DDRD | B11111000;
//this sets all of port B to be outputs, Port B corresponds to 13,b7,b6,b5,b4,b3
DDRB = B111111;
//this writes all our post values to low for now
PORTD = B00000000;
PORTB = B000000;
}

void loop() {
  
  //first turn the lights on one at a time
  for(int i = 0; i<10;i++){
  (i < 5) ? PORTD |= (1<< i+3) : PORTB |= (1<<i-5); //write the output high
  /*
  * the above statement is an inline if statement which we write this way for convenience
  * portd 3-7 = d0,d1,b0,b1,b2
  * portb 0-5 = b3,b4,b5,b6,b7
  */ 
  
  delay(30);
    }

  
  
  for(int i = 0; i<10;i++){
  (i < 5) ? PORTD &= ~(1<< i+3) : PORTB &= ~(1<<i-5); //write output low
  /*
  * another inline if statement. This time we are turning things off, note the difference in the statement
  * our statement to turn things on is:
  * PORTD |= (1<< port) where port is a number between 0 and 7
  * to turn them off this becomes
  * PORTD &= ~(1<< port)
  */
  delay(30);
  }
  

}


