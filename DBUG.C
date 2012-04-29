/**********************************************************************
 * dbug.c    Guard against exceeding the maximum stack use by tasks   *
 **********************************************************************/
#define _DBUG_
#include "hdrs.h"


#define SCANTIME 8       /* Time period for stack scanning */
#define STACKLOW 0x40    /* Low stack limit */

/****************************
 *  External Prototypes
 ****************************/
extern void printline(char *txt);


/****************
 *  Prototypes
 ****************/
WORD stackfree(WORD i);




/***************
 *  Structures
 ***************/
typedef struct {
  unsigned char state;        /* task state                */
  void *        actsp;        /* active task stack pointer */
  MCX_MAILBOX * msgthrd;      /* message thread            */
} TCB;

typedef struct {
  unsigned char initstate;    /* initial task state */
  void *        start;        /* task start address */
  void *        stack;        /* starting task SP   */
  TCB  *        tcb;          /* TCB address        */
} TCB_DATA;



/***************************************
 *  Define global pointing to TCB data
 ***************************************/
extern TCB_DATA TCBDATA;
TCB_DATA *tcb_data = &TCBDATA;


/******************************************************************
 *  Determine the amount of free bytes in the stack for task i by
 *  scanning the stack from the base up and checking for non-zero
 *  values (zero = initialized state).
 ******************************************************************/
WORD stackfree(WORD i)
  {
  BYTE  *stk;
  WORD  stkbase;

  stkbase = (WORD)tcb_data[i].stack - mcxtask[i].stacksize + 1;
  stk = (BYTE *)stkbase;
  while (*stk==0)
    stk++;

  return (WORD)stk-stkbase;
  }




/*****************************************************************
 *  Periodically check the stack area of tasks to determine if
 *  the maximum stack use allowed is exceeded.
 *****************************************************************/
void task_dbug()
  {
  BYTE i;

  while (FOREVER)
    {
    mcx_task_delay(MYSELF,0,TM_1sec*SCANTIME);

    rsrc_lock(&rsrc_SCI);
    sci_mdb_mode();                     /* set SCI to MDB mode          */
    for (i=0; i<NTASKS; i++)
      {
      printf(" %s(%02X)=%02X ", mcxtask[i].name, mcxtask[i].stacksize, stackfree(i));
      /*printf("%X-%X  ", (WORD)tcb_data[i].stack-mcxtask[i].stacksize, (WORD)tcb_data[i].stack);*/
      }
    rsrc_unlock(&rsrc_SCI);
    }

	}
