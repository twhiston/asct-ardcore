/*
 * User Settings
 * This file lists all variables that users can change to get different program function
 */

/*
 * Pattern File
 * include the name of the file with all your patterns here
 */
#include "patterns.h"

/*
 * A[5]: User Assignable Control Destination
 * A[5] is an input for modulation. A[4] can be used to select the channel to route the modulation to
 * The modulation destination parameter is defined below from these choices
 * user_PATTERN
 * user_SPEED
 * user_SHUFFLE_FREQ
 * user_SHUFFLE_AMT
 * user_NONE
 */
 #define user_PATTERN

/*
 * LOAD DEFAULTS
 * if this IS defined the program will load the default blank array on reset, else it will load preset 0
 */
#define SDEFAULTS

/*
 * CLOCK VALUES
 * This array contains the values that the clock can be set to. Below is a table of values and their associated intervals
 * The largest value in the array CANNOT be greater than 3072.
 * The array must be 32 values long
 * 
 * Be Aware that the 'default settings load value 13 from this array into the settings because it equals 48
 */
unsigned const int clkVals[24] = { 0,6144,3072,1536,1024,768,512,384,256,192,128,96,64,48,32,24,16,12,8,6,4,3,2,2};
/*
 * TIP: Set a few values next to each other to be the same number to create a 'wider' control area for that value, 
 * this is useful to create a few areas that are easier to find on the dial
 * 
 * unsigned const int clkVals[24] = { 0,1024,768,768,384,384,192,192,128,96,96,48,48,48,24,24,24,32,12,12,6,4,2,2};
 */

/*
 * Hardsync
 * Alternate sync method which will be work more accurately but if the input clock stops the output clock will stop as well
 * use this for some bulletproof sync timing if you know you have a stable clock input
 */
#define HARDSYNC

/*
 * Hardsync division
 * Lets you specify how many clk inputs it takes to trigger a clock high
 * clock should be twice the speed of your fastest division! (so a 48ppq clock is ideal)
 */
const int hDiv = 1;


/*
 * RESET EEPROM
 * YOU MUST LEAVE THIS IN THE FIRST TIME YOU UPLOAD THE SKETCH
 * also if you change the EEPROM offset in advanced settings run this again
 * After that you can undefine it to retain your eeprom settings after power cycle
 */
#define EEPROMreset


/*
 * PRESET LOAD ONLY
 * If this is defined you can load presets without having to change the save/load knob position (ie they load instantly when you change the preset knob. In this mode you CANNOT SAVE
 * This mode is mainly here as its useful for playing live where you dont want to accidentally save over your presets
 */
//#define LOADONLY



/*
 * ADVANCED SETTINGS
 *
 * These control some more advanced settings of the program, change them at your own peril!
 *
 */

/*
 * Knob Pickup
 * Comment out this line to turn off knob pickup
 */
#define PICKUP 

/*
 * ADC Settings
 */
unsigned const char settleTime = 2; //settle time for the ADC when changing inputs.
				    //default = 2. If you get jitter on your inputs or they wobble between values try increasing this

/*
 * EEPROM max writes per session (reset the ardcore to reset the count)
 * its set at a very conservative value at the moment, feel frre to increase at will (the EEPROM on the arduino is rated at about 100,000 write operations)
 * though this article claims much higher real world stats http://tronixstuff.com/2011/05/11/discovering-arduinos-internal-eeprom-lifespan/
 */
unsigned const int allowedWrites = 50;

/*
 * EEPROMoffset
 * To extend the life of your EEPROM you could consider making the sketch write to different addresses (since the presets do not fill the EEPROM)
 * our EEPROM is 1024 bytes.
 * Each preset is 68 bytes and we have 8*68 bytes = 544 total
 * This is just over half so we will always be writing into one area that we wrote before, but we can probably assume that we write preset 8 less than preset 1
 * max value for this could be 480
 */ 
 unsigned const int EEPROMoffset = 0;
 
/*
 * Variance
 * variance is how much around the calculated tempo we should log but not act upon
 */
 unsigned const variance = 200;
 
 /*
 * Variance Count
 * How many variances should occur before a reset
 */
 unsigned const varianceMax = 8;
 
 /*
 * A4 and A5 MIDDLE
 * Set the value that your A4 control is when you have no input, this is important to get the modulation on a5 working right
 */
 #define A4middle 131
 #define A5middle 131
 
