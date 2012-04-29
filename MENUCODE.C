/**********************
* M E N U C O D E . C *
***********************/
#include "hdrs.h"

#define FLASHTIME TM_800ms

int SetFlag(char *text, BYTE type, BYTE *faddr);

/****************************
* BCD to BINary conversion. *
*****************************/
WORD bcdbin(WORD bcd)
	{
	return ( ((bcd>>12)&15)*1000 + ((bcd>>8)&15)*100 +
		((bcd>>4)&15)*10 + (bcd&15) ) ;
	}


/****************************
* BINary to BCD conversion. *
*****************************/
WORD binbcd(WORD bin)
	{
  WORD bcdval=0, shft=0;

	for (; bin; shft+=4)
		{
		bcdval += (bin%10)<<shft;
		bin = bin/10;
		}

	return bcdval;
	}



/************************
* Menu function routine *
*************************/
int fn_mis_reset()
{
  if ( gPasswordValidated == TRUE )
  { 
    mis_clear();
    disp_printf("Clr");
    mcx_task_delay(MYSELF,0,TM_1sec);
  }

  return RTN_NORMAL;
}


/////////////////////
// Reset Errors    //
/////////////////////
int fnResetErrs(void)
{
  disp_printf("0000");
  mcx_task_delay(MYSELF,0,TM_1sec);

  gExitType[COUNTER_EXIT] = 0;
  gExitType[TIMER_EXIT]   = 0;
  gExitType[DIRTY_EXIT]   = 0;
  gExitType[NOPLS_EXIT]   = 0;

  return RTN_NORMAL;
}


/************************
* Menu function routine *
*************************/
int fn_exit()
  {
  return RTN_EXIT;
  }



/************************
* Menu function routine *
*************************/
int fn_return()
  {
  return RTN_BACK;
  }



char gNumStr[12];

/**********************************************
*  M E N U   F U N C T I O N   R O U T I N E  *
***********************************************/
int fn_menu(MENU *mbeg)
  {
  const static char *yesno[]={"NO","YES"};
  const static char *onoff[]={"OFF","On"};
  int  rtn_status, n, ln;
  BYTE key, i, j, k, *index=0, ix=0, iend;
  MENU *m, *mend;
  DWORD dw;
  char *pr;

  if (mbeg->mtype==M_INDEX)
    {
    index = mbeg->mpnt;
    mbeg++;
    }


  for (mend=mbeg, iend=0; *mend->text; mend++, iend++)
    ;


  keypadchar = 0;
  for (m=mbeg; ; )
    {
    /**************************************
    * Display item and wait for key input *
    ***************************************/
    keypadchar = 0;
    while (!keypadchar && gServiceTimeout<SERVICE_TIMEOUT)
      {
      disp_printf("%-4.4s", m->text);
      key_timeout(FLASHTIME);
      if (!keypadchar) switch (m->mtype)
        {
        case M_EDTFLAG:
          disp_printf("%4d", !!(*(BYTE *)m->mpnt));
          key_timeout(FLASHTIME);
          break;

        case M_EDTPRICE:
          disp_price(*(WORD *)m->mpnt);
          break;

        case M_EDTNUMB:
          disp_printf("%4d", *(WORD *)m->mpnt);
          break;

        case M_SHOWWORD:
          disp_printf("%4d", *(WORD *)m->mpnt);
          break;

        case M_EDTMISPRICE:
          if (gPasswordValidated == TRUE) 
          {
            dw = *(DWORD *)m->mpnt;
            pr = strdprice(dw);
            if (!keypadchar && dw>=10000)
            {
              disp_printf("%4.4s", pr);
              key_timeout(FLASHTIME);
            }
            disp_printf("%s", pr+4);
          }
          break;

        case M_EDTMISNUMBR:
          if (gPasswordValidated == TRUE) 
          {
            dw = *(DWORD *)m->mpnt;
            sprintf(gNumStr, "%8ld", dw);
            if (!keypadchar && dw>=10000)
            {
              disp_printf("%-4.4s", gNumStr);
              key_timeout(FLASHTIME);
            }
            disp_printf("%-4.4s", gNumStr+4);
          }
          break;

        case M_EDTTIME:
        case M_EDTCTIMER:
        case M_EDTDATE:
          disp_printf("%02X.%02X", *(WORD *)m->mpnt>>8, *(WORD *)m->mpnt&0xFF);
          break;

        case M_EDTTIMER:
          disp_printf("%02d.%02d", min(99,*(WORD *)m->mpnt/60), *(WORD *)m->mpnt%60);
          break;

        case M_EDTYESNO:
          disp_printf(yesno[!!(*(BYTE *)m->mpnt)]);
          break;

        case M_EDTONOFF:
          disp_printf(onoff[!!(*(BYTE *)m->mpnt)]);
          break;
        }
      if (!keypadchar)
        key_timeout(FLASHTIME);
      if (!keypadchar && m->mtype!=M_MENU)
        {
        disp_printf("");
        key_timeout(TM_300ms);
        }
      }
    key = keypadchar;

    /*******************
    * Act on key input *
    ********************/
    switch (key)
      {
      case 0:
      case KEY_MODE:
        return RTN_EXIT;

      case KEY_HOME:
        return RTN_NORMAL;

      case KEY_UP:
        ix++;
        m++;
        if (m->mtype==M_END)
          return RTN_NORMAL;
        else if (m->mtype==M_REPEAT)
          {
          ix = 0;
          m = mbeg;
          }
        keypadchar = 0;
        break;

      case KEY_DN:
        if (ix--)
          m--;
        else
          {
          ix = iend-1;
          m = mend-1;
          }
        break;


      case KEY_ENTER:
        /****************************
        * Execute function on ENTER *
        *****************************/
        switch (m->mtype)
          {
          case M_EDTTIME:
          case M_EDTCTIMER:
            rtn_status = SetTime(m->text, (WORD *)m->mpnt);
            break;

          case M_EDTTIMER:
            rtn_status = SetTimer(m->text, (WORD *)m->mpnt);
            break;

          case M_EDTPRICE:
            rtn_status = SetPrice(m->text, (WORD *)m->mpnt);
            break;

          case M_EDTNUMB:
            rtn_status = SetNumb(m->text, (WORD *)m->mpnt);
            break;

          case M_EDTONOFF:
          case M_EDTYESNO:
          case M_EDTFLAG:
            rtn_status = SetFlag(m->text, m->mtype, (BYTE *)m->mpnt);
            break;

          case M_EDTMISPRICE:
          case M_EDTMISNUMBR:
          case M_SHOWWORD:
            rtn_status = RTN_NORMAL;
            break;

          default:
            if (m->mpnt)
              rtn_status = fn_menu((MENU *)m->mpnt);
            else if (m->fn_exit)
              rtn_status = RTN_NORMAL;
            else
              rtn_status = RTN_EXIT;
            break;
          }

        /***********************
        * Execute exit routine *
        ************************/
        if (m->fn_exit && rtn_status==RTN_NORMAL)
          rtn_status = m->fn_exit(m->parm_exit);
        if (rtn_status==RTN_EXIT)
          return RTN_EXIT;
        else if (rtn_status==RTN_BACK)
          return RTN_NORMAL;
        keypadchar = 0;
        break;

      default:
        keypadchar = 0;
        break;
      }
    }
  }






/*******************
* Set time routine *
********************/
int SetTime(char *text, WORD *time)
  {
  BYTE key;
  WORD t, i, tm[2];
  const static WORD limit[2]={23,59};

  tm[0] = bcdbin(*time>>8);
  tm[1] = bcdbin(*time&0xFF);

  for (i=0; ; )
    {

    for (key=0; !key && gServiceTimeout<SERVICE_TIMEOUT; )
      {
      if (!key)
        {
        disp_printf("%02d.%02d", tm[0], tm[1]);
        key = key_timeout(FLASHTIME);
        }
      if (!key)
        {
        if (i==0)
          disp_printf("  .%02d", tm[1]);
        else
          disp_printf("%02d.  ", tm[0]);
        key = key_timeout(FLASHTIME);
        }
      }

    switch (key) {
      case 0:
      case KEY_MODE:
        return RTN_EXIT;

      case KEY_HOME:
        return RTN_NORMAL;

      case KEY_UP:
        if (++tm[i]>limit[i])
          tm[i] = 0;
        break;

      case KEY_DN:
        if (--tm[i]>limit[i])
          tm[i] = limit[i];
        break;

      case KEY_ENTER:
        if (++i==2)
          {
          t = binbcd(tm[0]*100+tm[1]);
          if (time < 0xB600  ||  time > 0xB800)
            *time = t;                  /* Write to RAM     */
          else
            eepcpy(time,&t,2);          /* Write to EEPROM  */
          return RTN_NORMAL;
          }
        break;
      }
    }
  }



/********************
* Set timer routine *
*********************/
int SetTimer(char *text, TIMER *timer)
  {
  BYTE key;
  WORD t, i, tm;

  tm = timer->value;

  for (i=0; ; )
    {

    for (key=0; !key && gServiceTimeout<SERVICE_TIMEOUT; )
      {
      if (!key)
        {
        disp_printf("%02d.%02d", tm/60, tm%60);
        key = key_timeout(FLASHTIME);
        }
      if (!key)
        {
        if (i==0)
          disp_printf("  .%02d", tm%60);
        else
          disp_printf("%02d.  ", tm/60);
        key = key_timeout(FLASHTIME);
        }
      }

    switch (key) {
      case 0:
      case KEY_MODE:
        return RTN_EXIT;

      case KEY_HOME:
        return RTN_NORMAL;

      case KEY_UP:
        tm += (i) ? 1 : 60;
        if (tm>timer->max || tm<timer->min)
          tm = timer->min;
        break;

      case KEY_DN:
        tm -= (i) ? 1 : 60;
        if (tm>timer->max || tm<timer->min)
          tm = timer->max;
        break;

      case KEY_ENTER:
        if (++i==2)
          {
          if (timer < 0xB600  ||  timer > 0xB800)
            timer->value = tm;            /* Write to RAM     */
          else
            eepcpy(&timer->value,&tm,2);  /* Write to EEPROM  */
          return RTN_NORMAL;
          }
        break;
      }
    }
  }



/********************
* Set price routine *
*********************/
int SetPrice(char *text, WORD *price)
  {
  BYTE key;
  WORD i, pr, scaling;

  /******************************
  * Determine scaling factor
  *******************************/
  if (coin.ok)
    scaling = coin.status.scaling;
  else if (bill.ok)
    scaling = bill.status.scaling;
  else
    scaling = 1;

  for (pr=*price, i=0; ; )
    {
    disp_price(pr);
    key = key_timeout(SERVICE_TIMEOUT);

    switch (key) {
      case 0:
      case KEY_MODE:
        return RTN_EXIT;

      case KEY_HOME:
        return RTN_NORMAL;

      case KEY_UP:
        pr += scaling;
        if (pr>=10000)
          pr = 0;
        break;

      case KEY_DN:
        pr -= scaling;
        if (pr>=10000)
          pr = 10000-scaling;
        break;

      case KEY_ENTER:
        if (price < 0xB600  ||  price > 0xB800)
          *price = pr;                  /* Write to RAM */
        else
          eepcpy(price,&pr,2);          /* Write to EEPROM  */
        return RTN_NORMAL;

      }
    }
  }







/*******************
* Set Flag routine *
********************/
int SetFlag(char *text, BYTE type, BYTE *faddr)
  {
  BYTE key, flag;
  int  n, ln;

  keypadchar = 0;
  for (flag=*faddr; ; )
    {
    if (keypadchar)
      key = keypadchar;
    else
      {
      if (type==M_EDTYESNO)
        disp_printf(flag?"YES":"NO");
      else if (type==M_EDTONOFF)
        disp_printf(flag?"On":"OFF");
      else
        disp_printf("%-3.3s%d", text, !!flag);
      key = key_timeout_beep(SERVICE_TIMEOUT);
      }

    switch (key)
      {
      case 0:
      case KEY_MODE:
        return RTN_EXIT;

      case KEY_HOME:
        return RTN_NORMAL;

      case KEY_UP:
      case KEY_DN:
        flag = !flag;
        keypadchar = 0;
        break;

      case KEY_ENTER:
        *faddr = flag;
        return RTN_NORMAL;

      default:
        keypadchar = 0;
        break;
      }
    }
  }



/************************
* Manual coin dispense
* from keypad.
*************************/
int fn_cpo()
  {
  BYTE key, i;
  BYTE map[16], num, err, c;
  WORD cr;

  coin_enable_service();

  for (i=0, num=0; i<16; i++)
    if (coin.status.credit && coin.status.routing&(1<<i))
      map[num++] = i;

  c = 0;
  keypadchar = 0;
  while ( coin.ok )
    {
    i = map[c];
    disp_price((WORD)coin.status.scaling*coin.status.credit[i]);

    if (keypadchar)
      key = keypadchar;                     /* new key already here */
    else
      key = key_timeout(SERVICE_TIMEOUT);   /* wait for new key     */
    keypadchar = 0;

    switch (key)
      {
      case 0:
      case KEY_MODE:
        coin_disable();
        return RTN_EXIT;

      case KEY_HOME:
        coin_disable();
        return RTN_NORMAL;

      case KEY_UP:
        c = ++c % num;
        break;

      case KEY_DN:
        c = (c+num-1) % num;
        break;

      case KEY_ENTER:
        i = map[c];
        rsrc_lock(&rsrc_SCI);               /* lock SCI resource    */
        err = !mdb_sendmsg(2,MDB_COIN_DISPENSE,(0x10|i),0,0, 0,0,0);
        rsrc_unlock(&rsrc_SCI);             /* unlock SCI resource    */
        if (!err)
          {
          cr = coin.status.credit[i];       /* calulate credit returned */
          if (coin.tube.count[i])           /* check for zero coins     */
            coin.tube.count[i]--;           /* reduce count of coins    */
          if (cr>coin.tube.total)
            coin.tube.total = 0;            /* zero credit in tubes     */
          else
            coin.tube.total -= cr;          /* reduce credit in tubes   */
          cr *= coin.status.scaling;        /* amount of credit paid    */
          mis.nr.cash_paid_manual += cr;    /* update amount paid out   */
          mis.rs.cash_paid_manual += cr;    /* update amount paid out   */
          mis.download.cash_paid_manual += cr;
          mis.nr.cash_paid_change += cr;    /* update amount paid out   */
          mis.rs.cash_paid_change += cr;    /* update amount paid out   */
          mis.download.cash_paid_change += cr;
          coin.pollstatus |= COIN_STATUS_PAYOUT_BUSY | COIN_STATUS_BUSY;
          while (coin.ok && (coin.pollstatus & (COIN_STATUS_PAYOUT_BUSY | COIN_STATUS_BUSY)))
              mcx_task_delay(MYSELF,0,TM_20ms);
          gCalcExact = TRUE;                /* compute for exact change */
          }
        break;
      }
    }
    return RTN_NORMAL;
  }



/*********************
* Set number routine *
**********************/
int SetNumb(char *text, NUMB *numb)
  {
  BYTE key;
  WORD n;

  /******************************
  * Determine scaling factor
  *******************************/
  for (n=numb->value; ; )
    {
    disp_printf("%4d", n);
    key = key_timeout(SERVICE_TIMEOUT);

    switch (key) {
      case 0:
      case KEY_MODE:
        return RTN_EXIT;

      case KEY_HOME:
        return RTN_NORMAL;

      case KEY_UP:
        if (++n>numb->max)
          n = numb->min;
        break;

      case KEY_DN:
        --n;
        if (n==0xFFFF || n<numb->min)
          n = numb->max;
        break;

      case KEY_ENTER:
        if (numb < 0xB600  ||  numb > 0xB800)
          numb->value = n;              /* Write to RAM */
        else
          eepcpy(&numb->value,&n,2);    /* Write to EEPROM  */
        return RTN_NORMAL;

      }
    }
  }




/************************
* Test vend
*************************/
int fn_vend(WORD v)
  {
  WORD g, tsk;

  if (v < 3)
    tsk = 0;
  else
    tsk = 1;

  g = v%3;

  gGalSelect[tsk] = g;
  gWaterCounter[tsk] = gGallon[tsk][g].value;
  keypadchar = 0;
  gWaterCount[tsk] = 0;
  while (!keypadchar && (gVendTaskActive[tsk] || gWaterCounter[tsk]))
  {
    disp_printf("%4d", gWaterCount[tsk]);
    mcx_task_delay(MYSELF,0,TM_50ms);
  }
  gWaterCounter[tsk] = 0;

  return RTN_NORMAL;
  }

