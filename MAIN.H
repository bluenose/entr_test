/*********
* main.h *
**********/

#define FOREVER 1
#define MYSELF  0


/**********************************************************
* Set constant below to TRUE to compile in house version  *
* for testing and setting the serial number. Set to FALSE *
* to compile release version shipping.                    *
***********************************************************/
#define TESTMFG FALSE


/*********************
* MASTER SLAVE INDEX *
**********************/
#define SLAVE  0
#define MASTER 1


/********************
* Option Flags      *
*********************/
#define OP_FORCED   0
#define OP_NOCHEAT  1
#define OPNUM       2       /* Number of Options    */


/****************************
* Display state definitions *
*****************************/
#define DISP_CREDIT       0
#define DISP_PRICE_CHECK  1
#define DISP_OTHER        3


/****************************
* Display chase definitions *
*****************************/
#define CHASE_CIRCLE  1
#define CHASE_DEX     2


/***************************
* bit definitions for the
* OPTION & ADCTL registers.
****************************/
#define ADPU 0x80
#define SCAN 0x20
#define MULT 0x10


/*************
* Prototypes *
**************/
BYTE key_timeout(MCX_TICKS ticks);
BYTE key_timeout_beep(MCX_TICKS ticks);
WORD getmaxprice();
