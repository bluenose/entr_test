/*********************************************
* Name: mdb.c
*
* Description:
*  Multi-Drop Bus (MDB) Routines
*
**********************************************/
#include "hdrs.h"



/************
* Move data
*************/
BYTE *movdata(void *to, void *from, int count)
  {
  BYTE *t, *f;

  for ( t=to, f=from; count--; )
    *t++ = *f++;

  return t;
  }



/**************************************
* Queue mdb device block to be sent.
* This should have a high priority.
***************************************/
void task_mdb( void )
{
MCX_MAILBOX *mbox;
BYTE i, rcvcount, chksum, *buf, *dat;

while (FOREVER)
  {
  mcx_mailbox_receive(0,0,&mbox);         /* wait for message           */

  /*********************************
  * Initialize buffer to send
  **********************************/
  sci.rcvmax   = 36;                      /* max mdb rcv size           */
  sci.sndpnt   = 0;                       /* init to snd buf start      */
  sci.sndcount = (BYTE)mbox->message;     /* size of snd buffer         */
  sci.state = STATE_send;                 /* set state to send msg      */
  rcvcount = 0xFF;                        /* Receive check              */
  SCCR2 |= TCIE;                          /* Turn xmt interrupts on     */

  while (sci.state!=STATE_done)
    {
    mcx_task_delay(MYSELF,0,TM_8ms);    /* wait */
    if (sci.state==STATE_receive)
      {
      if (rcvcount!=sci.rcvcount)       /* chars being rcved? */
        rcvcount = sci.rcvcount;        /* yes, update check  */
      else
        {                               /* receive is idle  */
        sci.state = STATE_done;         /* Abort receive    */
        sci.msgstatus = STATUS_NAK;     /* Force NAK return */
        }
      }
    }
    mcx_sema_signal( SEMA_mdb );        /* signal the job done   */
  }
}



/***********************
* initialize mdb data
************************/
void mdb_init()
	{
  sci.state = STATE_done;
  coin_init();
	}



/************************************************
* Format and send a message by the MDB master
*   resptm   = Maximum retry time in seconds
*   cmd      = MDB command
*   subcmd   = MDB sub-command (-1 = none)
*   sndlen   = Size of additional MDB data
*   sndbuf   = Address of additional MDB data
*   dataoff  = Offset to relevent return data
*   datalen  = Length of relevent return data
*   databuf  = Address to transfer relevent data
*  Return:
*    1 = ACK
*    0 = NAK or no response
*************************************************/
BYTE mdb_sendmsg(BYTE resptm, BYTE cmd, int subcmd,
  BYTE sndlen, void *sndbuf,
  BYTE dataoff, BYTE datalen, void *databuf)
  {
  MCX_MAILBOX mbox;
  BYTE  i, count, chksum, msglen;
  BYTE  *tobuf, *frombuf;

  resptm *= 10;        /* maximum retry time in tenths of seconds */
  while (FOREVER)
    {
    /**************************************
    * xfer data to sci buffer & determine
    * command byte & checksum
    ***************************************/
    tobuf = sci.buffer;
    chksum = *tobuf++ = cmd;                        /* setup command      */
    count  = sndlen + 2;                            /* message size       */
    if ( subcmd>=0 )
      {
      count++;                                      /* message size       */
      chksum += *tobuf++ = (BYTE)subcmd;            /* setup sub-command  */
      }
    for (i=sndlen, frombuf=sndbuf; i--; )
      chksum += *tobuf++ = *frombuf++;              /* setup data         */
    *tobuf = chksum;                                /* setup checksum     */

    mbox.message = (void *)count;                   /* size of sci message */
    mcx_mailbox_sendw (TASK_mdb, SEMA_mdb, &mbox);  /* queue in message    */

    msglen = sci.rcvcount-1;
    if (databuf && msglen>dataoff && STATUS_ACK == sci.msgstatus)
      {
      count = msglen - dataoff;                     /* set "i" to transfer */
      i = min(datalen,count);                       /* only bytes recv'd   */
      for (tobuf=databuf, frombuf=sci.buffer+dataoff; i--; )
          *tobuf++ = *frombuf++;                    /* transfer data       */
      if (datalen>count)
        for (i=datalen-count; i--; )                /* zero partial buffer */
          *tobuf++ = 0;
      }
    if (sci.msgstatus==STATUS_ACK || !resptm--)     /* Loop again?            */
      break;                                        /* break out of this loop */

    mcx_task_delay(MYSELF,0,TM_100ms);              /* No Freight Training!   */
    }

  i = (STATUS_ACK == sci.msgstatus);                /* 1=success, 0=fail    */

  return i;
	}


/*****************
*  init MDB Task
******************/
void task_initmdb( void )
  {
  mcx_task_delay(MYSELF,0,TM_1sec);   /* Initial Wait */

  while ( FOREVER )
    {
    /*************************
    * Lock SCI resource
    **************************/
    rsrc_lock(&rsrc_SCI);             /* lock SCI resource    */
    sci.devtype = TYPE_MDB;           /* set msg types to MDB */

#if _COIN_
    coin_poll_init();
#endif

#if _BILL_
    bill_poll_init();
#endif

//#if _CARD_
//    card_poll_init();
//#endif

    /***********************************************
    * Unlock resource and temorarily suspend task
    ***********************************************/
    rsrc_unlock(&rsrc_SCI);
    mcx_task_delay(MYSELF,0,TM_5sec);
    }
  }





/******************
*  poll MDB Task
*******************/
void task_pollmdb( void )
  {
  while ( FOREVER )
    {
    rsrc_lock(&rsrc_SCI);   /* lock SCI resource    */
    sci.devtype = TYPE_MDB; /* set msg types to MDB */

#if _COIN_
    coin_poll();
#endif

#if _BILL_
    bill_poll();
#endif

//#if _CARD_
//    card_poll();
//#endif

    rsrc_unlock(&rsrc_SCI);
    mcx_task_delay(MYSELF,0,TM_100ms);
    }
  }

