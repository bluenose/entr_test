/***************
* MIS  routines
****************/
#include "hdrs.h"

#define FLASHTIME TM_1sec


/*************
* Prototypes
**************/
BYTE mis_disp_num(long);
BYTE mis_disp_price(long);


/*****************
* Display number *
******************/
BYTE mis_disp_num(long num)
  {
  int high, low;
  BYTE key;

  high = (int)(num/10000);
  low  = (int)(num%10000);
  if (high)
    {
    disp_printf("%4d", high);
    if (key = key_timeout(FLASHTIME))
      return key;
    disp_printf("%04d", low);
    return key_timeout(FLASHTIME);
    }
  else
    disp_printf("%4d", low);

  return key_timeout(FLASHTIME);
  }


/****************
* Display price *
*****************/
BYTE mis_disp_price(long num)
  {
  int high, low;
  BYTE key;

  high = (int)(num/10000);
  low  = (int)(num%10000);
  if (high)
    {
    disp_printf("%4d", high);
    if (key = key_timeout(FLASHTIME))
      return key;
    gZeroFill = TRUE;
    disp_price(low);
    gZeroFill = FALSE;
    return key_timeout(FLASHTIME);
    }
  else
    disp_price(low);

  return key_timeout(FLASHTIME);
  }



/***********************
* total MIS accounting *
************************/
void mis_tot()
  {
  long num;
  BYTE key;

  while (FOREVER)
    {
    disp_printf("tot");                      /* logo */
    if (!(key = key_timeout_beep(SERVICE_TIMEOUT)))
      return;                                /* timeout */


    /***************
    * Display item *
    ****************/
    switch (key)
      {

      case 1:
        num = mis.nr.cash;
        mis_disp_price(num);
        break;

      case 2:
        mis_disp_num((long)mis.nr.vends);
        break;

      case 3:
        num = mis.rs.cash;
        mis_disp_price(num);
        break;

      case 4:
        mis_disp_num((long)mis.rs.vends);
        break;


      /************
      * Exit keys *
      *************/
      case KEY_MODE:
        return;


      /*****************
      * Error response *
      ******************/
      default:
        disp_printf("Err");
        key_timeout(TM_1sec);
        break;
      }
    }
  }





/***************************
* selection MIS accounting *
****************************/
void mis_sel()
  {
  }


/****************************
* High level MIS accounting *
*****************************/
void mis_disp_acct()
  {
  }




/***********************
* Clear MIS resetables *
************************/
void mis_clear()
{
  BYTE i;

  for (i=0; i<6; i++)
  {
    copduty();
    mis.rs.sel[i].cash = 0;            /* total */
    mis.rs.sel[i].vends = 0;           /* total */
    mis.rs.sel[i].so = 0;
  }
  mis.rs.cash = 0;
  mis.rs.vends = 0;
  mis.rs.cash2box = 0;
  mis.rs.cash2tubes = 0;
  mis.rs.bill_cash = 0;
  mis.rs.card_cash = 0;
  mis.rs.card_vends = 0;
  mis.rs.cash_paid_change = 0;
  mis.rs.cash_paid_manual = 0;
  mis.rs.token_vends = 0;
  mis.rs.token_value = 0;
}

/***********************
* Clear DEX MIS resetables *
************************/
void dex_mis_clear()
{
  BYTE i;

  for (i=0; i<6; i++) /* selmax = 4 */
  {
    copduty();
    mis.download.sel[i].vends = 0;    /* for each side */
    mis.download.sel[i].cash  = 0;     /* for each side */
    mis.download.sel[i].so    = 0;
  }
  mis.download.cash = 0;
  mis.download.vends = 0;
  mis.download.cash2box = 0;
  mis.download.cash2tubes = 0;
  mis.download.bill_cash = 0;
  mis.download.card_cash = 0;
  mis.download.card_vends = 0;
  mis.download.cash_paid_change = 0;
  mis.download.cash_paid_manual = 0;
  mis.download.token_vends = 0;
  mis.download.token_value = 0;
}



/******************************
* Clear MIS resetable request *
*******************************/
void mis_clear_request()
  {
  BYTE key;

  disp_printf("Res");                       /* logo */
  key = key_timeout_beep(SERVICE_TIMEOUT);

  if (key!=KEY_CLEAR)
    return;

  disp_printf("Clr");                       /* Clear */
  key = key_timeout_beep(TM_5sec);
  if (key==KEY_ENTER)
    {                                       /* yes - do it */
    disp_printf("YES");
    mis_clear();                            /* clear MIS resetables */
    mcx_task_delay(MYSELF,0,TM_1sec);
    }
  else
    {
    disp_printf("no");                      /* no - don't do it */
    mcx_task_delay(MYSELF,0,TM_1sec);
    }
  }
