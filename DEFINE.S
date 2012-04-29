;*=============================================================================
;*
;*  FILE NAME       :  define.s
;*
;*  BUILD FILE NAME :  c:\cosmic\mcx11.bat
;*
;*  DESCRIPTION :  This is an include containing the configuration constants
;*                 for the MCX11 operating system.
;*
;*  LOCAL FUNCTIONS :
;*
;*    Name                    Abstract
;*    ----------------------  -----------------------------------------------
;*    <None>
;*
;*  EXTERNAL DOCUMENTATION :
;*                                                    File
;*    Title                       Author        Ver.  Rev.  Comment
;*    --------------------------  -----------  -----  ----  -----------------
;*
;*
;* -----------------------------------------------------------------------------
;*
;*  REVISION HISTORY :
;*
;*  DCR #    DATE      REV.  WHO  COMMENTS
;*  -------  --------  ----  ---  ----------------------------------------------
;*  <None>   12/20/95  1.5   SLF  Moved from the system.as file to this include
;*                                since the Cosmic compiler couldn't carry symbols
;*                                between files.
;*  <None>   1/23/95   1.5   SLF  Changed RTI to 4.096 ms.
;*  <None>   2/26/95   1.5a  BC   MCXVAR set to EF, MCX11 commented out, and
;*                                TMSK2 change to 1024h from 24h.
;* ========================================================================== */

;******************************************************************************
;                      BASIC CONFIGURATION DATA EQUATES                       *
;                 These should change for your application                    *
;******************************************************************************


;******************************************************************************
;                           CLOCK AND PRESET EQUATES                          *
;                 These should change for your application                    *
;******************************************************************************
; TMSK2 change to 1024h from 24h - #BC 2/26/95

        .DEFINE    PORTA   = 1000h      ; Address of PORT A
        .DEFINE    COPRST  = 103Ah      ; Address of COPRST register
        .DEFINE    TMSK2   = 1024H      ; Address of TMSK2 register
        .DEFINE    TFLG2   = TMSK2+1
        .DEFINE    PACTL   = TFLG2+1
        .DEFINE    RTII    = 40H      ; RTI Interrupt enabled, prescale = 00
        .DEFINE    RTR     = 00H      ; 4.096 ms per TICK @ 8MHz
        .DEFINE    RTIF    = 40H
        .DEFINE    TOCK    = 1        ; 1 TICKs per TOCK (4.096ms)
;        .DEFINE    TOCK    = 2        ; 2 TICKs per TOCK (8.192ms)
;        .DEFINE    TOCK    = 3        ; 3 TICKs per TOCK (16.384ms)

;******************************************************************************
;                              MCX11  EQUATES                                 *
;                      (These should not be changed)                          *
;******************************************************************************

        .DEFINE    TCBLEN  = 5        ; Length of a Task Control Block
        .DEFINE    TIMRLEN = 8        ; Length of a Timer Block
        .DEFINE    QHDRLEN = 3        ; Length of a Queue Header


;******************************************************************************
;                   TCB INITIALIZATION DATA BLOCK EQUATES                     *
;                      (These should not be changed)                          *
;******************************************************************************

        .DEFINE    INITST  = 0         ; Task STATE at system initialization
        .DEFINE    STRTADR = 1         ; Task starting address
        .DEFINE    RSTSP   = 3         ; Base address of task's stack
        .DEFINE    TCBADDR = 5         ; Address of task's TCB
        .DEFINE    TCBDATL = 7         ; Length of a TCB initialization data block

;******************************************************************************
;                  QUEUE INITIALIZATION DATA BLOCK EQUATES                    *
;                      (These should not be changed)                          *
;******************************************************************************

        .DEFINE    WIDTH   = 0        ; Width of the queue entry
        .DEFINE    DEPTH   = 1        ; Depth of queue (# of entries)
        .DEFINE    QHADR   = 2        ; Queue Header address
        .DEFINE    QADDR   = 4        ; Address of the Queue Body
        .DEFINE    QUEDATL = 6        ; Length of a Queue initialization data block

;******************************************************************************
;    MCX ESR equates                                                          *
;******************************************************************************

        .DEFINE    .wait.   = 1       ; Wait for an event to occur
        .DEFINE    .signal. = 2       ; Signal the occurence of an event
        .DEFINE    .pend.   = 3       ; Set a semaphore to PENDing state
        .DEFINE    .send.   = 4       ; Send a message to a task
        .DEFINE    .sendw.  = 5       ; Send a message and wait for response
        .DEFINE    .receive.= 6       ; Receive a message
        .DEFINE    .deque.  = 7       ; Dequeue an entery from a FIFO queue
        .DEFINE    .enque.  = 8       ; Enqueue an entry into a FIFO queue
        .DEFINE    .resume. = 9       ; Resume a suspended task
        .DEFINE    .suspend. = 10     ; Suspend a task
        .DEFINE    .terminate. = 11   ; Terminate a task
        .DEFINE    .execute. = 12     ; Execute a task
        .DEFINE    .delay.  = 13      ; Delay a task for a period of time
        .DEFINE    .timer.  = 14      ; Set up a timer
        .DEFINE    .purge.  = 15      ; Purge active timer(s)


;******************************************************************************
;                              TASK STATE EQUATES                             *
;******************************************************************************

        .DEFINE _RUN    = 00H            ;    RUN status - 1/18/95 SLF
        .DEFINE _SUSPND = 80H            ;    SUSPENDed status
        .DEFINE _WAIT   = 40H            ;    WAITing status
        .DEFINE _RCVWAT = 20H            ;    Waiting status for message RECEIVE
        .DEFINE _IDLE   = 01H            ;    Task IDLE status

;******************************************************************************
;                             MESSAGE EQUATES                                 *
;******************************************************************************

        .DEFINE MLINK   = 0              ;    Message link pointer
        .DEFINE MTASK   = 2              ;    Message's sending task
        .DEFINE MSEMA   = 3              ;    Message semaphore
        .DEFINE MBODY   = 4              ;    Start of message body

;******************************************************************************
;                          TIMER BLOCK EQUATES                                *
;******************************************************************************

        .DEFINE CLINK   = 0              ;    Timer block link pointer
        .DEFINE CTOCKS  = 2              ;    Clock tocks in timer
        .DEFINE CRESET  = 4              ;    Reset timer
        .DEFINE CTASK   = 6              ;    Task waiting on timer
        .DEFINE CSEMA   = 7              ;    Semaphore number

;******************************************************************************
;                          QUEUE HEADER EQUATES                               *
;******************************************************************************

        .DEFINE CURSIZ  = 0              ;    Current size of queue (# of entries)
        .DEFINE PIX     = 1              ;    Put Index
        .DEFINE QSEMA   = 2              ;    Active semaphore: NOTMT or NOTFUL (=NOTMT+1)

;******************************************************************************
;                                TCB LAYOUT                                   *
;******************************************************************************
;
        .DEFINE STATE   = 0              ;    Byte 0   Task status:
;                                            bit 7: Suspended
;                                            bit 6: Waiting for an event
;                                            bit 5: Receive wait
;                                            bit 4: - Reserved -
;                                            bit 3: - Reserved -
;                                            bit 2: - Reserved -
;                                            bit 1: - Reserved -
;                                            bit 0: Task not in use
        .DEFINE ACTSP   = 1              ;    Byte 1-2 Active Stack Pointer for task
        .DEFINE MSGTHRD = 3              ;    Byte 3-4 Message thread pointer

;******************************************************************************
;                            STACK CONTEXT EQUATES                            *
;         (These values represent offsets from the Top-of-Stack +1)           *
;******************************************************************************

        .DEFINE CCR     = 0              ;     Condition Code Register
        .DEFINE ACCB    = 1              ;     Accumulator B
        .DEFINE ACCA    = 2              ;     Accumulator A
        .DEFINE IX      = 3              ;     Index Register X
        .DEFINE IXH     = 3              ;     Index Register X (High Byte)
        .DEFINE IXL     = 4              ;     Index Register X (Low Byte)
        .DEFINE IY      = 5              ;     Index Register Y
        .DEFINE PC      = 7              ;     Program Counter

;******************************************************************************
;                         MISCELLANEOUS EQUATES                               *
;******************************************************************************

        .DEFINE _PEND   = 1              ;     PENDing state


