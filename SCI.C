/*********************************************
* Name: sci.c
*
* Description:
*  Low level SCI drivers for MDB & DEX
*
**********************************************/
#include "hdrs.h"


/***********************
* Initialize sci port.
* Default to MDB.
************************/
void sci_init()
	{
  BAUD  = B9600;                  /* set baud rate */
  SCCR1 = M;                      /* 9-bit */
  SCCR2 = (BYTE)(RIE | TE | RE);  /* TCIE, RIE, TE, RE */
	}



/*******************************************
* Rcv/Snd characters from SCI port for mdb.
********************************************/
void sci_mdb()
	{
	BYTE data, status, ctrl;
	BYTE *b, chksum, i;

  status = SCSR;        /* read status, assume char ready */

  if (status&RDRF)      /* Character ready */
		{
    ctrl   = SCCR1;     /* get control & bit nine data               */
    data   = SCDR;      /* get char data. This will clear RDRF flag. */

    if (sci.state==STATE_receive)
			{
      if (sci.rcvcount<=sci.rcvmax)
        sci.buffer [ sci.rcvcount++ ] = (BYTE)data;

			if (ctrl & R8)
				{
        for (i=1, b=sci.buffer, chksum=0; i<sci.rcvcount; i++)
        chksum += *b++;

        sci.msgstatus = (chksum==*b) ? STATUS_ACK : STATUS_NAK;

        if ( sci.msgstatus==STATUS_ACK && sci.rcvcount>1 )
          {
          SCCR1 = 0x10;       /* T8=0, M  */
          SCDR =  MDB_ACK;    /* send ACK */
          }

        sci.state = STATE_done;
				}
			}
		}

  else if (status&TC) /* Transmit ready */
		{
    SCCR1 = sci.sndpnt ? 0x10 : 0x50;   /* Set 9th bit to mark or space */
    SCDR  = sci.buffer [ sci.sndpnt++ ];
    if (sci.sndpnt==sci.sndcount)
			{
      SCCR2 &= ~TCIE;                    /* Turn xmt interrupts off */
      sci.state = STATE_receive;
      sci.rcvcount = 0;
			}
		}
	}


BYTE lastchar;

/*******************************************
* Rcv/Snd characters from SCI port for dex.
********************************************/
void sci_dex()
	{
  BYTE data, status;

  status = SCSR;        /* read status, assume char ready */
  if (status&RDRF)      /* Character ready */
		{
    data = SCDR;        /* get char data. This will clear RDRF flag. */

    switch ( dex.rcvstate )
      {
      case DEX_CRC16A:
        dex.block_crc = data<<8;
        dex.rcvstate  = DEX_CRC16B;
        break;

      case DEX_CRC16B:
        dex.block_crc |= data;
        dex.rcvstate  = DEX_DONE;
        break;

      case DEX_DONE:
      case DEX_READY:
        if ( data==EOT || data==ENQ )
          {
          sci.buffer[0] = data;
          sci.rcvcount  = 1;
          dex.seq = '0';
          dex.rcvstate = DEX_DONE;
          dex.rcvstatus = DEX_STATUS_OK;
          }
        else if ( data==DLE )
          {
          dex.rcvstate = DEX_BUSY;
          sci.rcvcount = 0;
          }
        break;

      case DEX_BUSY:
        if ( lastchar!=DLE )
          {
          if (sci.rcvcount<=sci.rcvmax)             /* if buffer is not full, */
            sci.buffer [ sci.rcvcount++ ] = data;   /* drop chars into buffer */
          }
        else
          {
          if ( data==SOH || data==STX )           /* DLE SOH  or  DLE STX */
            {
            sci.buffer[0] = data;
            sci.rcvcount = 1;
            dex.rcvstate = DEX_BUSY;
            }
          else if ( data==dex.seq )               /* DLE 0  or  DLE 1 */
            {
            sci.buffer[0] = data;
            sci.rcvcount = 1;
            dex.seq ^= 1;
            dex.rcvstate = DEX_DONE;
            dex.rcvstatus = DEX_STATUS_OK;
            }
          else if ( data==ETX || data==ETB )      /* DLE ETX  or  DLE ETB */
            {
            if (sci.rcvcount<=sci.rcvmax)             /* if buffer is not full, */
              sci.buffer [ sci.rcvcount++ ] = data;   /* drop data into buffer */
            dex.rcvstate = DEX_CRC16A;
            }
          else if ( data!=SYN )
            {
            dex.rcvstate = DEX_DONE;
            dex.rcvstatus = DEX_STATUS_BAD;
            }
          }
        break;
      }
      lastchar = data;
		}

  else if (status&TC) /* Transmit ready */
		{
    SCCR1 = 0;                          /* 8 bit characters */
    SCDR  = sci.buffer [ sci.sndpnt++ ];
    if (sci.sndpnt==sci.sndcount)
			{
      SCCR2 &= ~TCIE;                   /* Turn xmt interrupts off */
      dex.sndstate = DEX_DONE;          /* Sending data is done    */
      dex.rcvstate = DEX_READY;         /* Make ready to rcv data  */
			}
		}
	}




/***********************************
* Rcv/Snd characters from SCI port.
************************************/
void sci_int()
	{
  BYTE data;

  if ( sci.devtype == TYPE_MDB )
    sci_mdb();
  else if ( sci.devtype == TYPE_DEX )
    sci_dex();
  else
    {
    data   = SCSR;      /* read status, assume char ready */
    data   = SCDR;      /* get char data. This will clear RDRF flag. */
    }
  }
