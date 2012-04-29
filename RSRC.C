/*********************************************
* Name: rsrc.c
*
* Description:
*  Resource wait Routines
*
**********************************************/
#include "hdrs.h"


/****************
* Lock resource
*****************/
void rsrc_lock(BYTE *rsrc)
    {
    sei();
    while ( *rsrc != 0 )
        {
        cli();
        mcx_task_delay(MYSELF,0,TM_50ms);
        sei();
        }
     (*rsrc)++;
     cli();
     }



/******************
* Unlock resource
*******************/
void rsrc_unlock(BYTE *rsrc)
    {
    sei();
    if ( *rsrc==1 )
        (*rsrc)--;
    else
      {
      _asm("nop\n");
      _asm("nop\n");
      _asm("nop\n");
      _asm("nop\n");
      _asm("nop\n");
      }
    cli();
    }
