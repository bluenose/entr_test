/********
* spi.c *
*********/
#include "hdrs.h"

#define FiveMinutes 300


/****************
* Get A/D value *
*****************/
BYTE getad(BYTE addr)
  {
  ADCTL = addr;
  while (!(ADCTL & 0x80))
    ;
  return ADR4;
  }



/**************************************
* Miscellaneous periodic routines
***************************************/
void task_periodic()
{
  const static WORD ledgal[2][3]={G1_LED1,G3_LED1,G5_LED1,G1_LED2,G3_LED2,G5_LED2};
  BYTE i, lastmode=0, newmode=0;
  WORD second=0, lastcredit=-1, crSecs=0, blinktimer=0;
  BYTE blink;
  DEBOUNCE debmode;
  MCX_MAILBOX mbox_outp;
  DEVMSG_OUTP outp;

  outp.devtype = DEV_OUTP;
  mbox_outp.message = &outp;
  debounce_init(&debmode, 0, 3);      /* init debounce structure  */

  mcx_timer_start(MYSELF,SEMA_periodic,-TM_PERIODIC);
  while ( FOREVER )
    {
    mcx_sema_wait(SEMA_periodic);

    /*************************************
    * Accumilate gallon count from slave *
    **************************************/
    if (gMaster && gSlaveGalCount)
      {
      sei();
      gGallonCount  +=  gSlaveGalCount;   /* Add to Total gallon count */
      gROgalcnt     +=  gSlaveGalCount;   /* Add to RO gallon count    */
      gSlaveGalCount = 0;
      cli();
      }

    /*************************
    * read mode switch
    **************************/
    newmode = debounce(&debmode,!(PORTE&MODESW));
    /* newmode -- True if pressed, FALSE otherwise */
    if (lastmode!=newmode )
      {
      if (newmode)
        {
        gServiceTimeout = 0;
        keypadchar = KEY_MODE;
        mcx_sema_signal(SEMA_keypad);
        }
      lastmode = newmode;
      }


    /****************************************
    * Blink or turn on water pouring lights *
    *****************************************/
    if ((blinktimer += TM_PERIODIC) > TM_800ms)
      blinktimer = 0;
    blink = blinktimer<TM_400ms;
    outp.ctrl = MASK;
    outp.mask = G1_LED1 | G3_LED1 | G5_LED1 | G1_LED2 | G3_LED2| G5_LED2;
    outp.select = 0;
    if (!gServiceMode)
    {
      for (i=0; i<2; i++)
      {
        // both tasks active
        if ((gVendTaskActive[0]) && (gVendTaskActive[1]))
          outp.select = ledgal[0][gGalSelect[0]] | ledgal[1][gGalSelect[1]];         // turn on both sides
        // if one task is active
        else if (gVendTaskActive[i])
        {
          outp.select = ledgal[i][gGalSelect[i]];                                    // turn on that side
          if (gCredit >= gPrice[GAL1] && (!gVendFailTimer[!i]))                      // blink other if not SO
            outp.select |= (blink) ? ledgal[!i][GAL1] : 0;
          if (gCredit >= gPrice[GAL3] && (!gVendFailTimer[!i]))                      // blink other if not SO
            outp.select |= (blink) ? ledgal[!i][GAL3] : 0;
          if (gCredit >= gPrice[GAL5] && (!gVendFailTimer[!i]))                      // blink other if not SO
            outp.select |= (blink) ? ledgal[!i][GAL5] : 0;
        }
        // if both tasks aren't active
        else if (!gVendTaskActive[0] && !gVendTaskActive[1] && !gVendFailTimer[i])   // not SO
        {
          if (gCredit >= gPrice[GAL1])   // blink 1 gallons
            outp.select |= (blink) ? ledgal[i][GAL1] : 0;
          if (gCredit >= gPrice[GAL3])   // blink 3 gallons  Version 1.24
            outp.select |= (blink) ? ledgal[i][GAL3] : 0;
          if (gCredit >= gPrice[GAL5])   // blink 5 gallons
            outp.select |= (blink) ? ledgal[i][GAL5] : 0;
        }
      }
    }
    mcx_mailbox_send(TASK_dev,0,&mbox_outp);
    /********************
    * Do countup timers *
    *********************/
    gServiceTimeout += TM_PERIODIC;
    gVendTimer[0] += TM_PERIODIC;
    gVendTimer[1] += TM_PERIODIC;
    second += TM_PERIODIC;

    if (!gCredit)
      gFirstTry = FALSE;

    /*******************
    * Do second timers *
    ********************/
    if (second>=TM_1sec)
      {
      second = 0;

      /**************************
      * Update countup counters *
      ***************************/
      if (gNoPulseTimer[0] < 5)
        gNoPulseTimer[0]++;
      if (gNoPulseTimer[1] < 5)
        gNoPulseTimer[1]++;
      gROpostTimer++;
      if (gROtimerOff>=(4*60*60))       /* Four hours */
      {
        gROstate = ON;
        gROTimer = 0;
        gROtimerOff = 0;
      }
      else if (gROstate==ON)
      {
        gROtimerOff = 0;
        gROTimer++;
      }
      else
        gROtimerOff++;

      gPourTimer[0]++;
      gPourTimer[1]++;

      if (gVendTaskActive[0] || gVendTaskActive[1])
        gReturnTime = 0;
      else
        gReturnTime++;
      /**************************
      * Update countdown timers *
      ***************************/
      if (gVendFailTimer[0])
        --gVendFailTimer[0];
      if (gVendFailTimer[1])
        --gVendFailTimer[1];

      /************************************
      * Zero untouched credit over 5 mins *
      *************************************/
      crSecs++;
      if (gCredit != lastcredit)
        crSecs = 0;
      else if (crSecs > FiveMinutes)
        crSecs = gCredit = coin.tokenused = FALSE;
      lastcredit = gCredit;


      }
    }
  }



/********************************
* Send SPI data
*********************************/
BYTE spi_sndrcv(BYTE by)
  {
  /*rsrc_lock(&rsrc_AD);*/
  SPDR = by;
  while ( !(SPSR & 0x80) )
    ;
  /*rsrc_unlock(&rsrc_AD);*/
  return (BYTE)SPDR;
  }




/**********************************
* Receive messages for devices
***********************************/
void task_dev( void )
  {
  UNIONWB gpod;
  BYTE i, cols, cntr;
  int delay,temp_data;
  MCX_MAILBOX *mbox;
  DEVMSG_GENERIC *msg;
  DEVMSG_DISPLAY *disp;
  DEVMSG_OUTP    *outp;
  DEVMSG_KEYPAD  *keypad;

  gpod.w = 0;

  /**********************************************
  * Loop here forever waiting for mbox messages
  ***********************************************/
	while (FOREVER)
		{
    mcx_mailbox_receive(0,0,&mbox);
    msg = (DEVMSG_GENERIC *)mbox->message;

    switch ( msg->devtype )
      {

      /**********************
      *   Display
      ***********************/
      case DEV_DISPLAY:
        disp = (DEVMSG_DISPLAY *)mbox->message; /* open letter                  */
        PORTA |= ADDR_DISPLAY;                  /* address display              */
        spi_sndrcv(0x01);                       /* 7 zero bits & one start bit  */
        spi_sndrcv(0x00);                       /* 8 zero bits */
        spi_sndrcv(0x00);                       /* 8 zero bits */
        spi_sndrcv(0x00);                       /* 8 zero bits */
        spi_sndrcv(0x00);                       /* 8 zero bits */
        spi_sndrcv(0x01);                       /* 7 zero bits & one start bit  */
        for (i=0; i<4; i++)                     /* 4 data bytes = 32 data bits  */
          spi_sndrcv(disp->digit[i]);           /* send it */
        spi_sndrcv(disp->led);                  /* 2 bits and 6 zero bits       */
        PORTA ^= ADDR_DISPLAY;                  /* un-address display           */
        break;


      /**********************
      *   Keypad
      ***********************/
      case DEV_KEYPAD:
        keypad = (DEVMSG_KEYPAD *)mbox->message;
        PORTA |= ADDR_ICLK;
        PORTA ^= ADDR_ICLK;
        PORTA |= ADDR_LOAD;
        PORTA ^= ADDR_LOAD;
        PORTA |= ADDR_KEYPAD;
        keypad->data = 0;
        keypad->data |= spi_sndrcv(0x00);           /* get LOW keypad byte      */
        keypad->data |= (WORD)spi_sndrcv(0x00)<<8;  /* get HIGH keypad byte     */
        PORTA ^= ADDR_KEYPAD;
        break;


      /**********************
      *   DEX/MDB select
      ***********************/
      case DEV_SCI_SEL:
        break;


      /**********************
      *   Output devices
      ***********************/
      case DEV_OUTP:
        outp = (DEVMSG_OUTP *)mbox->message;
        cntr = outp->ctrl;

        if (cntr==MASK)
          gpod.w = (gpod.w & ~outp->mask) | outp->select;

        else if (cntr==ON)
          gpod.w |= outp->select;

        else
          gpod.w &= ~outp->select;

        sei();
        PORTA |= OUTP_ENB;
        PORTA |= ADDR_OUTP;
        spi_sndrcv(gpod.b[0]);
        spi_sndrcv(gpod.b[1]);
        PORTA ^= ADDR_OUTP;
        PORTA &= ~OUTP_ENB;
        cli();
        break;

      }
		}
  }
