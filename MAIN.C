/****************
*  MAIN MODULE  *
*****************/
#define _MAIN_
#include "hdrs.h"

#define TESTMODE 0                 /* 1=testmode, 0=normal */

/*************
* Prototypes *
**************/
void vendproduct(WORD v, BYTE side);

/*****************************************
*     Platform specific and SCI setup
*****************************************/
void InitMCU()
{
  const static BYTE configval=0x09;

  PORTA = 0x80;                   /* Disable motors, clear addr bits */
  PACTL = 0x88;
  PORTD = 0x00;
  DDRD =  0x38;                   /* configure SS, SCK, & MOSI for output */
  SPCR =  0x52;                   /* SPI enable, Master mode set */
  OPTION |= ADPU;                 /* Power up A/D */
  ADCTL = 0;                      /* single scan, single channel */
  if (CONFIG!=configval)
    eepcpy(&CONFIG,&configval,1); /* Write NOSEC=1, NOCOP=0, ROMON=0, EEON=1 */
  gMaster = !(PORTE&PORTE_SLAVE);
  if (gMaster)
  {
    TCTL2 = EDG1A|EDG2A|EDG3A;    /* Rising edge input captures     */
    TMSK1 = IC1I;                 /* Interrupt enable gallon count  */
  }
  else
    TCTL2 = EDG2A|EDG3A;          /* Rising edge input captures     */
}


/***************************
* Determine maximum price
****************************/
void getmaxminprice()
{
}

/********************
* Flash soldout LED *
*********************/
void flash_soldout()
{
  BYTE j;

  keypadchar = 0;
  for (j=0; j<4 && !keypadchar; j++)
  {
    disp_led(LED_SOLD,1);
    mcx_task_delay(MYSELF,0,TM_500ms);
    disp_led(LED_SOLD,0);
    mcx_task_delay(MYSELF,0,TM_500ms);
  }
}


/*************************
* Flash exact change LED *
**************************/
void flash_exact()
{
  BYTE j;

  keypadchar = 0;
  for (j=0; j<4 && !keypadchar; j++)
  {
    disp_led(LED_EXACT,1);
    mcx_task_delay(MYSELF,0,TM_500ms);
    disp_led(LED_EXACT,0);
    mcx_task_delay(MYSELF,0,TM_500ms);
  }
}


/********************************
* Return true if water is dirty *
********************************/
int WaterDirty()
{
  if (gMaster)
    return !gSwitch[SW_TDS_MONITOR] ? TRUE : !gSwitch[SW_UV_DETECT_PRI];
  else
    return !!(PORTA&0x04);              /* Slave finds out from Master  */
}

/***********************
*  M A I N    T A S K
************************/
void task_main( void )
  {
  BYTE key, v, n, k, side;
  BYTE state, status, first_time_boot;
  WORD i, j, pr;
  WORD *setcode;
  long *sn;

  /****************************
  * Determine first time boot *
  *****************************/
  first_time_boot = (BYTE)((eepversion&0xFFF0)!=(progvers&0xFFF0));

  /**************
  * Basic Inits *
  ***************/
  InitMCU();                          /* init MCU platform      */
  mdb_init();                         /* init mdb               */
  sci_init();                         /* init SCI               */
  eeprom_init();                      /* init eeprom if needed  */
  rsrc_SCI = 0;                       /* set SCI rsrc to off    */
  debounce_init(&gDebDex,0,3);        /* initialize dex detect  */
  getmaxminprice();                   /* init max & min prices  */

  /**************
  * Start Tasks *
  ***************/
  mcx_task_execute(TASK_dev);         /* device driver task         */
  sci_mdb_mode();                     /* default SCI to MDB mode    */
  SCCR2 |= 1;                         /* Turn on break              */
  mcx_task_delay(MYSELF,0,TM_100ms);  /* Wait 100 ms                */
  SCCR2 ^= 1;                         /* Turn off break             */
  mcx_task_delay(MYSELF,0,TM_100ms);  /* Wait 100 ms                */
  mcx_task_execute(TASK_display);     /* display task               */
  mcx_task_execute(TASK_keypad);      /* keypad task                */
  mcx_task_execute(TASK_mdb);         /* low level MDB task         */
  mcx_task_execute(TASK_initmdb);     /* MDB setup task             */
  mcx_task_execute(TASK_pollmdb);     /* MDB polling task           */
  mcx_task_execute(TASK_periodic);    /* misc periodic routine task */
  mcx_task_execute(TASK_vend1);       /* Vend task #1               */
  mcx_task_execute(TASK_vend2);       /* Vend task #2               */
  if (gMaster)
    mcx_task_execute(TASK_maintenance); /* Water quality maintnance */
  //mcx_task_execute(TASK_dbug);        /* Debug task                 */


  /*********************************
  * Determine serial number status *
  **********************************/
  setcode = (WORD *)SN_CODE;
  sn      = (long *)SN_DATA;
  if ((*setcode != SN_SET) && (*setcode != SN_UNSET))
  {
    *sn      = 0x10000;
    *setcode = SN_UNSET;
  }
  gSNset = (BYTE)(*setcode == SN_SET);


  /*************************
  * Display version number *
  **************************/
  if (new_version)
  {
    disp_printf("Init");
    mcx_task_delay(MYSELF,0,TM_1sec);
  }

  disp_printf("%X.%02X", progvers>>8, progvers&0xFF);
  mcx_task_delay(MYSELF,0,TM_1sec);

  /**********************
  *  M A I N   L O O P  *
  ***********************/
  disp_led(LED_SOLD,0);
  disp_led(LED_EXACT,0);
  gCredit = 0;

  while (FOREVER)
  {
    /**************************************************
    * Sets Master & Slave Sold-Outs when particulates *
    * are high, or when the UV lamps are inoperative, *
    * or when both vend windows are in a fail mode.   *
    **************************************************/
    if ((!gVendTaskActive[0] && !gVendTaskActive[1]))
      gAllSoldOut = WaterDirty();
    if (gMaster)
    {
      if (!gSwitch[SW_TDS_MONITOR] || !gSwitch[SW_UV_DETECT_SEC])
        PORTD &= ~0x20;                 /* Signal Slave Sold-Out  */
      else
        PORTD |= 0x20;                  /* Signal Slave OK        */
    }
    if (gVendFailTimer[0] && gVendFailTimer[1])
      gAllSoldOut = TRUE;               /* Both vend windows are failed */

    disp_led(LED_SOLD,gAllSoldOut);     /* Light up sold-out LED        */

    /*******************************
    * Determine Exact change state
    ********************************/
    if (coin.ok && gCalcExact)
    {
      disp_led(LED_EXACT,coin_exact());
      gCalcExact = FALSE;
    }

    /**********************************
    * Enable or disable coins & bills *
    **********************************/
    if (gAllSoldOut)   // if all sold out or coupon vending --> disable
    {
      coin_disable();
      bill_disable();
    }
    else
    {
      coin_enable();
      bill_enable();
    }

    /**************************
    * Display price or dashes *
    ***************************/
    if (coin.ok || bill.ok)          // if coin or bills
      disp_price(gCredit);
    else                             // if no MDB
      disp_printf("----");

    /***************
    * Get keypress *
    ****************/
    if (keypadchar)
      key = keypadchar;
    else
      key = key_timeout(TM_200ms);
    keypadchar = 0;


    /**********************************************
    * Execute code below on a vend key depression *
    ***********************************************/
    if ((key > 0) && ((key <= SELMAX) || key == KEY_3_A || key == KEY_3_B))
    {
      // choose the # of gallons
      if (key == KEY_1_A || key == KEY_1_B)
        i = 0;
      else if (key == KEY_3_A || key == KEY_3_B)
        i = 1;
      else if (key == KEY_5_A || key == KEY_5_B)
        i = 2;
      // choose the side A or B
      if (key == KEY_1_A || key == KEY_3_A || key == KEY_5_A)
        side = 0;
      else
        side = 1;

      /**************************************************
      *         Not enough funds to purchase            *
      **************************************************/
      pr = gPrice[i];
      if (gCredit < pr)
      {
        if (!bill.tokenused)
        {
          disp_price(gPrice[i]);
          if (!keypadchar)
            key_timeout(TM_1sec);
          if (!keypadchar)
            key_timeout(TM_200ms);
        }
      }

      /***********************************************
      * Return change rather than cheat the customer *
      ************************************************/
      else if (gCredit>pr && gOption[OP_NOCHEAT] && !bill.tokenused &&
        !coin_change_exact(gCredit-pr,gCoinCount))
      {
        if (gCoinToken)
        {
          //mis.nr.cash -= min(mis.nr.cash, gCoinToken*20);
          //mis.rs.cash -= min(mis.rs.cash, gCoinToken*20);
          //mis.download.cash -= min(mis.download.cash, gCoinToken*20);
        }
        coin_release(gCredit);
        gCoinToken = gCredit = 0;
        flash_exact();
      }

      /*****************
      * Vend a product *
      ******************/
      else if ((!side) && (!gVendTaskActive[0]))
      {
        //mcx_task_delay(MYSELF,0,TM_1sec);  // do not take out - needed for coupon vending (lose token counts)
        vendproduct(i, side);
        disp_price(gCredit);
      }
      else if ((side) && (!gVendTaskActive[1]))
      {
        //mcx_task_delay(MYSELF,0,TM_1sec);  // do not take out - needed for coupon vending (lose token counts)
        vendproduct(i, side);
        disp_price(gCredit);
      }
    }

    /********************************************
    * Release coins if release lever is pressed *
    * or all sold out.                          *
    *********************************************/
    if (gCredit)
    {
      if (gAllSoldOut && !gVendTaskActive[0] && !gVendTaskActive[1])
      {
        coin_enable();
        bill_enable();
        gCredit -= min(gCredit,bill_release());
        if (gCoinToken)
        {
          //mis.nr.cash -= min(mis.nr.cash, gCoinToken*20);
          //mis.rs.cash -= min(mis.rs.cash, gCoinToken*20);
          //mis.download.cash -= min(mis.download.cash, gCoinToken*20);
        }
        coin_release(gCredit);
        gCoinToken = gCredit = 0;
      }
      else if (coin.release && !bill.tokenused)
      {
        gCredit -= min(gCredit,bill_release());
        if (gCoinToken)
        {
          //mis.nr.cash -= min(mis.nr.cash, gCoinToken*20);
          //mis.rs.cash -= min(mis.rs.cash, gCoinToken*20);
          //mis.download.cash -= min(mis.download.cash, gCoinToken*20);

          // Someone trying to cheat system - clear credit without returning
          gCoinToken = gCredit = 0;
        }
        else
        {
          coin_release(gCredit);
          gCoinToken = gCredit = 0;
        }
      }
    }
    else
      coin.release = FALSE;

    /*********************************
    * Check for service mode request *
    **********************************/
    if (key==KEY_MODE)
    {
    
      if (*setcode == SN_SET)
      {
        // reset password validation EVERY time into service mode
        gPasswordValidated = FALSE;
        GetPassword();
      }

      gServiceMode = TRUE;
      coin_disable();
      bill_disable();
      gVendFailTimer[0] = gVendFailTimer[1] = 0;
      if (gSNset)
        fn_menu(gMaster ? menu_master : menu_slave);
      else
        SetSN();
      getmaxminprice();
      keypadchar = 0;
      gServiceMode = FALSE;
      gCalcExact = TRUE;
    }

    if (gReturnChange && (gReturnTime > 5) && !gVendTaskActive[0] && !gVendTaskActive[1])
    {
      if (gCoinToken)
      {
        //mis.nr.cash -= min(mis.nr.cash, gCoinToken*20);
        //mis.rs.cash -= min(mis.rs.cash, gCoinToken*20);
        //mis.download.cash -= min(mis.download.cash, gCoinToken*20);

        // Someone trying to cheat system - clear credit without returning
        gCoinToken = gCredit = 0;
      }
      else
      {
        coin_release(gCredit);            // Version 1.24 - always return the change
        gCoinToken = gCredit = 0;
        gReturnChange = FALSE;
      }
    }

    /**********************************
    * If dex is detected, do dex xfer *
    ***********************************/
    if (debounce(&gDebDex,!(PORTE&DEXSW)))
    {
      dex_xfer();

      dex_disconnect();

      mcx_task_delay(MYSELF,0,TM_1sec);
      sci_init();                    /* init SCI                                */
      rsrc_SCI = 0;                  /* set SCI rsrc to off                     */
      debounce_init(&gDebDex,0,3);   /* initialize dex detect                   */

      while ( debounce( &gDebDex, !(PORTE&DEXSW) ) )
        mcx_task_delay(MYSELF,0,TM_50ms);
    }
  }
}

