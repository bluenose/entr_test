/********
* mdb.h *
*********/

#ifndef _MDB_
#define _MDB_

/************************************
* Flags below determines which MDB  *
* devices are used.                 *
************************************/
#define _COIN_  TRUE
#define _BILL_  TRUE
#define _CARD_  TRUE


#define MDB_ACK 0x00
#define MDB_RET 0xAA
#define MDB_NAK 0xFF

#define MDB_COIN     0x01
#define MDB_CARD     0x02
#define MDB_BILL     0x06
#define MDB_MENUMART 0x1E



/*************
* Prototypes *
**************/
BYTE *movdata(void *to, void *from, int count);
BYTE mdb_sendmsg(BYTE resptm, BYTE cmd, int subcmd, BYTE sndlen, void *sndbuf,
        BYTE dataoff, BYTE datalen, void *databuf);

#endif   /** _MDB_ **/
