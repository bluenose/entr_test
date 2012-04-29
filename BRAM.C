/*********************************
* bram.c - battary backed ram
*********************************/
#include "hdrs.h"

WORD bramvalid;                     /* bram valid word        */
WORD gROpostTimer;                  /* Countup second timer   */
WORD gROtimerOff;                   /* Four hour second timer */
BYTE gROstate;                      /* State of RO maintnance */
NUMB gPourTimerMax={60,1,120};      /* Max time to pour water */
WORD gPourTimer[2];                 /* Pour timer             */
WORD gROTimer;                      /* RO On timer            */
WORD gGallonCount;                  /* Total Gallons served   */
WORD gROgalcnt;                     /* RO gallon count        */
NUMB gROgalmax={25,5,80};           /* RO gallon max          */
NUMB gShakeTimeInHalfSecIntervals={4,0,20}; /* Shake time     */ 
TIMER gBackFlushInt={3600,0,5999};    /* Back flush interval    */
TIMER gBackFlushTime={300,0,5999};  /* Back flush time        */
NUMB gGallon[2][3]={

  1380,1,9999,  4300,3,9999,  7125,5,9999,
  1380,1,9999,  4300,3,9999,  7125,5,9999
  };
/*
NUMB gGallon[2][3]={

  5,1,9999,  5,3,9999,  5,5,9999,
  5,1,9999,  5,3,9999,  5,5,9999
  };
*/
int  gWaterCounter[2];              /* Water counter          */
BYTE gGalSelect[2];                 /* Selected gallons       */
MDBCOIN coin;                       /* mdb coin structure     */
BYTE gOption[OPNUM];                /* Option flags           */
WORD gCredit;                       /* Customer credit        */
WORD gExitType[4];                  // Test - debug for error codesMIS  mis
MIS  mis;
