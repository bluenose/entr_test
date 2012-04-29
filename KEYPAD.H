/*******************
* keypad.h --      *
*  Key definitions *
*******************/

/** Obsolete definitions **/
#define KEY_CLEAR   5             // used in mis routine
#define KEY_BREAK   6             // not used
#define KEY_CHANGE  7             // not used

/************************
* Key value definitions *
************************/

#define KEY_ENTER    1            // J7 - Pin 3
#define KEY_HOME     2            // J7 - Pin 4
#define KEY_UP       3            // J7 - Pin 5
#define KEY_DN       4            // J7 - Pin 6
#define KEY_RO_START 5            // J7 - Pin 7
#define KEY_RO_STOP  6            // J7 - Pin 8

#define KEY_1_A      1            // Version 1.24
#define KEY_1_B      3            // Version 1.24
#define KEY_3_A      7            // Version 1.24
#define KEY_3_B      8            // Version 1.24
#define KEY_5_A      2            // Version 1.24
#define KEY_5_B      4            // Version 1.24
#define KEY_NUM      8

#define KEY_MODE    17

extern long gKeyStuck;
extern char gKeyMatrix[];
