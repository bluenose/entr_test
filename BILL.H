/*********
* bill.h *
**********/


/**************
* Bill data
***************/
typedef struct {
  BYTE feature;
  WORD country;
  WORD scaling;
  BYTE decpnt;
  WORD capacity;
  WORD security;
  BYTE escrow;
  BYTE credit[16];
} MDBBILLSTATUS;
typedef struct {
  BYTE full;
  BYTE type;
} MDBBILLESCROW;
typedef struct {
  BYTE ok;
  BYTE reset;
  MDBBILLSTATUS status;
  WORD pollstatus;
  BYTE enabled;
  MDBBILLESCROW escrow;
  BYTE newcointotal;
  BYTE tokenused;         /* token is being used  */
} MDBBILL;


#define MDB_BILL_RESET      0x30
#define MDB_BILL_STATUS     0x31
#define MDB_BILL_SECURITY   0x32
#define MDB_BILL_POLL       0x33
#define MDB_BILL_BILLTYPE   0x34
#define MDB_BILL_ESCROW     0x35
#define MDB_BILL_STACKER    0x36
#define MDB_BILL_EXPANSION  0x37


/*************
* Prototypes *
**************/
