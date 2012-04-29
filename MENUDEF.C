/**************
* M E N U . C *
***************/
#include "hdrs.h"




/******************
* Menu definition *
*******************/
MENU menu_options[]={
  "NCh ", M_EDTONOFF,  &gOption[OP_NOCHEAT],      0,0,
  "Frcd", M_EDTONOFF,  &gOption[OP_FORCED],       0,0,
  "ShOT", M_EDTNUMB,   &gShakeTimeInHalfSecIntervals, 0, 0,
  "", M_END, 0, 0, 0
};


/******************
* Menu definition *
*******************/
MENU menu_mis[]={
  "CASH", M_EDTMISPRICE, &mis.rs.cash,    0,0,
  "Sold", M_EDTMISNUMBR, &mis.rs.vends,   0,0,
  "RESE", M_MENU,       0, fn_mis_reset,   0,
  "", M_END, 0, 0, 0
};




/******************
* Menu definition *
*******************/
MENU menu_setprice[]={
  "PR-1", M_EDTPRICE, &gPrice[GAL1],  0, 0,
  "PR-3", M_EDTPRICE, &gPrice[GAL3],  0, 0,
  "PR-5", M_EDTPRICE, &gPrice[GAL5],  0, 0,
  "HI  ", M_EDTPRICE, &gMaxPrice,     0, 0,
  "TOCN", M_EDTPRICE, &gPrice[TOKEN], 0, 0,
  "", M_END, 0, 0, 0
};



/******************
* Menu definition *
*******************/
MENU menu_setpour[]={
  "F1-1", M_EDTNUMB, &gGallon[0][GAL1],   0, 0,
  "F1-3", M_EDTNUMB, &gGallon[0][GAL3],   0, 0,
  "F1-5", M_EDTNUMB, &gGallon[0][GAL5],   0, 0,
  "F2-1", M_EDTNUMB, &gGallon[1][GAL1],   0, 0,
  "F2-3", M_EDTNUMB, &gGallon[1][GAL3],   0, 0,
  "F2-5", M_EDTNUMB, &gGallon[1][GAL5],   0, 0,
  "SECS", M_EDTNUMB, &gPourTimerMax,      0, 0,
  "", M_END, 0, 0, 0
};



/******************
* Menu definition *
*******************/
MENU menu_test[]={
  "t1-1", M_MENU,    0,    fn_vend, (WORD *)0,
  "t1-3", M_MENU,    0,    fn_vend, (WORD *)1,
  "t1-5", M_MENU,    0,    fn_vend, (WORD *)2,
  "t2-1", M_MENU,    0,    fn_vend, (WORD *)3,
  "t2-3", M_MENU,    0,    fn_vend, (WORD *)4,
  "t2-5", M_MENU,    0,    fn_vend, (WORD *)5,
  "", M_END, 0, 0, 0
};


/******************
* Menu definition *
*******************/
MENU menu_setro[]={
  "GAL ", M_EDTNUMB,  &gROgalmax,      0, 0,
  "INTR", M_EDTTIMER, &gBackFlushInt,  0, 0,
  "LENG", M_EDTTIMER, &gBackFlushTime, 0, 0,
  "", M_END, 0, 0, 0
};

/////////////////////////////
// Test Menu for Debugging //
/////////////////////////////
#if DEBUG_TEST
  MENU menu_errors[]={
    "CNT ", M_SHOWWORD, &gExitType[COUNTER_EXIT],     0,  0,
    "TINE", M_SHOWWORD, &gExitType[TIMER_EXIT],       0,  0,
    "DRTY", M_SHOWWORD, &gExitType[DIRTY_EXIT],       0,  0,
    "NPLS", M_SHOWWORD, &gExitType[NOPLS_EXIT],       0,  0,
    "CLR ", M_MENU,     0,                  fnResetErrs,  0,
    "",     M_END,      0,                            0,  0
  };
#endif

/******************
* Menu definition *
*******************/
MENU menu_master[]={
  "ACCT", M_MENU,  menu_mis,      0,0,
  "PRIC", M_MENU,  menu_setprice, 0,0,
  "POUR", M_MENU,  menu_setpour,  0,0,
  "tESt", M_MENU,  menu_test,     0,0,
  "OPT ", M_MENU,  menu_options,  0,0,
  "RO  ", M_MENU,  menu_setro,    0,0,
#if DEBUG_TEST
  "ERRS", M_MENU,  menu_errors,   0,0,
#endif
  "", M_REPEAT, 0, 0, 0
};


/******************
* Menu definition *
*******************/
MENU menu_slave[]={
  "ACCT", M_MENU,  menu_mis,      0,0,
  "PRIC", M_MENU,  menu_setprice, 0,0,
  "POUR", M_MENU,  menu_setpour,  0,0,
  "tESt", M_MENU,  menu_test,     0,0,
  "OPT ", M_MENU,  menu_options,  0,0,
#if DEBUG_TEST
  "ERRS", M_MENU,  menu_errors,   0,0,
#endif
  "", M_REPEAT, 0, 0, 0
};

