/************************************
* util.c - general utility routines
*************************************/
#include "hdrs.h"


/***********************************
* Initialize debounce structure
************************************/
void debounce_init(DEBOUNCE *deb, BYTE val, BYTE max)
  {
  deb->data     = val;    /* debounced data   */
  deb->ok       = FALSE;  /* debounce ok flag */
  deb->newval   = val;    /* new data value   */
  deb->oldval   = val;    /* old data value   */
  deb->count    = max;    /* debounce count   */
  deb->max      = max;    /* maximum count    */
  }


/******************************************
* Debounce value
*******************************************/
BYTE debounce(DEBOUNCE *deb, BYTE newval)
{
  deb->oldval = deb->newval;
  deb->newval = newval;

  if (deb->newval!=deb->oldval)
    deb->count = deb->max;
  else if (deb->count)
    if (!(--deb->count))
      deb->data = deb->newval;

  deb->ok = !deb->count;

  return deb->data;
}



/***********************************
* Initialize debounce2 structure
************************************/
void debounce2_init(DEBOUNCE2 *deb, WORD val, BYTE max)
{
  deb->data     = val;    /* debounced data   */
  deb->ok       = FALSE;  /* debounce ok flag */
  deb->newval   = val;    /* new data value   */
  deb->oldval   = val;    /* old data value   */
  deb->count    = max;    /* debounce count   */
  deb->max      = max;    /* maximum count    */
}


/******************************************
* Debounce2 value
*******************************************/
WORD debounce2(DEBOUNCE2 *deb, WORD newval)
{
  deb->oldval = deb->newval;
  deb->newval = newval;

  if (deb->newval!=deb->oldval)
    deb->count = deb->max;
  else if (deb->count)
    if (!(--deb->count))
      deb->data = deb->newval;

  deb->ok = !deb->count;

  return deb->data;
}



/******************************
 *  Display a button number
 ******************************/
void disp_btn(WORD num)
{
  disp_printf("-%02d-", num);
}



/**********************************
* switch SCI hardware to MDB mode
***********************************/
void sci_mdb_mode()
{
  PORTA &= 0xF7;
}


/**********************************
* switch SCI hardware to DEX mode
***********************************/
void sci_dex_mode()
{
  PORTA |= 0x08;
}


/***************************************
* Scroll a message accross the display *
****************************************/
void disp_scroll(char *txt)
{
  for ( ;*txt; txt++)
  {
    disp_printf("%-4.4s", txt);
    mcx_task_delay(MYSELF,0,TM_200ms);
  }
}





/*****************************************
* Format a double word to a price string *
******************************************/
char *strdprice(DWORD num)
{
  static char s[10];
  BYTE dp;

  if (num>99999999)
    sprintf(s, "--------");                /* Out of Range */
	else
  {
    /**********************
    * Place decimal point
    ***********************/
    if (coin.ok)
      dp = coin.status.decpnt;
    else if (bill.ok)
      dp = bill.status.decpnt;
    else
      dp = 0;

    switch (dp)
    {
      case 0:
        sprintf(s, "%8ld.", num);
        break;

      case 1:
        if (num/10)
          sprintf(s, "%7ld.%ld", num/10, num%10);
        else
          sprintf(s, "       .%ld", num%10);
        break;

      case 2:
        if (num/100)
          sprintf(s, "%6ld.%02ld", num/100, num%100);
        else
          sprintf(s, "      .%02ld", num%100);
        break;

      case 3:
        if (num/1000)
          sprintf(s, "%5ld.%03ld", num/1000, num%1000);
        else
          sprintf(s, "     .%03ld", num%1000);
        break;

      case 4:
        if (num/10000)
          sprintf(s, "%4ld.%04ld", num/10000, num%10000);
        else
          sprintf(s, "    .%04ld", num%10000);
        break;

      default:
        sprintf(s, "8ld", num);
        break;
    }
  }
  return s;
}


/***********************
* Edit 4 digits of BCD *
************************/
WORD snchange(WORD bcd)
{
  BYTE key;
  WORD i, digit, newbcd;
  char msg[5];

  newbcd = bcd;
  for (i=0; ; )
  {

    while (FOREVER)
    {
      disp_printf("%04X", newbcd);
      if (key = key_timeout(TM_250ms))
        break;
      sprintf(msg, "%04X", newbcd);
      msg[i] = ' ';
      disp_printf(msg);
      if (key = key_timeout(TM_250ms))
        break;
    }

    if (key==KEY_UP)
    {
      digit = (newbcd>>(4*(3-i))) & 0xF;
      digit = ++digit % 10;
      newbcd &= ~(0xF<<(4*(3-i)));
      newbcd |= digit<<(4*(3-i));
    }

    else if (key==KEY_DN)
    {
      digit = (newbcd>>(4*(3-i))) & 0xF;
      digit = (digit+9) % 10;
      newbcd &= ~(0xF<<(4*(3-i)));
      newbcd |= digit<<(4*(3-i));
    }

    else if (key==KEY_ENTER)
    {
      if (++i==4)
        return newbcd;
    }
  }
}




/************************************
* S E T   S E R I A L   N U M B E R *
*************************************/
void SetSN()
{
  BYTE  key;
  WORD  pr, *setcode;
  WORD *sn;

  setcode = (WORD *)SN_CODE;
  sn      = (WORD *)SN_DATA;
  disp_printf("Sn");
  key = key_timeout(SERVICE_TIMEOUT);
  if (key==0)
    return;
  else if (key!=KEY_MODE)
  {
    sn[0] = snchange(sn[0]);
    sn[1] = snchange(sn[1]);
    *setcode = SN_SET;
    disp_printf("%4X", sn[0]);
    mcx_task_delay(MYSELF,0,TM_1sec);
    disp_printf("%04X", sn[1]);
    mcx_task_delay(MYSELF,0,TM_2sec);
    disp_printf("");
    mcx_task_delay(MYSELF,0,TM_100ms);
    return ;
  }
}

///////////////////////////////
// Check for proper password //
/////////////////////////////// 
BYTE GetPassword()
{
  BYTE key;
  WORD passwordAttempt = 0xFFFF;
  WORD *sn;
  WORD nibbles[4];
  WORD password = 0;

  // already validated -- return
  if (gPasswordValidated == TRUE) 
    return TRUE;

  sn = (WORD *)SN_DATA;
  
  disp_printf("PSD");
  key = key_timeout(SERVICE_TIMEOUT);

  switch (key)
  { 
    case 0:
    case KEY_HOME:
      return FALSE;
      //
    default:
      // get password attempt
      passwordAttempt = snchange(0);

      // get the actual password
      nibbles[0] = sn[0] & 0xF000; 
      nibbles[1] = sn[0] & 0x000F;
      nibbles[2] = sn[1] & 0x0F00;
      nibbles[3] = sn[1] & 0x00F0;
      password = nibbles[0] | nibbles[1] | nibbles[2] | nibbles[3];

      //
      //disp_printf("old1");
      //mcx_task_delay(MYSELF,0,TM_500ms);
      //disp_printf("%04X", sn[0]);
      //mcx_task_delay(MYSELF,0,TM_1sec);
    
      //disp_printf("old2");
      //mcx_task_delay(MYSELF,0,TM_500ms);
      //disp_printf("%04X", sn[1]);
      //mcx_task_delay(MYSELF,0,TM_1sec);

      //disp_printf("nu");
      //mcx_task_delay(MYSELF,0,TM_500ms);
      //disp_printf("%04X", password);
      //mcx_task_delay(MYSELF,0,TM_1sec);

      //disp_printf("Attp");
      //mcx_task_delay(MYSELF,0,TM_500ms);
      //disp_printf("%04X", passwordAttempt);
      //mcx_task_delay(MYSELF,0,TM_1sec);

      // validate
      gPasswordValidated = (password == passwordAttempt) ? TRUE : FALSE;
  }

  if (gPasswordValidated == TRUE)
  {
    disp_printf("GOOD");
    mcx_task_delay(MYSELF,0,TM_500ms);
    disp_printf("PSD");
    mcx_task_delay(MYSELF,0,TM_500ms);
  }
  else
  {
    disp_printf("BAD");
    mcx_task_delay(MYSELF,0,TM_500ms);
    disp_printf("PSD");
    mcx_task_delay(MYSELF,0,TM_500ms);
  }

  return gPasswordValidated;
}
