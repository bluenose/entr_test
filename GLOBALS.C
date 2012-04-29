/*********************************
* globals.c - Global definitions
**********************************/
#include "hdrs.h"

BYTE gMaster;           /* Master flag                    */
WORD gWaterCount[2];    /* Generic counter                */
BYTE gGalSolCount[2];   /* Gallons for Solenoids counter  */
BYTE gVendTaskActive[2];/* Active Vend task               */
BYTE gSlaveGalCount;    /* Slave Gallon count             */
BYTE gMeterPulse[2];    /* Transition counter             */
WORD gVendFailTimer[2]; /* 5 minute Fail timer            */
WORD gVendTimer[2];     /* General vend timer             */
BYTE gSwitch[SW_MAX];   /* Switch states                  */
BYTE rsrc_AD;           /* resource flag                  */
BYTE rsrc_SCI;          /* resource flag                  */
SCI_BLOCK sci;          /* sci structure                  */
MDBBILL bill;           /* mdb bill structure             */
DEX dex;                /* dex structure                  */
WORD beep;              /* beeper word                    */
BYTE keypadchar;        /* received keypad char           */
BYTE gServiceMode;      /* True if in service mode        */
BYTE gDoor;             /* True when Door is open         */
WORD gServiceTimeout;   /* Timer for service timeout      */
BYTE gZeroFill;         /* force zero filling of price display */
DEBOUNCE gDebDex;       /* Dex detect structure           */
BYTE gChase;            /* Display chase flag             */
BYTE gBill_accepted;    /* Bill accepted flag             */
BYTE gCalcExact;        /* Calculate Exact change needs   */
BYTE gCoinCount[16];    /* Coin count work area           */
BYTE gAllSoldOut;       /* Everything is sold out         */
BYTE gReturnChange;     // Flag that is set to return change after a vend
BYTE gCoinToken;        // Keeps track of the number of tokens
BYTE gReturnTime;       // Five second timer used to return change
BYTE gFirstTry;
BYTE gNoPulseTimer[2];   // used to determine if vend cycle is functioning properly
BYTE gPasswordValidated; // Used for remembering password validity
