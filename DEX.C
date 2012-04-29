/*********************************************
* Name: dex.c
*
* Description:
*  DEX Routines
*
**********************************************/
#include "hdrs.h"

#define D4 10000L
#define D6 1000000L
#define D8 100000000L

char TempString[30];


/**************************************************
* Create string that describes the shutter status *
***************************************************/
char *ShutterStatus()
  {
  sprintf(TempString, "A-%s, B-%s",
      gSwitch[SW_EXTEND1]?"Extended":"Retracted",
      gSwitch[SW_EXTEND2]?"Extended":"Retracted");
  return TempString;
  }


/*************************************************
* Create string that describes the keypad status *
**************************************************/
char *KeyStatus()
  {
  int  i;
  long m;
  strcpy(TempString,"");
  for (m=1, i=0; i<4; i++, m<<=1)
    strcat(TempString, (gKeyStuck&m ? "1":"0"));
  return TempString;
  }



/*********************************************
* Create string that describes the RO status *
**********************************************/
char *ROstatus()
  {
  sprintf(TempString, "%s, %s",
      gROstate?"On ":"Off", gSwitch[SW_RO_PRESSURE]?"High":"Low");
  return TempString;
  }





/******************************
* Create string for US prices *
*******************************/
char *FmtDollars(long pr)
  {
  sprintf(TempString, "%ld.%02ld", pr/100, pr%100);
  return TempString;
  }



/************************************************
* Do DEX transfer of MIS data, beep for success *
* or failure, then hang until DEX unit unplugs. *
*************************************************/
void dex_xfer()
  {
  int  success=FALSE, ok=FALSE;
  BYTE i, j;
  long totgals;

  gChase = CHASE_DEX;
  coin_disable();
  bill_disable();
  //card_disable();
  dex_connect();
  if (*(WORD *)SN_CODE!=SN_SET)
    *(long *)SN_DATA = 0;         /* zero SN if it is not set */

  totgals =    mis.nr.sel[0].vends + mis.nr.sel[3].vends  +
            3*(mis.nr.sel[1].vends + mis.nr.sel[4].vends) +
            5*(mis.nr.sel[2].vends + mis.nr.sel[5].vends);

  if (dex_session_begin())
    {
    dex.start = STX;
    dex.end = ETB;

    if (gMaster)
      {
      gROstate = ON;
      mcx_task_delay(MYSELF,0,TM_10sec);
      }

    if (dex_sndrec("\nID"))
    if (dex_sndrec(" Entre-Pure version: %X.%2X", progvers>>8, progvers&0xFF))
    if (dex_sndrec(" Master/Slave: %s", gMaster?"Master":"Slave"))
    if (dex_sndrec(" Serial number: %X", *(long *)SN_DATA))
    if (dex_sndrec("\nMIS DATA"))
    if (dex_sndrec(" Cash: %s", FmtDollars(mis.nr.cash)))
    if (dex_sndrec(" Coins: %s", FmtDollars(mis.nr.cash2box+mis.nr.cash2tubes)))
    if (dex_sndrec(" Coins to tubes: %s", FmtDollars(mis.nr.cash2tubes)))
    if (dex_sndrec(" Coins to box: %s", FmtDollars(mis.nr.cash2box)))
    if (dex_sndrec(" Bills: %s", FmtDollars(mis.nr.bill_cash)))
    if (dex_sndrec(" Gallons sold: %ld", totgals))
    if (dex_sndrec(" Tokens: %ld", mis.nr.token_vends))
    if (dex_sndrec(" Token Credit Value: %s", FmtDollars((long)mis.nr.token_value)))

    if (dex_sndrec(" Count of all gallons Side A: %ld",
        mis.nr.sel[GAL1].vends + 3*mis.nr.sel[GAL3].vends + 5*mis.nr.sel[GAL5].vends))
    if (dex_sndrec(" Count of 1 gallons Side A: %ld", mis.nr.sel[GAL1].vends))
    if (dex_sndrec(" Count of 3 gallons Side A: %ld", mis.nr.sel[GAL3].vends))
    if (dex_sndrec(" Count of 5 gallons Side A: %ld", mis.nr.sel[GAL5].vends))

    if (dex_sndrec(" Count of all gallons Side B: %ld",
        mis.nr.sel[GAL1+3].vends + 3*mis.nr.sel[GAL3+3].vends + 5*mis.nr.sel[GAL5+3].vends))
    if (dex_sndrec(" Count of 1 gallons Side B: %ld", mis.nr.sel[GAL1+3].vends))
    if (dex_sndrec(" Count of 3 gallons Side B: %ld", mis.nr.sel[GAL3+3].vends))
    if (dex_sndrec(" Count of 5 gallons Side B: %ld", mis.nr.sel[GAL5+3].vends))



    if (dex_sndrec(" Count of total gallons since last dex download: %ld",
        mis.download.sel[GAL1].vends+(3*mis.download.sel[GAL3].vends)+(5*mis.download.sel[GAL5].vends)+
        mis.download.sel[GAL1+3].vends+(3*mis.download.sel[GAL3+3].vends)+(5*mis.download.sel[GAL5+3].vends)))
    if (dex_sndrec(" Count of tokens since last download: %ld", mis.download.token_vends))
    if (dex_sndrec(" Token credit since last download: %s", FmtDollars((long)mis.download.token_value)))


    if (dex_sndrec(" High price: %s", FmtDollars((long)gMaxPrice)))
    if (dex_sndrec(" One gallon price: %s", FmtDollars((long)gPrice[GAL1])))
    if (dex_sndrec(" Three gallon price: %s", FmtDollars((long)gPrice[GAL3])))
    if (dex_sndrec(" Five gallon price: %s", FmtDollars((long)gPrice[GAL5])))



    if (dex_sndrec("\nMACHINE DATA"))
      {
      if (gMaster)
        {
        if (dex_sndrec(" Vending status: %s", gAllSoldOut?"Disabled":"Enabled"))
        if (dex_sndrec(" TDS status: %s", gSwitch[SW_TDS_MONITOR]?"OK":"Dirty"))
        if (dex_sndrec(" UV primary status: %s", gSwitch[SW_UV_DETECT_PRI]?"OK":"Failed"))
        if (dex_sndrec(" UV secondary status: %s", gSwitch[SW_UV_DETECT_SEC]?"OK":"Failed"))
        if (dex_sndrec(" Shutter status: %s", ShutterStatus()))
        if (dex_sndrec(" Front panel switches: %s", KeyStatus()))
        if (dex_sndrec(" RO states: %s", ROstatus()))
        if (dex_sndrec(" Pre-treatment: %s", gSwitch[SW_PRE_TREATMENT]?"High":"Low"));
        if (dex_sndrec(" Post-treatment: %s", gSwitch[SW_POST_TREATMENT]?"High":"Low"))
          ok = TRUE;
        }
      else
        {
        if (dex_sndrec(" Vending status: %s", gAllSoldOut?"Disabled":"Enabled"))
        if (dex_sndrec(" Shutter status: %s", ShutterStatus()))
        if (dex_sndrec(" Front panel switches: %s", KeyStatus()))
          ok = TRUE;
        }
      }
    }
  if (ok)
    success = dex_session_end();
  dex_disconnect();

  /********************************
  * Indicate success, or failure. *
  *********************************/
  gChase = 0;
  disp_printf("");
  mcx_task_delay(MYSELF,0,TM_400ms);
  disp_printf(success?"done":"FAIL");
  if (success)
  {
    dex_mis_clear();
    mis.dexreads++;
  }
  mcx_task_delay(MYSELF,0,TM_3sec);
  }


/**************************************************
* Suspend all non-DEX tasks - This should only be
* called when SCI is locked for DEX.
***************************************************/
void dex_suspend_all()
  {
  mcx_task_suspend(TASK_mdb);         /* low level MDB messaging */
  mcx_task_suspend(TASK_initmdb);     /* do MDB setup   */
  mcx_task_suspend(TASK_pollmdb);     /* do MDB polling */
  mcx_task_suspend(TASK_keypad);      /* keypad task    */
  }


/**********************************
* Unsuspend all non-DEX tasks.
***********************************/
void dex_resume_all()
  {
  mcx_task_resume(TASK_mdb);         /* low level MDB messaging */
  mcx_task_resume(TASK_initmdb);     /* do MDB setup   */
  mcx_task_resume(TASK_pollmdb);     /* do MDB polling */
  mcx_task_resume(TASK_keypad);      /* keypad task    */
  }










/**************************
* Calculate Block crc16
***************************/
WORD updcrc(WORD crc, BYTE c)
  {
  int count, tempchar;

  tempchar = c;
  count = 8;
  while(--count >= 0)
    {
    if((tempchar ^ crc) & 0x0001)
      {
      crc >>=1;
      crc ^= 0xa001;
      }
    else
      crc >>= 1;

    tempchar >>= 1;
    }

  return crc;
  }



/**********************
* Send a DEX message.
***********************/
void dex_snd()
  {
  dex.rcvstate = DEX_READY;               /* make ready to rcv reply    */
  sci.rcvmax   = 36;                      /* max mdb rcv size           */
  sci.sndpnt   = 0;                       /* pointer into buffer        */
  SCCR2 |= TCIE;                          /* Turn xmt interrupts on     */
  }



/**************************
* Receive a DEX message.
***************************/
void dex_rcv()
  {
  BYTE i;
  /*****************************************
  * Loop for 1.1 seconds to received msg
  ******************************************/
  for ( i=0; i<137 && dex.rcvstate!=DEX_DONE; i++)
    mcx_task_delay(MYSELF,0,TM_8ms);
  }




/********************
* Do DEX disconnect
*********************/
void dex_disconnect( void )
  {
  while ( !(SCSR & TC) )
    ;                                     /* wait till xmit complete */
  sci.devtype = TYPE_MDB;                 /* default to MDB type */
  sci_mdb_mode();                         /* default to MDB mode */
  rsrc_unlock(&rsrc_SCI);                 /* unlock SCI resource */
  dex_resume_all();                       /* resume all non-DEX tasks */
  }




/******************
* End DEX session
*******************/
int dex_session_end( void )
  {
  *sci.buffer  = EOT;                     /* EOT */
  sci.sndcount = 1;                       /* size of snd buffer  */
  dex_snd();                              /* send dex message    */

  return 1;
  }



/***************************
* Do DEX connect sequence
****************************/
void dex_connect( void )
  {
  rsrc_lock(&rsrc_SCI);                   /* lock SCI resource  */
  while ( !(SCSR & TC) )                  /* xmit complete?     */
    ;                                     /* wait till xmit complete */
  sci_dex_mode();                         /* set to dex mode    */
  sci.devtype = TYPE_DEX;                 /* set to DEX type    */
  dex_suspend_all();                      /* suspend all non-DEX tasks */
  /*beeper(0);                              /* force beeper off   */
  }



/*********************
* Begin DEX session
**********************/
int dex_session_begin( void )
  {
  BYTE *b, done, i, tries;

  /**************************
  * Send ENQ & get response
  ***************************/
  dex.seq = '0';
  dex.start = SOH;
  dex.end = ETX;
  for ( done=FALSE, tries=0; !done; mcx_task_delay(MYSELF,0,TM_1sec), tries++ )
    {
    if ( tries==10 )                        /* return on non-response */
      return 0;
    *sci.buffer  = ENQ;                     /* Enquire for downloader     */
    sci.sndcount = 1;                       /* size of snd buffer         */
    dex_snd();                              /* send dex message    */
    dex_rcv();                              /* wait to recieve msg */
    done = ( dex.rcvstate==DEX_DONE && *sci.buffer=='0' );
    }

  /**********************************
  * Initial setup
  * id, function, ucc rev, & level
  ***********************************/
  dex.do_crc = FALSE;
  dex.add_crlf = FALSE;
  if (!dex_sndrec("%s%s",VCS_ID,"SR01L01"))
    return 0;

  /****************************
  * Send EOT and wait for ENQ
  *****************************/
  for ( done=FALSE, tries=0; !done; mcx_task_delay(MYSELF,0,TM_100ms), tries++ )
    {
    if ( tries==10 )                        /* return on non-response */
      return 0;
    *sci.buffer  = EOT;                     /* EOT                */
    sci.sndcount = 1;                       /* size of snd buffer */
    dex_snd();                              /* send dex message   */
    dex_rcv();                              /* wait to recieve msg */

    done = ( dex.rcvstate==DEX_DONE && *sci.buffer==ENQ );
    }


  /****************************
  * Send Good response
  *****************************/
  dex.seq = '0';                          /* 1st frame #  */
  sci.buffer[0] = DLE;                    /* DLE          */
  sci.buffer[1] = dex.seq;                /* frame number */
  sci.sndcount = 2;                       /* size of snd buffer */
  dex_snd();                              /* send DEX msg */

  /****************************
  * Get HHC ID
  *****************************/
  dex_rcv();                              /* rcv DEX msg  */

  /****************************
  * Send Good response
  *****************************/
  dex.seq ^= 1;                           /* next frame # */
  sci.buffer[0] = DLE;                    /* DLE          */
  sci.buffer[1] = dex.seq;                /* frame number */
  sci.sndcount = 2;                       /* size of snd buffer */
  dex_snd();                              /* send DEX msg */

  /****************************
  * Get EOT
  *****************************/
  dex_rcv();                              /* rcv DEX msg  */


  /**************************
  * Send ENQ & get response
  ***************************/
  dex.seq = '0';
  for ( done=FALSE, tries=0; !done; mcx_task_delay(MYSELF,0,TM_1sec), tries++ )
    {
    if ( tries==10 )                        /* return on non-response */
      return 0;
    *sci.buffer  = ENQ;                     /* Enquire for downloader     */
    sci.sndcount = 1;                       /* size of snd buffer         */
    dex_snd();                              /* send dex message    */
    dex_rcv();                              /* wait to recieve msg */
    done = ( dex.rcvstate==DEX_DONE && *sci.buffer=='0' );
    }
  dex.record_crc = 0;
  dex.do_crc = TRUE;
  dex.add_crlf = TRUE;

  return 1;
  }



/*************************************************************
* Send DEX transaction record - add CR, LF to end of record,
* and compute crc16 for record if 'do_crc' is TRUE.
**************************************************************/
int dex_sndrec (char *fmt, char *args)
  {
  BYTE i, tries, length, done, *p;


  /**************
  * Send record *
  ***************/
  dex.setcount++;
  for ( done=FALSE, tries=0; !done; mcx_task_delay(MYSELF,0,TM_100ms), tries++ )
    {
    if ( tries==10 )                            /* return on non-response */
      return 0;
    p = sci.buffer;
    *p++ = DLE;
    *p++ = dex.start;                           /* SOH or STX */

    *p = '\0';
    _print(&p, fmt, &args);                     /* format DEX message     */
    length = (BYTE)strlen((char *)sci.buffer);  /* determine msg length   */
    p = &sci.buffer[length];                    /* p = next char position */

    if ( dex.add_crlf )
      {
      *p++ = CR;
      *p++ = LF;
      length += 2;  /* length = <DLE> <SOH/STX> <msg> <CR> <LF> */
      }

    /* Compute block crc - Skip <DLE> <SOH/STX> in CRC16 calculate, but include <ETB/ETX>. */
    for ( dex.block_crc=0, i=2; i<length; i++ )
      dex.block_crc = updcrc(dex.block_crc, sci.buffer[i]);
    dex.block_crc = updcrc(dex.block_crc, dex.end);

    /* Compute record crc - include no delimeters */
    if ( !tries && dex.do_crc )
      for ( i=2; i<length; i++ )
        if ( sci.buffer[i]>=' ' && sci.buffer[i]<='~' )
          dex.record_crc = updcrc(dex.record_crc, sci.buffer[i]);

    *p++ = DLE;                                     /* DLE */
    *p++ = dex.end;                                 /* ETB/ETX */
    *p++ = (BYTE)(dex.block_crc&0xFF);              /* low crc16 */
    *p++ = (BYTE)(dex.block_crc>>8);                /* high crc16 */
    sci.sndcount = length+4;                        /* size of buffer to send */
    dex_snd();                                      /* send dex message    */
    dex_rcv();                                      /* wait to recieve msg */

    done = ( dex.rcvstate==DEX_DONE && dex.rcvstatus==DEX_STATUS_OK );
    }

  return 1;
  }
