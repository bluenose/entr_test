/********
* sci.h *
*********/

#ifndef _SCI_
#define _SCI_

#define SCIBUFSIZE 60

#define SOH 0x01
#define STX 0x02
#define ETX 0x03
#define EOT 0x04
#define ENQ 0x05
#define ACK 0x06
#define LF  0x0A
#define CR  0x0D
#define DLE 0x10
#define NAK 0x15
#define SYN 0x16
#define ETB 0x17

/*********************************************
* Structure for defining the sci structure
**********************************************/
typedef struct {
  BYTE devtype;             /* 0=mdb, 1=dex, 2=other       */
  BYTE state;               /* message state (send, receive, done) */
  BYTE sndpnt;              /* pointer to byte being sent  */
  BYTE sndcount;            /* count of bytes to send      */
  BYTE rcvcount;            /* count of bytes received     */
  BYTE rcvmax;              /* maximum bytes to receive    */
  BYTE msgstatus;           /* msg return status (ACK/NAK) */
  BYTE buffer[SCIBUFSIZE];  /* pointer to snd/rcv buf      */
} SCI_BLOCK;

#define TYPE_MDB 0
#define TYPE_DEX 1

#define STATE_send    1
#define STATE_receive 2
#define STATE_done    3

#define STATUS_ACK     1
#define STATUS_NAK     2

/* Baud rates */
#define B9600 0x30
#define B1200 0x33

/* SCSR */
#define TDRE   0x80
#define TC     0x40
#define RDRF   0x20
#define OR     0x08

/* SCCR1 */
#define R8     0x80
#define T8     0x40
#define M      0x10
#define WAKE   0x08

/* SCCR2 */
#define TIE  0x80
#define TCIE 0x40
#define RIE  0x20
#define TE   0x08
#define RE   0x04


#endif   /* _SCI_ */
