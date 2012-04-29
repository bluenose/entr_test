/******************************************
* bram.h - battery backed RAM definitions
*******************************************/


extern  WORD bramvalid;         /* bram valid word        */
extern  WORD gROpostTimer;      /* Countup second timer   */
extern  WORD gROtimerOff;       /* Four hour second timer */
extern  BYTE gROstate;          /* State of RO maintnance */
extern  NUMB gPourTimerMax;     /* Max time to pour water */
extern  WORD gPourTimer[2];     /* Pour timer             */
extern  WORD gROTimer;          /* RO On timer            */
extern  WORD gGallonCount;      /* Total Gallons served   */
extern  WORD gROgalcnt;         /* RO gallon count        */
extern  NUMB gROgalmax;         /* RO gallon max          */
extern  NUMB gShakeTimeInHalfSecIntervals; /* Shake time  */
extern  TIMER gBackFlushInt;    /* Back flush interval    */
extern  TIMER gBackFlushTime;   /* Back flush time        */
extern  NUMB gGallon[2][3];     /* one & five gallon count */        // Version 1.24
extern  int gWaterCounter[2];   /* Water counter          */
extern  BYTE gGalSelect[2];     /* Selected gallons       */
extern  MDBCOIN coin;           /* mdb coin structure     */
extern  BYTE gOption[OPNUM];    /* Option flags           */
extern  WORD gCredit;           /* Customer credit        */
extern  WORD gExitType[4];      // Test - debug for error codes
extern  MIS  mis;

