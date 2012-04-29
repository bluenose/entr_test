/****************
* Vending tasks *
*****************/
#include "hdrs.h"

#define ThreeMinutes 180

/***********************************************
****      V E N D    P R O D U C T          ****
************************************************/
void vendproduct(WORD v, BYTE side)
{
  WORD status;
  BYTE couponvendval;

  status = do_vend(v, side);                       // do the vend; return the status

  if (status == VEND_OK)
  {
    gCredit -= min(gCredit, gPrice[v]);            // subtract the price to only allow acceptable vend
    disp_price(gCredit);                           // display the new credit value
  }
  else                                             // un-successful vend
  {
    if (!bill.tokenused)
      disp_price(gCredit);                         // display the new credit value

    gFirstTry = TRUE;
    flash_soldout();                               // Unsuccessful-- flash soldout
  }
}

/*********************************
* Call this routine to do a vend *
**********************************/
int do_vend(WORD v, BYTE side)
{
  const static BYTE extendsw[2]={SW_EXTEND1,SW_EXTEND2};
  WORD tsk, g;

  tsk = side;
  gReturnChange = FALSE;

  if (gVendTaskActive[tsk])                          // if the current task is active ...
    return VEND_BUSY;                                //   return busy
  else if (gVendFailTimer[tsk])                      // if the current task has timed out...
    return VEND_SO;                                  //   return SOLD OUT
  else if (gSwitch[extendsw[tsk]])                   // if nozzle already extended 
  {
    gVendFailTimer[tsk] = ThreeMinutes;
    return VEND_SO;
  }

  gGalSelect[tsk] = v;
  gWaterCounter[tsk] = gGallon[tsk][v].value;

  return VEND_OK;
}


/****************************
*  D O   V E N D   T A S K  *
*****************************/
void vendtask(WORD tsk)
{
  const static BYTE extendsw[2]={SW_EXTEND1,SW_EXTEND2};
  const static WORD extend[2]={EXTEND1,EXTEND2};
  const static WORD valve[2]={VALVE1,VALVE2};
  const static WORD meterpwr[2]={FMETERPWR1, FMETERPWR2};
  const static BYTE intenb[2]={IC2I,IC3I};

  BYTE g, i, statusOK, enbit, x, dirty[2], tokensused, extended[2];
  WORD gallons, credit, maxtime, tokenCredit;
  MCX_MAILBOX mbox_outp;
  DEVMSG_OUTP outp;

  mbox_outp.message = &outp;
  outp.devtype = DEV_OUTP;
  outp.ctrl = MASK;
  outp.mask = extend[tsk] | valve[tsk] | meterpwr[tsk];
  enbit = intenb[tsk];
  gWaterCount[tsk] = 0;                // determines the vend SO status; be sure to clear

  while (FOREVER)
  {
    while (gWaterCounter[tsk] == 0)
    {
      mcx_task_delay(MYSELF,0,TM_50ms);
      gVendTaskActive[tsk] = FALSE;
    
	  // Turn off all output drives when they should not be on.
	  // This is done in attempt to make the machine 'noise proof'.
      outp.select = 0;
      mcx_mailbox_send(TASK_dev,0,&mbox_outp);
    }

    gVendTaskActive[tsk] = TRUE;
    if (gGalSelect[tsk] == GAL1)
    {
      gallons = 1;
      credit = gPrice[GAL1];
    }
    else if (gGalSelect[tsk] == GAL3)
    {
      gallons = 3;
      credit = gPrice[GAL3];
    }
    else if (gGalSelect[tsk] == GAL5)
    {
      gallons = 5;
      credit = gPrice[GAL5];
    }
    maxtime = gallons * gPourTimerMax.value;   // solenoid timeout
    i = (BYTE)(tsk*3) + gGalSelect[tsk];       // i = the selection

    /*** Check retract state ***/
    //statusOK = gSwitch[extendsw[tsk]];
    statusOK = TRUE;

    /*** Extend ***/
    if (statusOK)
    {
      statusOK = FALSE;
      outp.select = extend[tsk];
      mcx_mailbox_send(TASK_dev,0,&mbox_outp);

      for (gVendTimer[tsk]=0; gVendTimer[tsk]<TM_10sec;)
      {
        // if extended then continue on ...
        if (gSwitch[extendsw[tsk]])
        {
          statusOK = TRUE;
          break;
        }
        
        mcx_task_delay(MYSELF,0,TM_200ms);
      }
    }

    /*** Pour the water ***/
    if (statusOK)
    {
      outp.select = extend[tsk] | valve[tsk] | meterpwr[tsk];
      mcx_mailbox_send(TASK_dev,0,&mbox_outp);
      gMeterPulse[tsk] = 0;
      TMSK1 |= enbit;                                  // Enable counter interrupts
      gNoPulseTimer[tsk]=dirty[tsk]=gPourTimer[tsk]=0;      // initialize loop
      for ( ; ((gWaterCounter[tsk]>0) && (gPourTimer[tsk] < maxtime) && (dirty[tsk] < VEND_FAIL_DIRTY) && (gNoPulseTimer[tsk] < VEND_FAIL_NOPULSES)); )
      {
        if (WaterDirty())
          dirty[tsk]++;
        else
          dirty[tsk] = 0;
        mcx_task_delay(MYSELF,0,TM_200ms);
        if (gMeterPulse[tsk])
        {
          gWaterCount[tsk]   += gMeterPulse[tsk];
          gWaterCounter[tsk] -= gMeterPulse[tsk];
          gMeterPulse[tsk]    = 0;
          gNoPulseTimer[tsk]  = 0;      // change in pulse; reset timer
        }
      }

      #if DEBUG_TEST
        if (gWaterCounter[tsk] <= 0)
          gExitType[COUNTER_EXIT]++;
        else if (gPourTimer[tsk] >= maxtime)
          gExitType[TIMER_EXIT]++;
        else if (dirty[tsk] >= VEND_FAIL_DIRTY)
          gExitType[DIRTY_EXIT]++;
        else if (gNoPulseTimer[tsk] >= VEND_FAIL_NOPULSES)
          gExitType[NOPLS_EXIT]++;
      #endif

      TMSK1 &= ~enbit;                      /* Disable counter interrupts */
      outp.select = extend[tsk];            /* the shake */
      mcx_mailbox_send(TASK_dev,0,&mbox_outp);
      mcx_task_delay(MYSELF,0,(TM_500ms*gShakeTimeInHalfSecIntervals.value));
    }

    // x signifies which gallon value to point to
    if (gallons == 1)
      x = 0;
    else if (gallons == 3)
      x = 1;
    else if (gallons == 5)
      x = 2;

    ///////////////////////////
    // Test for any failures //
    ///////////////////////////
    if (dirty[tsk] >= VEND_FAIL_DIRTY)
	{
      statusOK = FALSE;
	}
	// if there is over 1/4 (25%) left then give them their change back
	// *** this is a fix put in place for a cheat where the customer stops the vend counter (plugs nozzle) at the end of the vend ***
    else if (((gNoPulseTimer[tsk] >= VEND_FAIL_NOPULSES) || (gPourTimer[tsk] >= maxtime)) && 
		(gWaterCounter[tsk] > (gGallon[tsk][gGalSelect[tsk]].value - (gGallon[tsk][gGalSelect[tsk]].value / 4))))
	{
      statusOK = FALSE;
	}

    ///////////////////////////
    // Turn everything Off   //
    ///////////////////////////
    outp.select = 0;
    mcx_mailbox_send(TASK_dev,0,&mbox_outp);

    /************
    * VEND FAIL *
    ************/
    if (!statusOK)
    {
      gVendFailTimer[tsk] = ThreeMinutes;
      if (!gServiceMode)
        gCredit += credit;                  /* Return credit      */
    }

    /***************
    * VEND SUCCESS *
    ***************/
    else
    {
      tokenCredit = tokensused = 0;       // clear out old token and token value
      if (!gServiceMode)                  // safeguard; not in service mode
      {
        if (gCoinToken)
        {
          if (gallons == 1)
            tokensused = 1;
          if (gallons == 3)
            if (gCoinToken <= 3)
              tokensused = gCoinToken;
            else
              tokensused = 3;
          if (gallons == 5)
            tokensused = gCoinToken;

          tokenCredit = 20 * tokensused;  // token credit val = # of tokens * 20
        }

        //credit -= tokenCredit;          // rest of value = non-token val

        if (gCoinToken)
        {
          mis.rs.token_value += tokenCredit;
          mis.nr.token_value += tokenCredit;
          mis.download.token_value += tokenCredit;
          mis.rs.token_vends += tokensused;
          mis.nr.token_vends += tokensused;
          mis.download.token_vends += tokensused;
          if ((gCoinToken - tokensused) >= 0)
            gCoinToken -= tokensused;

        }

        mis.nr.cash += credit;
        mis.rs.cash += credit;
        mis.download.cash += credit;
        mis.nr.vends += gallons;
        mis.rs.vends += gallons;
        mis.download.vends += gallons;
        mis.nr.sel[i].vends++;
        mis.rs.sel[i].vends++;
        mis.download.sel[i].vends++;
        mis.nr.sel[i].cash += credit;
        mis.rs.sel[i].cash += credit;
        mis.download.sel[i].cash += credit;

        gReturnChange = TRUE;
        gReturnTime = FALSE;
      }

      gGallonCount  +=  gallons;
      gROgalcnt     +=  gallons;

      if (gMaster)
        gGalSolCount[MASTER] += gallons;  /* Counter for Solenoid           */
      else for (g=0; g<gallons; g++)
      {
        PORTD ^= 0x20;                    /* Report gallon count to master  */
        mcx_task_delay(MYSELF,0,TM_200ms);
        PORTD ^= 0x20;
        mcx_task_delay(MYSELF,0,TM_200ms);
      }
    }

    /*************************************
    * Pause before allowing another vend *
    **************************************/
    mcx_task_delay(MYSELF,0,TM_200ms);
    gWaterCount[tsk] = 0;
    gWaterCounter[tsk] = 0;                /* Zero water counter */
  }
}



void task_vend1()   { vendtask(0);  }   /* VEND TASK 1 */
void task_vend2()   { vendtask(1);  }   /* VEND TASK 2 */




/********************************
* Back flush and RO maintenance *
*********************************/
void task_maintenance()
{
  MCX_MAILBOX mbox_outp;
  DEVMSG_OUTP outp;
  BYTE i, flush;
  const static WORD metercnt[2]={FMETERCNT1, FMETERCNT2};
  DEBOUNCE debsw;

  mbox_outp.message = &outp;
  outp.devtype = DEV_OUTP;
  outp.ctrl = MASK;
  outp.mask = RO_ON | RO_FLUSH | metercnt[0] | metercnt[1];
  while (FOREVER)
  {
    while ((gROgalmax.value==0 || gROgalcnt<gROgalmax.value) && gROstate==OFF)
    {
      if (gGalSolCount[0]==0 && gGalSolCount[1]==0)
        mcx_task_delay(MYSELF,0,TM_200ms);
      else
        for (i=0; i<2; i++)
          for (; gGalSolCount[i]; gGalSolCount[i]--)
          {
            /***************************
            * Toggle solenoid counters *
            ****************************/
            outp.select = metercnt[i];
            mcx_mailbox_send(TASK_dev,0,&mbox_outp);
            mcx_task_delay(MYSELF,0,TM_50ms);
            outp.select = 0;
            mcx_mailbox_send(TASK_dev,0,&mbox_outp);
            mcx_task_delay(MYSELF,0,TM_100ms);
          }
    }

    /*************************
    * Get the RO pressure up *
    **************************/
    gROstate    = ON;
    outp.select = RO_ON;
    gROpostTimer = 0;
    debounce_init(&debsw, 1, 25);       /* Debounce 25 times or 5 seconds */
    while (gROstate==ON && gSwitch[SW_RO_PRESSURE]==OFF)
    {
      // was ... if (gROpostTimer>120...
      if (gROpostTimer>20 && debounce(&debsw,gSwitch[SW_POST_TREATMENT])==0)
        break;
      if (debounce(&debsw,gSwitch[SW_POST_TREATMENT])==ON)
        gROpostTimer = 0;
      if (gBackFlushInt.value)
      {
        if (gROTimer >= gBackFlushInt.value+gBackFlushTime.value)
          gROTimer = 0;
        outp.select = RO_ON | (gROTimer >= gBackFlushInt.value ? RO_FLUSH : 0);
        mcx_mailbox_send(TASK_dev,0,&mbox_outp);
        mcx_task_delay(MYSELF,0,TM_200ms);
      }
      gROgalcnt = 0;
    }
    gROstate = OFF;

    /*************************
    * Do Final RO back flush *
    **************************/
    outp.select = RO_ON | RO_FLUSH;
    mcx_mailbox_send(TASK_dev,0,&mbox_outp);
    //mcx_task_delay(MYSELF,0,TM_2sec);     1.28a
    outp.select = 0;
    mcx_mailbox_send(TASK_dev,0,&mbox_outp);
  }
}
