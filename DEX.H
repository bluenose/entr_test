/********
* dex.h *
*********/

#ifndef _DEX_
#define _DEX_

typedef struct {
  char seq;         /* record block sequence - '0' or '1' */
  BYTE rcvstate;    /* receive state */
  BYTE rcvstatus;   /* receive status - OK / BAD */
  BYTE sndstate;    /* sending state */
  BYTE do_crc;      /* compute crc16 in dex_sndrec routine  */
  BYTE add_crlf;    /* add <cr> <lf> in dex_sndrec routine  */
  BYTE start;       /* SOH or STX */
  BYTE end;         /* ETB or ETX */
  WORD setcount;    /* Number of transaction sets xmitted   */
  WORD block_crc;   /* crc16 value computed for each block  */
  WORD record_crc;  /* crc16 value computed for all records */
} DEX;

#define DEX_READY  0
#define DEX_BUSY   1
#define DEX_CRC16A 2
#define DEX_CRC16B 3
#define DEX_DONE   4


#define DEX_STATUS_OK  0
#define DEX_STATUS_BAD 1


#define VCS_ID "9264230000"
#define DEX_SEND     0
#define DEX_RECEIVE  1



/*************
* Prototypes *
**************/
void dex_xfer ( void );
void dex_connect ( void );
void dex_disconnect ( void );
int  dex_session_begin ( void );
int  dex_session_end ( void );



#endif   /** _DEX_ **/
