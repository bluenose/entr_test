/**************
* M E N U . H *
***************/


/******************************
* Menu function return status *
*******************************/
#define RTN_NORMAL   0
#define RTN_EXIT     1
#define RTN_BACK     2


/*********************
* Menu display flags *
**********************/
#define M_MENU          0
#define M_EDTFLAG       1
#define M_EDTPRICE      2
#define M_EDTTIME       3
#define M_EDTYESNO      4
#define M_EDTONOFF      5
#define M_EDTDATE       6
#define M_EDTYEAR       7
#define M_EDTCTIMER     8
#define M_END           9
#define M_REPEAT        10
#define M_INDEX         11
#define M_EDTMISPRICE   12
#define M_EDTMISNUMBR   13
#define M_EDTNUMB       14
#define M_EDTTIMER      15
#define M_SHOWWORD      16


/***********************
* Other menu constants *
************************/
#define FLAG_DIGIT   0
#define FLAG_ONOFF   1
#define FLAG_YESNO   2


/******************
* Clock constants *
*******************/
#define CLK_YEAR 0
#define CLK_DATE 1
#define CLK_TIME 2
#define CLK_STOP 3



/**********************
* Menu Time structure *
***********************/
typedef const struct {
  char *text;
  BYTE item;
  BYTE onoff;
} TMENU;



/*****************
* Flag structure *
******************/
typedef const struct {
  char *text;
  BYTE type;
  BYTE *faddr;
  int  (*fn)();
} FLG;


/*******************
* Number structure *
********************/
typedef struct {
  WORD value;
  WORD min;
  WORD max;
} NUMB;


/*******************
* Number structure *
********************/
typedef struct {
  WORD value;
  WORD min;
  WORD max;
} TIMER;



/*****************
* Menu structure *
******************/
typedef const struct {
  void *  text;
  BYTE    mtype;
  void *  mpnt;
  int (*  fn_exit) ();
  void *  parm_exit;
} MENU;


/*************************
* Menu function routines *
**************************/
int fn_null();
int fn_exit();
int fn_return();
int fn_psw(char *psw);
int fn_menu(MENU *menu);
int fn_tmenu();
int fn_day_all();
int fn_fmenu();
int fn_numb();
int fn_price_all();
int fn_price(WORD itm);
int fn_sts(char *);
int fn_csts();
int fn_setclock();
int fn_stopclock();
int fn_flag(FLG *flg);
int fn_dlst();
int SetPrice();
int fn_fdigit_coke(BYTE *faddr);
int fn_fonoff_coke(BYTE *faddr);
int fn_fonoff(BYTE *faddr);
int fn_fyesno(BYTE *faddr);
int fn_test();
int fn_test_col(char *);
int fn_relay(WORD rdev);
int fn_relay_reset();
int fn_disp();
int fn_mis_reset();
int fn_lang();
int fn_cpo();
int fn_vend(WORD v);
int fnResetErrs(void);

/*******************
* Menu definitions *
********************/
extern MENU  menu_master[];
extern MENU  menu_slave[];


/******************
*  G L O B A L S  *
*******************/
