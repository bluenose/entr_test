/**********************
* keypad.c --         *
*  Driver for keypad  *
***********************/
#include "hdrs.h"


/*****************************************
* Wait for keypress with timeout.
* Return keypad code or zero if timeout.
******************************************/
BYTE key_timeout(MCX_TICKS ticks)
  {
  keypadchar = 0;                               /* default keypadchar to zero     */
  mcx_sema_pend(SEMA_keypad);                   /* make sure keypress happens     */
  mcx_timer_start(MYSELF,SEMA_keypad,ticks);    /* ascociate timer with keypress  */
  mcx_sema_wait(SEMA_keypad);                   /* wait for keypress or timer     */
  mcx_timer_purge(MYSELF,SEMA_keypad,0);        /* purge timer                    */
  return keypadchar;                            /* return keypress or zero        */
  }


/*****************************************
* Wait for keypress with timeout.
* Return keypad code or zero if timeout.
* Beep if timeout.
******************************************/
BYTE key_timeout_beep(MCX_TICKS ticks)
  {
  keypadchar = 0;                               /* default keypadchar to zero     */
  mcx_sema_pend(SEMA_keypad);                   /* make sure keypress happens     */
  mcx_timer_start(MYSELF,SEMA_keypad,ticks);    /* ascociate timer with keypress  */
  mcx_sema_wait(SEMA_keypad);                   /* wait for keypress or timer     */
  mcx_timer_purge(MYSELF,SEMA_keypad,0);        /* purge timer                    */
  if (!keypadchar)                              /* check for timeout              */
    beep = 1;                                   /* beep if timeout                */
  return keypadchar;                            /* return keypress or zero        */
  }


/**********
* Globals *
***********/
long gKeyPress;
long gKeyStuck;
const char gKeyMatrix[8]={1,2,3,4,5,6,7,8};


/****************
* Read keypad
*****************/
BYTE key_get()
  {
  BYTE i, r, c, style;
  MCX_MAILBOX mbox_keypad;
  DEVMSG_KEYPAD devmsg_keypad;
  static DEBOUNCE debsw[10];
  static DEBOUNCE debdoor;
  static DEBOUNCE2 debkey;
  static BYTE ch=0;
  static BYTE init=0;
  long newkey;
  BYTE switches, sw_key;

  /********************************************
  * Do a one time init of door debounce value
  *********************************************/
  if (!init)
    {
    init = TRUE;
    debounce_init(&debdoor,0,2);
    debounce2_init(&debkey,0,2);
    for (i=0; i<10; i++)
      debounce_init(&debsw[i],0,2);
    }


  /************************************
  * Update keypad press matrix
  *************************************/
  devmsg_keypad.devtype = DEV_KEYPAD;
  mbox_keypad.message = &devmsg_keypad;
  mcx_mailbox_send(TASK_dev,0,&mbox_keypad);
  gKeyPress = debounce2(&debkey,(~devmsg_keypad.data)&0x3F);
  sw_key = (BYTE)(~(devmsg_keypad.data>>8));
  gKeyPress = gKeyPress | (long)(sw_key & 0xC0);

  /******************
  * Update switches *
  *******************/
  switches = (BYTE)(~(devmsg_keypad.data>>6));
  for (i=0; i<SW_MAX; i++, switches>>=1)
    gSwitch[i] = debounce(&debsw[i],switches&1);    /* Debounce switches */

  /***************************************
  * Determine keypress & keystuck status
  ****************************************/
  if (gKeyPress!=gKeyStuck)
    {
    newkey = gKeyPress & ~gKeyStuck;
    for ( ch=0, r=0; r<KEY_NUM; r++ )
      {
      if (newkey & (1L<<r))
        {
        ch = gKeyMatrix[r];
        }
      }
    gKeyStuck = gKeyPress;
    }


  /****************************
  * Zero timeout on key press *
  *****************************/
  if (ch)
    gServiceTimeout = 0;


  /*******************
  * Control RO plant *
  ********************/
  if (ch==KEY_RO_START || ch==KEY_RO_STOP)
    {
    gROstate = (ch==KEY_RO_START);
    ch = 0;
    }


  return ch;
  }



/***************************
*     keypad task
****************************/
void task_keypad( void )
  {
  BYTE oldch,newch;
  WORD rpt;
  DEBOUNCE deb;

  debounce_init(&deb,0,2);

  while (FOREVER)
    {
    mcx_task_delay(MYSELF,0,TM_12ms);
    newch = debounce(&deb,key_get());
    if (TRUE || gServiceMode || newch<=SELMAX)
      {
      if ( newch && oldch!=newch )
        {
        rpt = 0;
        beep = 1;
        keypadchar = newch;
        mcx_sema_signal(SEMA_keypad);
        }
      else if ( gServiceMode && newch && oldch==newch )
        {
        rpt++;
        if ( rpt>300 || (rpt>60 && !(rpt%10)) )      /* Repeat key speed */
          {
          keypadchar = newch;
          mcx_sema_signal(SEMA_keypad);
          }
        }
      oldch = newch;
      }
    }
  }
