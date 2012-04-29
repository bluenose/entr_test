/**************
 * display.c
 **************/

#include  "hdrs.h"


#define DP   0x01
#define DASH 0x02


/***********************************************************************
* Table used to translate from ascii to the LED segment code.
*
*       g
*    +------+
*    |      |
*  b |      | f
*    |  a   |        +-----+-----+-----+-----+-----+-----+-----+------+
*    +------+        |  g  |  f  |  e  |  d  |  c  |  b  |  a  |  dp  |
*    |      |        +-----+-----+-----+-----+-----+-----+-----+------+
*  c |      | e         80    40    20    10    08    04    02    01
*    |      |
*    +------+
*       d
*
************************************************************************/
const BYTE seg_xlate[96]={
 0x00,0x0C,0x44,0x00, 0x00,0x00,0x00,0x04,   /*   !"#  $%&'  */
 0x00,0x00,0x00,0x62, 0x08,0x02,0x00,0x00,   /*  ()*+  ,-./  */
 0xFC,0x60,0xDA,0xF2, 0x66,0xB6,0xBE,0xE0,   /*  0123  4567  */
 0xFE,0xE6,0x12,0x12, 0x00,0x12,0x00,0xCA,   /*  89:;  <=>?  */
 0x00,0xEE,0x3E,0x9C, 0x7A,0x9E,0x8E,0xBC,   /*  @ABC  DEFG  */
 0x6E,0x0C,0x78,0x00, 0x1C,0xCC,0xEC,0xFC,   /*  HIJK  LMNO  */
 0xCE,0x00,0x8C,0xB6, 0x1E,0x7C,0x7C,0x3C,   /*  PQRS  TUVW  */
 0x00,0x66,0x00,0x00, 0x00,0x00,0x80,0x10,   /*  XYZ[  \]^_  */
 0xC6,0xFA,0x3E,0x1A, 0x7A,0xDE,0x8E,0xF6,   /*  `abc  defg  */
 0x2E,0x08,0x78,0x00, 0x0C,0xE4,0x2A,0x3A,   /*  hijk  lmno  */
 0xCE,0x00,0x0A,0xB6, 0x1E,0x38,0x38,0x78,   /*  pqrs  tuvw  */
 0x00,0x66,0x00,0x00, 0x0C,0x00,0x80,0x00    /*  xyz{  |}~   */
};
const BYTE seghexcode[16]={
  0xFC,0x60,0xDA,0xF2,0x66,0xB6,0xBE,0xE0,0xFE,0xE6,0xEE,0x3E,0x9C,0x7A,0x9E,0x8E};



/********************
* Display structure *
*********************/
DISP gDisp;


/*************
* Prototypes *
**************/
void disp_printf(char *fmt, char *args);



/*********************************
*  display hexidecimal value
**********************************/
void disp_led(BYTE led, BYTE state)
  {
  if (state)
    gDisp.led |= led;     /* turn on led  */
  else
    gDisp.led &= ~led;    /* turn off led */
  }




/*********************************
*  display hexidecimal value
**********************************/
void disp_hex(WORD num)
	{
  WORD  i, digit;

	for (i=0; i<4; i++)
		{
		digit = 0x0F & (num >> ((3-i)*4));
    gDisp.digit[i] = seghexcode[digit];
		}
	}



/************************************************
 * display bcd with decimal point (dp) control.
 * dp:
 *   5 = No decimal point, no leading zeroes.
 *   4 = No decimal point, leading zeroes.
 *   0-3 = decimal place.
 ************************************************/
void disp_bcd(WORD bcd)
	{
  WORD  i, digit;
  BYTE  dp;
  BYTE  zero;

  dp = gDisp.dp;
	zero = FALSE;
  if (dp==4 || gZeroFill)
		zero = TRUE;
	for (i=0; i<4; i++)
		{
		if (i==3)
			zero = TRUE;
		digit = 0x0F & (bcd >> ((3-i)*4));
		if (digit || zero)
			{
			zero = TRUE;
      gDisp.digit[i] = digit<=9 ? seghexcode[digit] : DASH;
			}
		else
      gDisp.digit[i] = 0;
    if (gDisp.dp==3-i && bcd!=0xFFFF)
			{
			zero = TRUE;
      gDisp.digit[i] |= DP;
			}
		}
	}


/*************************************************
 *  Display a number with decimal point control. *
 *************************************************/
void disp_price(WORD num)
	{
	int  i;
  WORD dig, bcd;


  /*************************
  * Out of range = "----"
  **************************/
  /*if (num>9999)
		{
		for (i=0; i<4; i++)
      gDisp.digit[i] = DASH;
		return;
    } */


  /*****************
  * Convert to BCD
  ******************/
		for (i=0, bcd=0; i<=12; i+=4)
			{
			dig = num%10;
			num /= 10;
			bcd |= dig<<i;
			}

    /**********************
    * Place decimal point
    ***********************/
    if (coin.ok)
      gDisp.dp = coin.status.decpnt;
    else if (bill.ok)
      gDisp.dp = bill.status.decpnt;
    else
      gDisp.dp = 0;

    /****************
    * Display price
    *****************/
    disp_bcd(bcd);
	}



/*********************
 *  Display a number
 *********************/
void disp_num(WORD num)
	{
	int  i;
  WORD dig, bcd;

  for (i=0, bcd=0; i<=12; i+=4)
    {
    dig = num%10;
    num /= 10;
    bcd |= dig<<i;
    }
  gDisp.dp = 5;
  disp_bcd(bcd);
	}



/*****************************
 *  Display a binary pattern *
 *****************************/
void disp_bin(WORD bin)
	{
	int  i;
  WORD dig, binmask;

	for (i=0, binmask=0x80; i<4; i++)
		{
    dig = 0x28;
		if (bin & binmask)
      dig |= 0x04;
		binmask = binmask>>1;
		if (bin & binmask)
      dig |= 0x40;
		binmask = binmask>>1;
    gDisp.digit[i] = (BYTE)dig;
		}
	}





/************************
* Task - Display driver
*************************/
void task_display()
	{
  WORD addr;
  BYTE update;
  int  i;
  MCX_MAILBOX mbox_display;
  DEVMSG_DISPLAY devmsg_display;
  char *txt;
  const static BYTE chase[12][2]={
    0,0x80, 1,0x80, 2,0x80, 3,0x80, 3,0x40, 3,0x20,
    3,0x10, 2,0x10, 1,0x10, 0,0x10, 0,0x08, 0,0x04};

  mbox_display.message = &devmsg_display;
  devmsg_display.devtype = DEV_DISPLAY;
  devmsg_display.led = 0xFF;    /* force display update */
  while ( FOREVER )
    {
    /******************
    * Do chase circle *
    *******************/
    for (i=0; gChase==CHASE_CIRCLE; i=++i%12)
      {
      devmsg_display.digit[0] = 0;
      devmsg_display.digit[1] = 0;
      devmsg_display.digit[2] = 0;
      devmsg_display.digit[3] = 0;
      devmsg_display.digit[ chase[i][0] ] = chase[i][1];
      mcx_mailbox_send(TASK_dev,0,&mbox_display);
      mcx_task_delay(MYSELF,0,TM_80ms);
      }

    /***************
    * Do DEX chase *
    ****************/
    while (gChase==CHASE_DEX)
      {
      for (txt="    Loading "; *txt && gChase==CHASE_DEX; txt++)
        {
        for (i=0; i<4 && txt[i]; i++)
          devmsg_display.digit[i] = seg_xlate[txt[i]-' '];
        for (; i<4; i++)
          devmsg_display.digit[i] = 0;
        devmsg_display.led = gDisp.led;
        mcx_mailbox_send(TASK_dev,0,&mbox_display);
        mcx_task_delay(MYSELF,0,TM_200ms);
        }
      }


    /*********************************************
    * Check for any changes in what is displayed
    **********************************************/
    update = FALSE;
    if (devmsg_display.led != gDisp.led)
        update = TRUE;
    else for (i=0; i<4; i++)
      if (devmsg_display.digit[i] != gDisp.digit[i])
        update = TRUE;


    /***********************************
    *     Display needs updating
    ************************************/
    //if (update)
    mcx_task_delay(MYSELF,0,TM_50ms);
    if (1)
      {
      for (i=0; i<4; i++)
        devmsg_display.digit[i] = gDisp.digit[i];
      devmsg_display.led = gDisp.led;
      mcx_mailbox_send(TASK_dev,0,&mbox_display);
      }

    /*****************************************************
    * pause for 100 ms or if signal on display semaphore
    ******************************************************/
    mcx_sema_pend(SEMA_display);                   /* make sure wait happens                */
    mcx_timer_start(MYSELF,SEMA_display,TM_100ms); /* ascociate timer with display change   */
    mcx_sema_wait(SEMA_display);                   /* wait for display change or timer      */
    mcx_timer_purge(MYSELF,SEMA_display,0);        /* purge timer                           */
    }
  }


/*************************************************
* Use like a printf() function to send a message
* to the display.
**************************************************/
void disp_printf(char *fmt, char *args)
  {
  char msg[8];
  char i, *p;

  msg[0] = fmt[0];
  msg[1] = fmt[1];
  p = msg;
  *p = '\0';
  _print(&p, fmt, &args);                   /* format message           */

  for ( p=msg, i=0; *p && i<4; p++ )
    {
    if (*p=='.')                            /* check for decimal point   */
      {
      if (i)
        gDisp.digit[i-1] |= DP;             /* put in decimal point      */
      }
    else if (*p>=' ' && *p<='~')            /* check for char range      */
      gDisp.digit[i++] = seg_xlate[*p-' ']; /* translate to segment code */
    }
  if (i && *p=='.')
    gDisp.digit[i-1] |= DP;                 /* put in trailing decimal point */

  for (; i<4; i++)
    gDisp.digit[i] = 0;                     /* blank unused segments     */

  mcx_sema_signal(SEMA_display);
  }
