/******************
* M I S   d a t a
*******************/

typedef struct {
  long cash;
  long vends;
  long so;
} SELMIS;

typedef struct {
  long cash;
  long vends;
  long cash2box;
  long cash2tubes;
  long bill_cash;
  long card_cash;
  long card_vends;
  long cash_paid_change;
  long cash_paid_manual;
  long token_vends;
  long token_value;
  SELMIS  sel[6];    /* selection MIS          */
} MISDATA;

typedef struct {
  MISDATA nr;             /* general Non-Resetables */
  MISDATA rs;             /* general Resetables     */
  MISDATA download;       /* test_code */
  long dexreads;          /* number of DEX reads    */
  long dooropens;         /* number of door opens   */
} MIS;
