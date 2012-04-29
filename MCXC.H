/*
**  FILE NAME       : MCXC.H
**
**  DESCRIPTION : Header for all applications that use MCX11
**  DESCRIPTION :  This include file contains C prototypes for MCX11
**                 Services.
**
** -----------------------------------------------------------------------------
**
**  REVISION HISTORY :
**
**  DATE      REV  WHO  COMMENTS
**  --------  ---  ---  ----------------------------------------------
**  11/04/94  0.0  SLF  oss_type.h created.
**  01/13/95  1.0  SLF  Moved typedefs in from other files.
**  03/01/95  1.1  BC   OSS_MAILBOX added.
**  03/02/95  1.2  BC   OSS_MAILBOX defined better.
**
**  11/04/94  0.0  slf  oss_std.h created.
**  01/13/95  0.0  slf  Added missing argument to delay and moved
**                      typedefs to oss_type.h
**  02/26/95  0.1  bc   Added oss_task_terminate
**  03/02/95  0.2  bc   Added oss_mailbox_send, sendw, receive.
**                      Renamed oss_sema_wait_for to oss_sema_wait.
**  06/26/95  1.0  bc   Both oss_type.h & oss_std.h consolidated into mcxc.h
**                      oss prefixes changed to mcx.
**  07/28/95  1.1  bc   MCX_ names changed.
**
** ========================================================================== */


#define BOOLEAN         unsigned char
#define MCX_STATUS      unsigned char
#define MCX_CHAN_NUMBER unsigned char
#define MCX_TASK        unsigned char
#define MCX_SEMA        unsigned char
#define MCX_QUEUE       unsigned char
#define MCX_ENTRY_ADDR  unsigned int
#define MCX_TICKS       unsigned int


typedef struct {
	void *      next;     /* link to next mailbox message */
  MCX_TASK    task;     /* task id                      */
  MCX_SEMA    sema;     /* semaphore id                 */
  void *      message;  /* pointer to message body      */
} MCX_MAILBOX;



/* ------------------------------------------------------------------- */
/* * * * * * * * * * * * * * TASK OS Services * * * * * * * * * * * *  */
/* ------------------------------------------------------------------- */

MCX_STATUS mcx_task_delay
  ( MCX_TASK task_id, MCX_SEMA sema_id, MCX_TICKS delay_period );
MCX_STATUS mcx_task_execute( MCX_TASK task_id );
MCX_STATUS mcx_task_suspend( MCX_TASK task_id );
MCX_STATUS mcx_task_resume( MCX_TASK task_id );
MCX_STATUS mcx_task_terminate( MCX_TASK task_id );
MCX_STATUS mcx_task_timer
  ( MCX_TASK task_id, MCX_SEMA sema_id, MCX_TICKS ticks );


/* ------------------------------------------------------------------- */
/* * * * * * * * * * * * * * QUEUE OS Services * * * * * * * * * * * * */
/* ------------------------------------------------------------------- */

MCX_STATUS mcx_dequeue( MCX_QUEUE queue_id, MCX_ENTRY_ADDR entry_ptr );
MCX_STATUS mcx_enqueue( MCX_QUEUE queue_id, MCX_ENTRY_ADDR entry_ptr );


/* ------------------------------------------------------------------- */
/* * * * * * * * * * * * * SEMAPHORE OS Services * * * * * * * * * * * */
/* ------------------------------------------------------------------- */

MCX_STATUS mcx_sema_signal( MCX_SEMA sema_id );
MCX_STATUS mcx_sema_pend( MCX_SEMA sema_id );
MCX_STATUS mcx_sema_wait( MCX_SEMA sema_id );


/* ------------------------------------------------------------------- */
/* * * * * * * * * * * * * * Mailbox OS Services * * * * * * * * * * * */
/* ------------------------------------------------------------------- */

MCX_TASK mcx_mailbox_send( MCX_TASK, MCX_SEMA, MCX_MAILBOX * );
MCX_TASK mcx_mailbox_sendw( MCX_TASK, MCX_SEMA, MCX_MAILBOX * );
MCX_STATUS mcx_mailbox_receive( MCX_TASK, MCX_SEMA, MCX_MAILBOX ** );


/* ------------------------------------------------------------------- */
/* * * * * * * * * * * * * * TIMER OS Services * * * * * * * * * * * * */
/* ------------------------------------------------------------------- */

MCX_STATUS mcx_timer_start
  ( MCX_TASK task_id, MCX_SEMA sema_id, MCX_TICKS ticks );
MCX_STATUS mcx_timer_purge
  ( MCX_TASK task_id, MCX_SEMA sema_id, MCX_TICKS timer_count );
