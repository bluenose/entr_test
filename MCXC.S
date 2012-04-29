; =============================================================================
;*
;*
;*  FILE NAME       :  mcxc.s
;*
;*  DESCRIPTION :  This file contains the C interface to various selected
;*                 MCX11 services.  This is written for the COSMIC Assembler.
;*
;*  LOCAL FUNCTIONS :
;*
;*    Name                    Abstract
;*    ----------------------  -----------------------------------------------
;*    mcx_task_delay()        Delay Specified Task for Period of Time
;*    mcx_task_execute()      Execute Specified Task
;*    mcx_task_suspend()      Suspend Specified Task
;*    mcx_task_resume()       Resume Specified Task
;*    mcx_dequeue()           Get an Entry from Specified FIFO Queue
;*    mcx_enqueue()           Insert Data as an Entry into FIFO Queue
;*    mcx_sema_signal()       Signal Specified Semaphore
;*    mcx_sema_pend()         Force Specified Semaphore to PENDING State
;*    mcx_sema_wait()         Wait on Specified Semaphore
;*    mcx_mailbox_send()      Send a mailbox msg to a task
;*    mcx_mailbox_sendw()     Send a mailbox msg to a task & wait for SEMA
;*    mcx_mailbox_receive()   Receive a mailbox msg
;*    mcx_timer_start()       Start Specified Task's Timer
;*    mcx_timer_purge()       Purge Specified Task's Timer
;*
;*  EXTERNAL DOCUMENTATION :
;*                                                    File
;*    Title                       Author        Ver.  Rev.  Comment
;*    --------------------------  -----------  -----  ----  -----------------
;*   "MCX11 MicroController eXecutive
;*    for the Motorola MC68HC11"
;*                                A.T. Barrett  1.3    0.0  From Moto Freeware
;*   "C Cross Compiler User's Guide
;*    for Motorola MC68HC11"
;*                                COSMIC, Sarl and Whitesmiths
;*                                            3.32.10  0.0  page 3-12, C Interface
;*                                                          to Assembly Language
;*
;* -----------------------------------------------------------------------------
;*
;*  REVISION HISTORY :
;*
;*  DATE      REV.  WHO  COMMENTS
;*  --------  ----  ---  ----------------------------------------------
;*  12/19/94  0.0   slf  Created.
;*  01/17/95  1.0   slf  Updated interface to not mess with SP and to
;*                       use Y to get values from stack for ESRs.
;*  02/26/95  1.1   bc   References into stack changed. Task terminate
;*                       added.
;*  03/02/95  1.2   bc   Added oss_mailbox_send, sendw, & receive.
;*                       Renamed oss_sema_wait_for to oss_sema_wait.
;*  06/26/95  2.0   bc   OSS prefixes changed to MCX.
;*
;* ========================================================================== *


        .define .wait.        = 1
        .define .signal.      = 2
        .define .pend.        = 3
        .define .send.        = 4
        .define .sendw.       = 5
        .define .receive.     = 6
        .define .deque.       = 7
        .define .enque.       = 8
        .define .resume.      = 9
        .define .suspend.     = 10
        .define .terminate.   = 11
        .define .execute.     = 12
        .define .delay.       = 13
        .define .timer.       = 14
        .define .purge.       = 15

;*************************************************************************
; MCX_STATUS mcx_task_delay( MCX_TASK_ID task_id,                        *
;                            MCX_SEMA_ID sema_id,                        *
;                            MCX_TASK_TOCK delay_period );               *
;*************************************************************************

_mcx_task_delay:

        pshy
        pshx
        tsy             ; copy SP into Y - use Y plus offset to access stack values
	tba		; task id from call into ACCA
        ldab    7,Y     ; sema id from stack into ACCB
        ldx     8,Y     ; number of clock tocks to delay from stack in IX
        swi
        .byte   .delay.
        pulx
        puly
        rts

;**************************************************************************
; MCX_STATUS mcx_timer_start                                              *
;  ( MCX_TASK_ID task_id, MCX_SEMA_ID sema_id, MCX_TASK_TOCK tocks );     *
;**************************************************************************

_mcx_timer_start:

        pshy
        pshx
        tsy             ; copy SP into Y - use Y plus offset to access stack values
	tba		; task id from call into ACCA
        ldab    7,Y     ; sema id from stack into ACCB
        ldx     8,Y     ; number of clock tocks to delay from stack in IX
        swi
        .byte   .timer.
        pulx
        puly
        rts

;**************************************************************************
; MCX_STATUS mcx_task_execute( MCX_TASK_ID task_id );                     *
;**************************************************************************

_mcx_task_execute:

        tba              ; task id from stack into ACCA
        swi
        .byte   .execute.
        rts

;**************************************************************************
; MCX_STATUS mcx_task_suspend( MCX_TASK_ID task_id );                     *
;**************************************************************************

_mcx_task_suspend:

        tba             ; task id from stack into ACCA
        swi
        .byte   .suspend.
        rts

;**************************************************************************
; MCX_STATUS mcx_task_resume( MCX_TASK_ID task_id );                       *
;**************************************************************************

_mcx_task_resume:

        tba             ; task id from stack into ACCA
        swi
        .byte   .resume.
        rts

;**************************************************************************
; MCX_STATUS mcx_task_terminate( MCX_TASK_ID task_id );                    *
;**************************************************************************

_mcx_task_terminate:

        tba             ; task id from stack into ACCA
        swi
        .byte   .terminate.
        rts

;****************************************************************************
; MCX_STATUS mcx_dequeue( MCX_QUEUE_ID queue_id, MCX_ENTRY_ADDR entry_ptr); *
;****************************************************************************

_mcx_dequeue:

        pshy
        pshx
        tsy             ; copy SP into Y - use Y plus offset to access stack values
	tba		; queue id from call into ACCA
        ldx     6,Y     ; entry number from stack into IX
        swi
        .byte   .deque.
        pulx
        puly
        rts

;****************************************************************************
; MCX_STATUS mcx_enqueue( MCX_QUEUE_ID queue_id, MCX_ENTRY_ADDR entry_ptr); *
;****************************************************************************

_mcx_enqueue:

        pshy
        pshx
        tsy             ; copy SP into Y - use Y plus offset to access stack values
        tba             ; queue id from stack into ACCA
        ldx     6,Y     ; entry number from stack into IX
        swi
        .byte   .enque.
        pulx
        puly
        rts

;**************************************************************************
; MCX_STATUS mcx_sema_signal( MCX_SEMA_ID sema_id );                      *
;**************************************************************************

_mcx_sema_signal:

        ; semaphore id is in ACCB
        swi
        .byte   .signal.
        rts

;**************************************************************************
; MCX_STATUS mcx_sema_pend( MCX_SEMA_ID sema_id );                        *
;**************************************************************************

_mcx_sema_pend:

        ; semaphore id is in ACCB
        swi
        .byte   .pend.
        rts

;**************************************************************************
; MCX_STATUS mcx_sema_wait( MCX_SEMA_ID sema_id );                        *
;**************************************************************************

_mcx_sema_wait :

        ; semaphore id is in ACCB
        swi
        .byte   .wait.
        rts

;****************************************************************************
; MCX_TASK_ID mcx_mailbox_send( MCX_TASK_ID, MCX_SEMA_ID, MCX_MAILBOX * );  *
;****************************************************************************

_mcx_mailbox_send :

        pshy
        pshx
        tsy             ; copy SP into Y - use Y plus offset to access stack values
	tba		; task id from call into ACCA
        ldab    7,Y     ; sema id from stack into ACCB
        ldx     8,Y     ; address of mailbox message
        swi
        .byte   .send.
        pulx
        puly
	tab		; Task id is in AccB
	clra
        rts

;*****************************************************************************
; MCX_TASK_ID mcx_mailbox_sendw( MCX_TASK_ID, MCX_SEMA_ID, MCX_MAILBOX * );  *
;*****************************************************************************

_mcx_mailbox_sendw :

        pshy
        pshx
        tsy             ; copy SP into Y - use Y plus offset to access stack values
	tba		; task id from call into ACCA
        ldab    7,Y     ; sema id from stack into ACCB
        ldx     8,Y     ; address of mailbox message
        swi
        .byte   .sendw.
        pulx
        puly
	tab		; Task id is in AccB
	clra
        rts

;*****************************************************************************
; MCX_STATUS mcx_mailbox_receive( MCX_TASK_ID, MCX_SEMA_ID, MCX_MAILBOX * ); *
;*****************************************************************************

_mcx_mailbox_receive :

        pshy
        pshx
        tsy             ; copy SP into Y - use Y plus offset to access stack values
	tba		; task id from call into ACCA
        ldab    7,Y     ; sema id from stack into ACCB
        swi
        .byte   .receive.
        ldy     8,Y     ; iy = address pointer
	stx	0,Y	; store mailbox message address to address pointer
        pulx
        puly
        rts

;*****************************************************************************
; MCX_STATUS mcx_timer_purge                                                 *
;  (MCX_TASK_ID task_id, MCX_SEMA_ID sema_id, MCX_TIMER_COUNT no_of_timers); *
;*****************************************************************************

_mcx_timer_purge:

        pshy
        pshx
        tsy             ; copy SP into Y - use Y plus offset to access stack values
	tba		; task id from call into ACCA
        ldab    7,Y     ; sema id from stack into ACCB
        ldx     8,Y     ; 0 = purge all timers, >0 = Purge specific timer
        swi
        .byte   .purge.
        pulx
        puly
        rts

;* * * ------------ PUBLIC DECLARATIONS ------------ * * *

        .public   _mcx_task_delay
        .public   _mcx_timer_start
        .public   _mcx_task_execute
        .public   _mcx_task_suspend
        .public   _mcx_task_resume
        .public   _mcx_task_terminate
        .public   _mcx_dequeue
        .public   _mcx_enqueue
        .public   _mcx_sema_signal
        .public   _mcx_sema_pend
        .public   _mcx_mailbox_send
        .public   _mcx_mailbox_sendw
        .public   _mcx_mailbox_receive
        .public   _mcx_sema_wait
        .public   _mcx_timer_purge

        .end
