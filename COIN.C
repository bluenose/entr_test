/*********************************************
* Name: coin.c
*
* Description:
*  Multi-Drop Bus (MDB) Routines
*
**********************************************/
#include "hdrs.h"



/********************************
* initialize mdb coin structure
*********************************/
void coin_init()
{
  coin.ok = coin.enabled = coin.release = FALSE;
  coin.tokenused = FALSE;
}


/*********************
*  init MDB Coin Task
**********************/
void coin_poll_init()
{
  BYTE i, j;
  WORD total;

  if ( !coin.ok )
  {
    coin.enabled = FALSE;
    coin.tube.full = 0;
    for ( i=0; i<16; i++ )
      coin.tube.report[i] = 0;
    coin.expansion = 0;
    coin.reset = TRUE;
    if (mdb_sendmsg(0,MDB_COIN_RESET,       -1,0,0, 0,0,0))
      if (mdb_sendmsg(2,MDB_COIN_STATUS,      -1,0,0, 0,23,&coin.status))
      {
        if (coin.status.feature<3)
        {
          coin.ok = TRUE;
        }
        else
        {
          if (mdb_sendmsg(2,MDB_COIN_EXPANSION, 0x00,0,0, 29,4,&coin.expansion))
          {
            coin.expansion &= 1; /* set expansion */
            if (coin.expansion==0 || mdb_sendmsg(2,MDB_COIN_EXPANSION, 0x01,4,&coin.expansion, 0,0,0))
                coin.ok = TRUE;
          }
        }
      }
    if (coin.ok)  /* success on init - set up coin-to-tube map */
    {
      for (i=0, coin.tube.num=0, coin.tube.token=0; i<16; i++)
      {
        if (coin.status.credit[i])
          coin.tube.map[ coin.tube.num++ ] = i;
        if (coin.status.credit[i]==0xFF)
          coin.tube.token |= 1<<i;
      }
      gCalcExact = TRUE;
    }
  }
  if (coin.ok)
  {
    if (mdb_sendmsg(2,MDB_COIN_TUBESTATUS,-1,0,0, 0,18,&coin.tube))
    {
      for (j=0, total=0; j<coin.tube.num; j++)
      {
        i = coin.tube.map[j];
        if (coin.expansion&1 || coin.tube.full&(1<<i))
          coin.tube.count[i] = coin.tube.report[i];
        else if (coin.tube.report[i]>coin.tube.count[i])
          coin.tube.count[i] = coin.tube.report[i];
        if (coin.status.credit[i]!=0xFF)
          total += coin.tube.count[i] * coin.status.credit[i];
      }
      if ( coin.tube.total != total )
      {
        gCalcExact = TRUE;
        bill.newcointotal = TRUE;
      }
      coin.tube.total = total;
    }
  }
}





/*******************************************************
* Manual Coin dispense -  b1 = 1yyyxxxx, b2 = zzzzzzzz
*
*   yyy = Number of coins.
*
*   xxxx = Coin type.
*
*   zzzzzzzz =  Number of coins in tube.
*
********************************************************/
void coin_dispense(BYTE b1, BYTE b2)
{
  BYTE x,y,count;
  WORD cash;

  y = (b1&0x70)>>4;
  x = (b1&0x0F);
  count = min(coin.tube.count[x],y);
  coin.tube.count[x] -= count;
  coin.tube.total -= (WORD)count*coin.status.credit[x];
  cash = (WORD)y*coin.status.scaling*coin.status.credit[x];
  mis.nr.cash_paid_manual += cash;
  mis.rs.cash_paid_manual += cash;
  mis.download.cash_paid_manual += cash;
  mis.nr.cash_paid_change += cash;
  mis.rs.cash_paid_change += cash;
  mis.download.cash_paid_change += cash;
  gCalcExact = TRUE;
}



/**************************************************
* Coin deposited -  b1 = 01yyxxxx, b2 = zzzzzzzz
*
*   yy = Coin routing.
*     00 Cash box
*     01 Tubes
*     10 Unused
*     11 Reject
*
*   xxxx = Coin type.
*
*   zzzzzzzz =  Number of coins in the tube for
*               the coin type accepted.
*
**************************************************/
void coin_deposit(BYTE b1, BYTE b2)
{
  BYTE x,y,i;
  WORD cr;

  y = 0x03&(b1>>4);
  x = 0x0F&b1;

  if (y<2)
  {
    if (coin.status.credit[x] == 0xFF)            // a token is given a credit value of 20 Cents
    {
      gCoinToken++;
      cr = gPrice[TOKEN];
      //coin.tube.count[1]+=2;                      // fake a token as 2 dimes
      //coin.tube.total += coin.status.credit[1];   // add to total
      //coin.tube.total += coin.status.credit[1];   // add to total
    }
    else                                          // reg. coin
      cr = (WORD)coin.status.credit[x]*coin.status.scaling;

    if (y==1)
    {
      coin.tube.count[x]++;                     /* routed to tube */
      coin.tube.total += coin.status.credit[x];
      mis.nr.cash2tubes += cr;
      mis.rs.cash2tubes += cr;
      mis.download.cash2tubes += cr;
    }
    else if (coin.status.credit[x] != 0xFF)     // add coins to box only if not tokens
    {
      mis.nr.cash2box += cr;                    /* routed to box */
      mis.rs.cash2box += cr;
      mis.download.cash2box += cr;
    }
    if (!gServiceMode)
    {
      sei();
      gCredit += cr;                            /* update credit */
      cli();
      gReturnChange = FALSE;                    // credit added - don't kick out
    }
    gCalcExact = TRUE;
  }
}


void coin_slug(BYTE by)
  {}



/******************
*  poll coin MDB
*******************/
void coin_poll( void )
{
  BYTE *b;
  int i;
  WORD pollstatus;

  if (coin.ok)
  {
    if (!mdb_sendmsg(2,MDB_COIN_POLL,  -1,0,0, 0,0,0))
    {
      coin.ok = FALSE;
      for (i=0; i<16; i++)
        coin.tube.count[i] = 0;   /* re-init tube count if changer was unplugged */
    }
    else
    {
      pollstatus = 0;
      for ( b=sci.buffer, i=sci.rcvcount-1; i>0; b++, i-- )
      {
        if ( *b&0x80 )
        {
          coin_dispense(*b,*(b+1));
          b++;
          i--;
        }
        else if ( *b&0x40 )
        {
          coin_deposit(*b,*(b+1));
          b++;
          i--;
        }
        else if ( *b&0x20 )
          coin_slug(*b);
        else if ( !(*b&0xF0) )
          pollstatus |= 1L<<*b;
      }
      coin.pollstatus = pollstatus;
      if ((coin.pollstatus & COIN_STATUS_RELEASE)                                       &&   // lever pressed
          ((gFirstTry)                                                                  ||   // vend attempted with enough credit established
          ((!gOption[OP_FORCED] || !gBill_accepted) && !gCoinToken && !bill.tokenused)))  // not forced vend  or no bills and no tokens
        coin.release = TRUE;
    }
  }
}


/*********************************
* Enable coin mdb during service
**********************************/
void coin_enable_service()
{
  static long coin_enable=0x0000FFFF;

  if (coin.ok)
  {
    rsrc_lock(&rsrc_SCI);             /* lock SCI resource    */
    if (mdb_sendmsg(2,MDB_COIN_COINTYPE,-1,4,&coin_enable, 0,0,0))
      coin.enabled = TRUE;
    rsrc_unlock(&rsrc_SCI);           /* unlock SCI resource    */
  }
}



/*************************
* Enable coin acceptance *
**************************/
void coin_enable()
{
  static long old_enable=0x0000FFFF;
  long  new_enable;
  int   mask, depcr, availcr;
  BYTE  i, iref, lastdoor;
  static int  lastcredit=-1;
  static BYTE transition=TRUE;


  /********************************
  * Invalidate old enable pattern *
  *********************************/
  if (!coin.ok)
    old_enable = 0x0000FFFF;

  /******************
  * Detect a change *
  *******************/
  if (!coin.ok || gCredit!=lastcredit || !coin.enabled || gDoor!=lastdoor)
    transition = TRUE;

  /**********************************
  * Return when nothing has changed *
  ***********************************/
  if (!coin.ok || !transition)
    return;
  else
  {
    lastdoor   = gDoor;
    lastcredit = gCredit;
    transition = FALSE;
  }

  /****************************************************
  * Determine coins to accept. Accept all coins with  *
  * tubes, and accept all coins with available credit *
  * in tubes. Reject all coins when credit exceeds    *
  * highest product price.                            *
  *****************************************************/
  if (gCredit >= gMaxPrice)
    new_enable = 0;              /* reject all coins when max price is exceeded    */
  else if (!coin.status.routing)
  {
    for (i=0; i<16; i++)
      if (coin.status.credit[i]>0)// && coin.status.credit!=0xFF)
        new_enable |= 1<<i;      /* accept all valid coins when there are no tubes */
  }
  else
  {
    sei();
    depcr = gCredit/coin.status.scaling;          /* deposited credit in coin factors   */
    cli();
    availcr = coin.tube.total - depcr;            /* available credit for making change */
    for (iref=0, new_enable=0; iref<coin.tube.num; iref++)
    {
      i = coin.tube.map[iref];
      mask = (WORD)1<<i;
      if (/*!(coin.tube.token&mask) && */(coin.status.routing&mask || availcr>=coin.status.credit[i]))
        new_enable |= mask;
    }
  }
  //if (gCredit==0)
  if (1)
    new_enable |= coin.tube.token;                /* accept token routing when credit is zero */
  new_enable = new_enable << 16;                  /* shift bits to proper position            */

  /*******************************************
  * Enable manual dispense when door is open *
  ********************************************/
  if (gDoor)
    new_enable |= 0xFFFF;

  /**********************************
  * Send message to MDB coin device *
  ***********************************/
  if ((coin.ok && old_enable!=new_enable) || (coin.ok && !coin.enabled))
  {
    rsrc_lock(&rsrc_SCI);             /* lock SCI resource    */
    if (mdb_sendmsg(2,MDB_COIN_COINTYPE,-1,4,&new_enable, 0,0,0))
      coin.enabled = TRUE;
    rsrc_unlock(&rsrc_SCI);           /* unlock SCI resource    */
    old_enable = new_enable;
  }
}

/**************************
* Disable coin acceptance *
***************************/
void coin_disable()
{
  long coin_disable;

  /*******************************************
  * Enable manual dispense when door is open *
  ********************************************/
  if (gDoor)
    coin_disable = 0xFFFF;
  else
    coin_disable = 0;

  if (coin.ok && coin.enabled)
  {
    rsrc_lock(&rsrc_SCI);             /* lock SCI resource    */
    if (mdb_sendmsg(2,MDB_COIN_COINTYPE,-1,4,&coin_disable, 0,0,0))
      coin.enabled = FALSE;
    rsrc_unlock(&rsrc_SCI);           /* unlock SCI resource    */
  }
}


/******************************************************
* Determine if exact change exists for credit amount, *
* and calculate change to return.                     *
*******************************************************/
BYTE coin_change_exact(WORD credit, BYTE *coincount)
{
  char i, j, jref, iref;
  WORD cr;

  sei();
  credit /= coin.status.scaling;              /* scale to coin factors                */
  cli();
  if (credit==0 || coin.tube.num==0)
    return FALSE;                             /* can't make change for credit         */

  /*****************************************
  * Loop through the tubes to determine if
  * correct change can be made.
  ******************************************/
  for (iref=coin.tube.num; iref>0; iref--)
  {
    i = coin.tube.map[iref-1];
    coincount[i] = 0;
    if (coin.status.routing&(1<<i) && coin.status.credit[i]!=0xFF)
    {
      sei();
      coincount[i] = min( credit / coin.status.credit[i], coin.tube.count[i]);
      cli();
      for ( ; coincount[i]; coincount[i]-- )
      {
        cr = credit - coincount[i] * coin.status.credit[i];
        for (jref=iref-1; jref>0; jref--)
        {
          j = coin.tube.map[jref-1];
          if (coin.status.routing&(1<<j) && coin.status.credit[j]!=0xFF)
          {
            sei();
            coincount[j] = min(cr / coin.status.credit[j], coin.tube.count[j]);
            cli();
            cr -= coincount[j] * coin.status.credit[j];
          }
        }
        if (cr==0)
          return TRUE;                /* exact change is available */
      }
    }
  }
  return FALSE;                       /* exact change is not available */
}



/******************************************************
* Determine inexact change to return.                 *
*******************************************************/
BYTE coin_change_noexact(WORD credit, BYTE *coincount)
{
  BYTE i;
  WORD m;

  /**************************************
  * Loop through the tubes to determine *
  * change to be made.                  *
  ***************************************/
  sei();
  credit /= coin.status.scaling;
  cli();
  for (i=15, m=0x8000; m; i--, m=m>>1)
  {
    coincount[i] = 0;
    if (coin.status.routing&m && coin.status.credit[i]!=0xFF)
    {
      sei();
      coincount[i] = min( credit / coin.status.credit[i], coin.tube.count[i]);
      cli();
      credit -= coincount[i] * coin.status.credit[i];
    }
  }
}


/**************************************
* Coin release for change. Return the *
* value of change not returned.       *
***************************************/
WORD coin_release(WORD credit)
{
  WORD retcr, cr, val;
  BYTE n, cnt, i, iref, err;
  BYTE begcr, valcr;

  bill_disable();
  coin.release = FALSE;   /* reset coin release flag  */
  gBill_accepted = FALSE; /* Clear bill accepted flag */
  if (!credit)            /* is credit zero?          */
    return 0;             /* return if credit is zero */
  gCalcExact = TRUE;      /* compute for exact change */

  /***************
  * Return token *
  ****************/
  if (0) // was coin.tokenused
  {
    coin.tokenused = FALSE;                 /* reset token flag         */
    for (i=0; i<16; i++)
      if (coin.tube.count[i] && coin.status.credit[i]==0xFF)
      {
        rsrc_lock(&rsrc_SCI);               /* lock SCI resource        */
        err = mdb_sendmsg(2,MDB_COIN_DISPENSE,(0x10|i),0,0, 0,0,0);
        rsrc_unlock(&rsrc_SCI);             /* unlock SCI resource      */
        if (err)
        {
          coin.tube.count[i] -= cnt;        /* reduce count of coins    */
          coin.pollstatus |= COIN_STATUS_PAYOUT_BUSY | COIN_STATUS_BUSY;
          while (coin.ok && (coin.pollstatus & (COIN_STATUS_PAYOUT_BUSY | COIN_STATUS_BUSY)))
              mcx_task_delay(MYSELF,0,TM_200ms);
        }
      }
    return 0;
  }


  /*********************
  * Intellegent payout *
  **********************/
  else if (coin.expansion&1)
  {
    cr = credit;
    sei();
    begcr = credit / coin.status.scaling;         /* units of coin factors    */
    cli();
    rsrc_lock(&rsrc_SCI);                         /* lock SCI resource        */
    err = !mdb_sendmsg(2,MDB_COIN_EXPANSION,02,1,&begcr, 0,0,0);
    rsrc_unlock(&rsrc_SCI);                       /* unlock SCI resource      */
    if (err)
      return 0;                                   /* intellegent payout fails */

    rsrc_lock(&rsrc_SCI);                         /* lock SCI resource        */
    while (FOREVER)
    {
      mcx_task_delay(MYSELF,0,TM_500ms);
      err = !mdb_sendmsg(2,MDB_COIN_EXPANSION,04,0,0, 0,1,&valcr);
      if (err)
        break;

      if (sci.rcvcount>1)
      {
        val = (WORD)valcr * coin.status.scaling;
        cr -= val;
        disp_price(cr);                             /* display total    */
      }
      else
        break;
    }
    mcx_task_delay(MYSELF,0,TM_200ms);
    err = !mdb_sendmsg(2,MDB_COIN_EXPANSION,03,0,0, 0,16,&gCoinCount);
    cnt = sci.rcvcount-1;                         /* number of coin reports   */
    rsrc_unlock(&rsrc_SCI);                       /* unlock SCI resource      */

    /*********************
    * Update tube counts *
    **********************/
    if (!err)
    {
      for (i=0; i<cnt; i++)
      {
        if (gCoinCount[i])
        {
          val                  = (WORD)gCoinCount[i]*coin.status.credit[i]*coin.status.scaling;
          mis.nr.cash_paid_change += val;
          mis.rs.cash_paid_change += val;
          mis.download.cash_paid_change += val;
          credit              -= min(credit, val);
          coin.tube.count[i]  -= min(coin.tube.count[i], gCoinCount[i]);
          coin.tube.total     -= min(coin.tube.total, gCoinCount[i]*coin.status.credit[i]);
        }
      }
    }
    return 0;
  }


  /********************
  * Calculated payout *
  *********************/
  else
  {
    if (!coin_change_exact(credit, gCoinCount)) /* calculate exact change   */
      coin_change_noexact(credit, gCoinCount);  /* calculate inexact change */

    for (iref=coin.tube.num; iref>0 && credit && coin.ok && coin.enabled; iref--)
    {
      i = coin.tube.map[iref-1];
      cr = (WORD)coin.status.credit[i];     /* coin credit              */
      cnt = gCoinCount[i];                  /* # of coins               */
      if (cnt && cr!=0xFF)
      {
        for (err=0; cnt && !err; cnt-=n)
        {
          rsrc_lock(&rsrc_SCI);               /* lock SCI resource        */
          n = min(1,cnt);
          err = !mdb_sendmsg(2,MDB_COIN_DISPENSE,((n<<4)|i),0,0, 0,0,0);
          rsrc_unlock(&rsrc_SCI);             /* unlock SCI resource      */
          if (!err)
          {
            retcr = cr*n;                     /* calulate credit returned */
            coin.tube.count[i] -= n;          /* reduce count of coins    */
            coin.tube.total -= retcr;         /* reduce credit in tubes   */
            val = retcr*coin.status.scaling;  /* amount of credit paid    */
            mis.nr.cash_paid_change += val;   /* update amount paid out   */
            mis.rs.cash_paid_change += val;   /* update amount paid out   */
            mis.download.cash_paid_change += val;
            credit -= val;                    /* reduce credit to change  */
            disp_price(credit);               /* display total            */
            coin.pollstatus |= COIN_STATUS_PAYOUT_BUSY | COIN_STATUS_BUSY;
            while (coin.ok && (coin.pollstatus & (COIN_STATUS_PAYOUT_BUSY | COIN_STATUS_BUSY)))
                mcx_task_delay(MYSELF,0,TM_20ms);
          }
        }
      }
    }
    return 0;
  }
}


/**********************************************************
* Determine if enough change exist for the credit amount. *
* Check only those tubes that have a credit value of less *
* than the credit amount. This routine uses coin factors! *
***********************************************************/
BYTE coin_tube_change(WORD credit)
{
  BYTE i;
  WORD cr, imask;

  /*****************************************
  * Loop through the tubes to determine if
  * enough change is available.
  ******************************************/
  for (cr=0, imask=0x8000, i=15; imask; i--, imask=imask>>1)
    if (coin.status.routing&imask && coin.status.credit[i]<credit)
      cr += (WORD)coin.status.credit[i] * coin.tube.count[i];

  return (cr>=credit);
}


/*****************************************
* Return true if exact change is needed. *
******************************************/
BYTE coin_exact()
{
  BYTE i, smallest=0xFF, largest=0;
  WORD imask;

  /********************************************
  * Find the largest and smallest value coin. *
  *********************************************/
  for (imask=0x8000, i=15; imask; i--, imask=imask>>1)
    if (coin.status.routing&imask && coin.status.credit[i]!=0xFF)
    {
      smallest = min(smallest,coin.status.credit[i]);
      largest  = max(largest,coin.status.credit[i]);
    }
  if (smallest==largest)
    return FALSE;                                 /* Single coin value detected             */
  else if (smallest>largest)
    return TRUE;                                  /* No non-token coins are detected        */

  for (i=15, imask=0x8000; i>0; i--, imask=imask>>1)
    if (coin.status.routing&imask && coin.status.credit[i]>smallest) /* coins in tubes only */
      if (!coin_tube_change(coin.status.credit[i]))
        return TRUE;                              /* cannot make change for coin tube value */
  return FALSE;                                   /* change can be make for all coin tubes  */
}
