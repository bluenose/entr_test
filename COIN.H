/*********
* coin.h *
**********/


/**************
* Coin data
***************/
typedef struct {
  BYTE feature;     /* feature level - extended >= 3  */
  WORD country;     /* country code - USA = 0001      */
  BYTE scaling;     /* coin scaling factor            */
  BYTE decpnt;      /* decimal place - USA = 02       */
  WORD routing;     /* coins that have tubes          */
  BYTE credit[16];  /* coin value in scaling factors  */
} MDBCOINSTATUS;
typedef struct {
  WORD full;        /* from tube status               */
  BYTE report[16];  /* from tube status               */
  BYTE count[16];   /* updated internally             */
  WORD total;       /* total credits in tubes         */
  BYTE num;         /* number of coins to be mapped   */
  WORD token;       /* token routing                  */
  BYTE map[16];     /* coin map reference             */
} MDBCOINTUBE;
typedef struct {
  BYTE ok;                /* is in working order  */
  BYTE reset;             /* just reset, or not   */
  MDBCOINSTATUS status;   /* coin status          */
  long expansion;         /* expansion stuff      */
  MDBCOINTUBE tube;       /* tube status          */
  WORD pollstatus;        /* poll status          */
  BYTE enabled;           /* enabled flag         */
  BYTE tokenused;         /* token is being used  */
  BYTE release;           /* escrow lever pressed */
} MDBCOIN;



#define MDB_COIN_RESET      0x08
#define MDB_COIN_STATUS     0x09
#define MDB_COIN_TUBESTATUS 0x0A
#define MDB_COIN_POLL       0x0B
#define MDB_COIN_COINTYPE   0x0C
#define MDB_COIN_DISPENSE   0x0D
#define MDB_COIN_EXPANSION  0x0F

#define COIN_STATUS_RELEASE       0x0002
#define COIN_STATUS_PAYOUT_BUSY   0x0004
#define COIN_STATUS_NO_CREDIT     0x0008
#define COIN_STATUS_TUBE_DEFECT   0x0010
#define COIN_STATUS_DOUBLE_ARVL   0x0020
#define COIN_STATUS_ACCEPT_UNPLUG 0x0040
#define COIN_STATUS_TUBE_JAM      0x0080
#define COIN_STATUS_ROM_CHKSUM    0x0100
#define COIN_STATUS_ROUTING_ERR   0x0200
#define COIN_STATUS_BUSY          0x0400
#define COIN_STATUS_RESET         0x0800
#define COIN_STATUS_JAM           0x1000


/*************
* Prototypes *
**************/
void coin_init();
void coin_poll_init();
void coin_poll();
void coin_enable_service();
void coin_enable();
void coin_disable();
BYTE coin_change(WORD credit, BYTE *coincount);
WORD coin_release(WORD credit);
BYTE coin_exact();
