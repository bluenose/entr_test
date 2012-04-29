/* Name: card.c
*
* Description:
*  Multi-Drop Bus (MDB) Routines
*
**********************************************/
#include "hdrs.h"


/********************************
* initialize card mdb structure
*********************************/
void card_init()
  {
  card.ok = card.enabled = FALSE;
  card.session.state = 0;
  }


/*********************
*  init Card MDB
**********************/
void card_poll_init()
  {
  if (!card.ok)
    {
    card.reset = TRUE;
    card.enabled = FALSE;
    card.vmc.feature   = 1;
    card.vmc.disp_cols = 0;
    card.vmc.disp_rows = 0;
    card.vmc.disp_info = 0;
    card.price.max = 0xFFFF;
    card.price.min = 0;
    card.config = FALSE;

    mcx_task_delay(MYSELF,0,TM_100ms);
    if (mdb_sendmsg(0,MDB_CARD_RESET, -1,0,0, 0,0,0))
      {
      mcx_task_delay(MYSELF,0,TM_100ms);
      if (mdb_sendmsg(5,MDB_CARD_SETUP, 0x00,4,&card.vmc, 0,0,0))
        {
        card_process();
        mcx_task_delay(MYSELF,0,TM_100ms);
        if (!card.config)
          {
          card.ok = TRUE;
          rsrc_unlock(&rsrc_SCI);
          mcx_sema_signal(TASK_pollmdb);              /* Start Polling now */
          mcx_timer_start(MYSELF,SEMA_cardconfig,TM_5sec);
          mcx_sema_wait(SEMA_cardconfig);             /* wait on timer or poll msg */
          mcx_timer_purge(MYSELF,SEMA_cardconfig,0);  /* purge timer */
          rsrc_lock(&rsrc_SCI);                       /* lock SCI resource    */
          sci.devtype = TYPE_MDB;                     /* set msg types to MDB */
          card.ok = FALSE;
          }
        card.price.max = gMaxPrice/card.reader.scaling;
        card.price.min = gMinPrice/card.reader.scaling;
        if (card.config && mdb_sendmsg(5,MDB_CARD_SETUP, 0x01,4,&card.price, 0,0,0))
          card.ok = TRUE;
        }
      }
    }
  }




/*********************************
* Process message after polling
**********************************/
void card_process()
  {
  BYTE *b;
  char  i;

  for (b=sci.buffer, i=0; i<sci.rcvcount-1; )
    {
    switch ( b[i++] )
      {
      case CARD_ERROR:
      case CARD_CMD_BAD_SEQ:
          card.session.state = 0;
          i = sci.rcvcount;
          card.ok = FALSE;
          mcx_sema_signal(SEMA_cardvend);
          mcx_sema_signal(SEMA_cardconfig);
          mcx_sema_signal(SEMA_cardsession);
          break;

      case CARD_RESET:
          card.session.state = 0;
          if (card.reset)
            card.reset = FALSE;
          else
            card_init();
          break;

      case CARD_CONFIG:
          card.session.state = 0;
          card.config = TRUE;                 /* Configure data received */
          movdata(&card.reader,&b[i],7);      /* transfer data */
          mcx_sema_signal(SEMA_cardconfig);
          i += 7;
          break;

      case CARD_DISPLAY:
          i += (BYTE)(1 + card.vmc.disp_cols * card.vmc.disp_rows);
          break;

      case CARD_SESSION_BEGIN:
          movdata(&card.session, &b[i-1], 3);       /* transfer data */
          mcx_sema_signal(SEMA_cardsession);
          i += 2;
          break;

      case CARD_SESSION_CANCEL:
      case CARD_SESSION_END:
          card.session.state = b[i-1];
          mcx_sema_signal(SEMA_cardsession);
          break;

      case CARD_VEND_APPROVED:
          movdata(&card.vend, &b[i-1], 3);        /* transfer data */
          mcx_sema_signal(SEMA_cardvend);
          i += 2;
          break;

      case CARD_VEND_DENIED:
          mcx_sema_signal(SEMA_cardvend);
          card.vend.state = b[i-1];
          break;

      case CARD_CANCELLED:
          card.session.state = CARD_SESSION_CANCEL;
          card.state = CARD_CANCEL;
          break;

      case CARD_PERIPHERAL_ID:
          i += 29;
          break;

      case CARD_DIAGNOSTIC:
          i = sci.rcvcount;
          break;
      }
    }
  }


/******************
*  poll card MDB
*******************/
void card_poll( void )
  {
  if (card.ok)
    {
    if (mdb_sendmsg(card.reader.maxtime,MDB_CARD_POLL,  -1,0,0, 0,0,0))
      card_process();
    else
      card.ok = FALSE;
    }
  }


/******************
* Enable card mdb
*******************/
void card_enable()
  {
  if (card.ok && !card.enabled)
    {
    rsrc_lock(&rsrc_SCI);             /* lock SCI resource    */
    if (mdb_sendmsg(card.reader.maxtime,MDB_CARD_READER, CARD_ENABLE,0,0, 0,0,0))
      card.enabled = TRUE;
    rsrc_unlock(&rsrc_SCI);           /* unlock SCI resource  */
    }
  }



/*******************
* Disable card mdb
********************/
void card_disable()
  {
  if (card.ok && card.session.state!=0 && card.session.state!=CARD_SESSION_END)
    {
    card_complete_session();                      /* End session to eject card        */
    if (card.session.state!=CARD_SESSION_END)
      {
      mcx_timer_start(MYSELF,SEMA_cardsession,card.reader.maxtime*TM_1sec);
      mcx_sema_wait(SEMA_cardsession);            /* wait on timer or session msg     */
      mcx_timer_purge(MYSELF,SEMA_cardsession,0); /* purge timer                      */
      if (card.session.state!=CARD_SESSION_END)
        return ;                                  /* Could not end session - bail out */
      }
    }
  if (card.ok && card.enabled)
    {
    rsrc_lock(&rsrc_SCI);             /* lock SCI resource    */
    if (mdb_sendmsg(card.reader.maxtime,MDB_CARD_READER, CARD_DISABLE,0,0, 0,0,0))
      card.enabled = FALSE;
    rsrc_unlock(&rsrc_SCI);           /* unlock SCI resource  */
    }
  }


/***************************************
* Begin vend session
****************************************/
int card_vend_begin(WORD pri, WORD num)
  {
  struct {
    WORD price;
    WORD number;
  } item;

  item.price = pri;
  item.number = num;
  rsrc_lock(&rsrc_SCI);             /* lock SCI resource    */
  card.vend.state = 0;
  if (mdb_sendmsg(card.reader.maxtime,MDB_CARD_VEND,CARD_VEND_REQUEST,4,&item, 0,0,0))
    {
    card_process();
    mcx_task_delay(MYSELF,0,TM_100ms);
    if (card.vend.state!=CARD_VEND_APPROVED)
      {
      card.ok = TRUE;
      rsrc_unlock(&rsrc_SCI);
      mcx_sema_signal(TASK_pollmdb);              /* Start Polling now */
      mcx_timer_start(MYSELF,SEMA_cardvend,TM_5sec);
      mcx_sema_wait(SEMA_cardvend);               /* wait on timer or poll msg */
      mcx_timer_purge(MYSELF,SEMA_cardvend,0);    /* purge timer */
      rsrc_lock(&rsrc_SCI);                       /* lock SCI resource    */
      sci.devtype = TYPE_MDB;                     /* set msg types to MDB */
      }
    }
  rsrc_unlock(&rsrc_SCI);           /* unlock SCI resource    */

  return (card.vend.state==CARD_VEND_APPROVED);
  }


/***************************************
* Register vend as successful
****************************************/
int card_vend_successful(WORD num)
  {
  BYTE status;

  rsrc_lock(&rsrc_SCI);             /* lock SCI resource    */
  status = mdb_sendmsg(card.reader.maxtime,MDB_CARD_VEND,CARD_VEND_SUCCESS,2,&num, 0,0,0);
  rsrc_unlock(&rsrc_SCI);           /* unlock SCI resource    */

  return status;
  }



/***************************************
* Register vend as failed
****************************************/
int card_vend_failure()
  {
  BYTE status;

  rsrc_lock(&rsrc_SCI);             /* lock SCI resource    */
  status = mdb_sendmsg(card.reader.maxtime,MDB_CARD_VEND,CARD_VEND_FAILURE,0,0, 0,0,0);
  rsrc_unlock(&rsrc_SCI);           /* unlock SCI resource    */

  return status;
  }



/***************************************
* Register vend as successful
****************************************/
int card_complete_session()
  {
  BYTE status;

  rsrc_lock(&rsrc_SCI);             /* lock SCI resource    */
  if (!(status = mdb_sendmsg(card.reader.maxtime,MDB_CARD_VEND,CARD_SESSION_COMPLETE,0,0, 0,0,0)))
    {
    card.session.state = 0;
    card_process();
    }
  rsrc_unlock(&rsrc_SCI);           /* unlock SCI resource    */

  return status;
  }

