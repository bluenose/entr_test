/*********
* card.h *
**********/



/**************
* Card data
***************/
typedef struct {
  BYTE feature;
  BYTE disp_cols;
  BYTE disp_rows;
  WORD disp_info;
} MDBCARDVMC;
typedef struct {
  BYTE feature;
  WORD country;
  BYTE scaling;
  BYTE decpnt;
  BYTE maxtime;
  BYTE option;
} MDBCARDREADER;
typedef struct {
  WORD max;
  WORD min;
} MDBCARDPRICE;
typedef struct {
  BYTE state;
  WORD funds;
} MDBCARDSESSION;
typedef struct {
  BYTE state;
  WORD amount;
} MDBCARDVEND;
typedef struct {
  BYTE ok;                  /* Ready for polling              */
  BYTE reset;               /* Just reset status is pending   */
  BYTE config;              /* Reader Configure msg received  */
  BYTE state;               /* 0=Disable, 1=Enable, 2=Cancel  */
  MDBCARDVMC     vmc;       /* VMC configuration structure    */
  MDBCARDREADER  reader;    /* Reader configuration structure */
  MDBCARDPRICE   price;     /* Price selection structure  */
  MDBCARDSESSION session;   /* Session details structure  */
  MDBCARDVEND    vend;      /* Vend detail structure      */
  WORD pollstatus;          /* Status returned            */
  BYTE enabled;             /* Device enable flag         */
} MDBCARD;


#define MDB_CARD_RESET      0x10
#define MDB_CARD_SETUP      0x11
#define MDB_CARD_POLL       0x12
#define MDB_CARD_VEND       0x13
#define MDB_CARD_READER     0x14
#define MDB_CARD_EXPANSION  0x17

#define CARD_RESET          0x00
#define CARD_CONFIG         0x01
#define CARD_DISPLAY        0x02
#define CARD_SESSION_BEGIN  0x03
#define CARD_SESSION_CANCEL 0x04
#define CARD_VEND_APPROVED  0x05
#define CARD_VEND_DENIED    0x06
#define CARD_SESSION_END    0x07
#define CARD_CANCELLED      0x08
#define CARD_PERIPHERAL_ID  0x09
#define CARD_ERROR          0x0A
#define CARD_CMD_BAD_SEQ    0x0B
#define CARD_DIAGNOSTIC     0xFF

#define CARD_VEND_REQUEST      0
#define CARD_VEND_CANCEL       1
#define CARD_VEND_SUCCESS      2
#define CARD_VEND_FAILURE      3
#define CARD_SESSION_COMPLETE  4

#define CARD_DISABLE 0
#define CARD_ENABLE  1
#define CARD_CANCEL  2

#define CARD_FLAG_CONFIG 0x01
#define CARD_FLAG_VEND   0x02


/*************
* Prototypes *
**************/
void card_process( void );
int card_vend_begin(WORD pri, WORD num);
int card_vend_successful(WORD num);
int card_complete_session();
