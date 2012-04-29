/*********
* VEND.H *
**********/


#define STATUS_OK       0
#define STATUS_ERR1     1
#define STATUS_ERR2     2
#define STATUS_NOMOTOR  10
#define STATUS_JAM      11
#define STATUS_SO       13



/************************
*  V e n d   F l a g s  *
*************************/
#define VEND_OK   0
#define VEND_FAIL 1
#define VEND_SO   2
#define VEND_HAND 3
#define VEND_BUSY 4

#define GAL1  0
#define GAL3  1
#define GAL5  2
#define TOKEN 3

// Constants
#define VEND_FAIL_DIRTY     5
#define VEND_FAIL_NOPULSES  2 


// Test debug
#define COUNTER_EXIT  0
#define TIMER_EXIT    1
#define DIRTY_EXIT    2
#define NOPLS_EXIT    3
