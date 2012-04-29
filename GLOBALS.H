/*********************************
* globals.h - Global definitions
**********************************/

extern  BYTE gMaster;           /* Master flag                    */
extern  WORD gWaterCount[2];       /* Generic counter                */
extern  BYTE gGalSolCount[2];   /* Gallons for Solenoids counter  */
extern  BYTE gVendTaskActive[2];/* Active Vend task               */
extern  BYTE gSlaveGalCount;    /* Slave Gallon count             */
extern  BYTE gMeterPulse[2];    /* Transition counter             */
extern  WORD gVendFailTimer[2]; /* 5 minute Fail timer            */
extern  WORD gVendTimer[2];     /* General vend timer             */
extern  BYTE gSwitch[SW_MAX];   /* Switch states                  */
extern  BYTE rsrc_AD;           /* resource flag                  */
extern  BYTE rsrc_SCI;          /* resource flag                  */
extern  SCI_BLOCK sci;          /* sci structure                  */
extern  MDBBILL bill;           /* mdb bill structure             */
extern  DEX  dex;                /* dex structure                  */
extern  WORD beep;              /* beeper word                    */
extern  BYTE keypadchar;        /* received keypad char           */
extern  BYTE gServiceMode;      /* True if in service mode        */
extern  BYTE gDoor;             /* True when Door is open         */
extern  WORD gServiceTimeout;   /* Timer for service timeout      */
extern  BYTE gZeroFill;         /* force zero filling of price display */
extern  DEBOUNCE gDebDex;       /* Dex detect structure           */
extern  BYTE gChase;            /* Display chase flag             */
extern  BYTE gBill_accepted;    /* Bill accepted flag             */
extern  BYTE gCalcExact;        /* Calculate Exact change needs   */
extern  BYTE gCoinCount[16];    /* Coin count work area           */
extern  BYTE gAllSoldOut;       /* Everything is sold out         */
extern  BYTE gReturnChange;     // Flag that is set to return change after a vend
extern  BYTE gCoinToken;        // Keeps track of the number of tokens
extern  BYTE gReturnTime;       // 5 Second timer used to return change
extern  BYTE gFirstTry;         // flag used to monitor if a vend has been completed since credit was 0
extern  BYTE gNoPulseTimer[2];  // used to determine if vend cycle is functioning properly
extern  BYTE gPasswordValidated; // Used for remembering password validity