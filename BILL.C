/*********************************************
* Name: bill.c
*
* Description:
*  Multi-Drop Bus (MDB) Routines
*
**********************************************/
#include "hdrs.h"


/********************************
* initialize bill mdb structure
*********************************/
void mdb_bill_init()
{
  bill.ok = bill.enabled = FALSE;
  bill.escrow.full = FALSE;
  bill.tokenused = FALSE;
}

/*****************
*  Init Bill MDB
******************/
void bill_poll_init( void )
  {
  if ( !bill.ok )
    {
    bill.reset = TRUE;
    bill.enabled = FALSE;
    if (mdb_sendmsg(0,MDB_BILL_RESET,     -1,0,0, 0,0,0))
      if (mdb_sendmsg(5,MDB_BILL_STATUS,    -1,0,0, 0,27,&bill.status))
        bill.ok = TRUE;
    }
  }

/**************************************************
* Bill accepted -  by = 1yyyxxxx
*
*   yyy = Bill routing.
*     000 Stacked
*     001 Escrow
*     010 Returned
*     011 Unused
*     100 Rejected
*
*   xxxx = Bill type.
*
**************************************************/
void bill_accepted(BYTE by)
  {
  BYTE y, x, i;
  WORD cr;

  y = (0x70 & by)>>4;
  x = 0x0F & by;

  switch (y) {
    case 0:
      if (x == 1)
        bill.tokenused = bill.status.credit[1]==0xFF;
      if (bill.tokenused)
      {
        cr = 0;                         // Entrepure only has coin tokens
      }
      else
        cr = bill.status.credit[x]*bill.status.scaling;
      if (!(coin.tokenused || bill.tokenused))
      {
        mis.nr.bill_cash += cr;
        mis.rs.bill_cash += cr;
        mis.download.bill_cash += cr;
      }

      gBill_accepted = TRUE;
      sei();
      gCredit += cr;
      cli();
      gReturnChange = FALSE;                    // credit added - don't kick out
      break;
    case 1:
      if (x == 1)
        bill.tokenused = bill.status.credit[1]==0xFF;
      if (bill.tokenused)
      {
        cr = gPrice[GAL5];
      }
      else
        cr = bill.status.credit[x]*bill.status.scaling;
      bill.escrow.full = TRUE;
      bill.escrow.type = x;
      break;
    }
  }


/******************************************
* Bill escrow -  accept or reject bill
*******************************************/
void bill_escrow()
  {
  WORD cr=0, billcr=0;
  BYTE i, ok=0;

  if (coin.ok)
    {
    for ( i=0; i<16; i++ )
      if ( coin.tube.count[i] )
        cr += coin.tube.count[i] * coin.status.credit[i] * coin.status.scaling;
    i = bill.escrow.type;
    billcr = bill.status.credit[i] * bill.status.scaling;
    if ( billcr<=cr )
      ok = 1;
    }
  else
    ok = 1;

  i = bill.escrow.type;
  if (mdb_sendmsg(5,MDB_BILL_ESCROW,  ok,0,0, 0,0,0))
    bill.escrow.full = FALSE;
  }




void bill_attempts(BYTE by)
  {}





/*****************
* Poll Bill MDB
******************/
void bill_poll( void )
  {
  BYTE *b;
  int i;
  WORD pollstatus;

  if (bill.ok)
    {
    if (!mdb_sendmsg(5,MDB_BILL_POLL,  -1,0,0, 0,0,0))
      bill.ok = FALSE;
    else
      {
      pollstatus = 0;
      for ( b=sci.buffer, i=sci.rcvcount-1; i>0; b++, i-- )
        {
        if ( *b&0x80 )
          bill_accepted(*b);
        else if ( *b&0x60==0x40 )
          bill_attempts(*b&0x3F);
        else if ( !(*b&0xF0) )
            pollstatus |= 1L<<*b;
        }
      bill.pollstatus = pollstatus;
      }
    if (bill.escrow.full)
      bill_escrow();
    }
  }





/******************
* Enable bill mdb
*******************/
void bill_enable()
  {
  static long old_enable=0;
  long  new_enable;
  int   mask, depcr, availcr;
  BYTE  i, iref;
  static int  lastcredit=-1;
  static BYTE transition=TRUE;


  /********************************
  * Invalidate old enable pattern *
  *********************************/
  if (!bill.ok)
    old_enable = 0;

  /******************
  * Detect a change *
  ******************/
  if (!bill.ok || gCredit!=lastcredit || !bill.enabled || bill.newcointotal)
    transition = TRUE;

  /**********************************
  * Return when nothing has changed *
  ***********************************/
  if (!bill.ok || !transition)
    return;
  else
  {
    lastcredit = gCredit;
    transition = FALSE;
    bill.newcointotal = FALSE;
  }


  /********************************************************************
  * Determine bills to accept. Accept all bills with available credit *
  * in coin tubes. Reject all coins when credit exceeds the highest   *
  * product price.                                                    *
  *********************************************************************/
  if (gCredit>=gMaxPrice)    new_enable = 0;                               /* reject all bills when max price is exceeded    */
  else if (!coin.ok || !coin.status.routing)
    {
    for (i=0; i<16; i++)
      {
      if (bill.status.credit[i]==1)
        {
          new_enable |= 1<<i;                     /* accept ONLY $1 -- EntrePure only */
        }
      }
    }
  else
    {
    availcr = coin.tube.total*coin.status.scaling - gCredit;  /* available credit for making change */
    for (i=0, new_enable=0; i<16; i++)
      {
      if (bill.status.credit[i]==0xFF || availcr>=bill.status.credit[i]*bill.status.scaling)
        {
        if (gCredit==0)
          new_enable |= 1<<i;                     /* accept all valid bills when there are no tubes */
        else if ((bill.status.credit[i]!=0xFF) && (!bill.tokenused))   /* Ver 1.22  don't accept any new bills on token */
          new_enable |= 1<<i;                     /* accept all valid bills except tokens           */
        }
      }
    }
  new_enable = new_enable<<16;



  /**********************************
  * Send message to MDB bill device *
  ***********************************/
  if ((bill.ok && old_enable!=new_enable) || (bill.ok && !bill.enabled))
    {
    rsrc_lock(&rsrc_SCI);             /* lock SCI resource    */
    if (mdb_sendmsg(5,MDB_BILL_BILLTYPE,-1,4,&new_enable, 0,0,0))
      bill.enabled = TRUE;
    rsrc_unlock(&rsrc_SCI);           /* unlock SCI resource    */
    old_enable = new_enable;
    }
  }




/*******************
* Disable bill mdb
********************/
void bill_disable()
  {
  const static long bill_disable_all=0;

  if (bill.ok && bill.enabled)
    {
    rsrc_lock(&rsrc_SCI);             /* lock SCI resource    */
    if (mdb_sendmsg(5,MDB_BILL_BILLTYPE,  -1,4,&bill_disable_all, 0,0,0))
      bill.enabled = FALSE;
    rsrc_unlock(&rsrc_SCI);           /* unlock SCI resource    */
    }
  }


/**************************************
* Coin release for change. Return the *
* value of change not returned.       *
***************************************/
WORD bill_release()
  {
  bill.tokenused = FALSE;   /* reset token used flag              */

  return 0;                 /* return the amt of credit returned  */
  }
