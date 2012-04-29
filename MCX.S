;PAGE
;MCX.AS
        .INCLUDE "DEFINE.S"
        .INCLUDE "MCXDEF.S"

        .PUBLIC  isrrtn,mcxsrv,dispch
	.PUBLIC  dispch,next,testat,goodtsk,switch
	.PUBLIC  setcur,rtncur,intrtn

	.EXTERNAL tickcnt, FREE, ACTIVE, curtsk, curtcb, hipri
	.EXTERNAL pritcb, intlvl, temp, _width, _depth, notmt
        .EXTERNAL SYSTACK, TCBDATA, QUEDATA

;******************************************************************************
;                                                                             *
;                            MCX11 REAL-TIME KERNEL                           *
;                               FOR THE MC68HC11                              *
;                                                                             *
;              by                               for support contact           *
;         Tom Barrett                               Mike Wood   M/D OE319     *
;  A.T. Barrett & Associates                       Motorola,Inc               *
;       Houston, Texas                       6501 William Cannon Dr West      *
;                                               Austin, Texas 78735           *
;       (713)728-9688                              (512)891-2717              *
;                                                                             *
;******************************************************************************
;                                N O T I C E                                  *
;******************************************************************************
;                                                                             *
; This product is distributed  without charge  to  users  via  the  MOTOROLA  *
; FREEWARE Bulletin Board. The product is provided "as is" without warranty   *
; of any kind, either expressed or implied, including, but not limited to any *
; warranties of merchantability and fitness for a particular purpose.  All    *
; risks of using this product including the entire costs of any necesary rem- *
; edies are those of the user and MOTOROLA assumes no liability of any kind.  *
;                                                                             *
;******************************************************************************
;******************************************************************************
;                       R E V I S I O N   H I S T O R Y                       
;                                Release 1
;
; Rev #    Date        Modifications/Corrections for new Revision
; -----  ---------     ------------------------------------------------------
;  1.1    5/30/89   1. Change calling sequence of .deque. so that the return
;                      value is in ACCA if entry size is 1 byte.
;
;                   2. Changed calling sequence of .enque. so that if the size
;                      of the entry is 1 byte, it is passed to .enque. in bits
;                      8-15 of IX. If the size of the entry is 2 bytes, IX8-15
;                      contains byte 1 while IX0-7 contains byte 2.
;
;         11/1/89   3. Corrected incorrect jump destination label in .timer.
;                       from .pend to dopend
;
;                   4. Corrected incorrect branch in .send. causing messages to
;                      be put into a receiver's mailbox in improper order.
;
;                   5. Cleared intlvl during initialization.
;
;  1.2    1/12/90   1. Changed the exit logic in .recv. when there is no msg
;                      waiting in the mailbox. This corrects a problem which
;                      would leave the task semaphore of the receiver task in
;                      a WAIT state. Since the .send. ESR does not signal a
;                      semaphore but simply unblocks the receiving task, it is
;                      not necessary to manipulate the task's semaphore. The
;                      original code is deleted and replaced with a simple
;                      set which backs up the PC and exits.
;
;                   2. Moved the code for the backup PC routine and made it an
;                      internal part of the .deque. function
;
;  1.3    3/23/90   1. Disabled interrupts for the endfast routine in isrrtn
;                      which protects this critical path from being interrupted
;                      after intlvl is decremented.  If this were to happen
;                      multiple ISRs could try to restore a task's state.
;
;                   2. Removed a hanging push in .signal. that was not poped 
;                      before returning if the semaphore was not in the wait
;                      state.  This would only be a problem if an ISR were 
;                      returning to the Dispatcher or another ISR.
;
;                   3. Moved the sei and cli in .signal. to tighten up the 
;                      critical code and improve interrupt response time.
;
;                   4. Moved the sei in .wait. to protect the Critical Code
;                      there, and deleted the cli since immrtn is CC also.
;                      This corrects several problems with tasks that wait on 
;                      semaphores from interrupts.
;                      NOTE:  to shorten interrupt latency here you might
;                      move the cli to after setting the wait and then jump
;                      to endtsk, the tradeoff is another stacked interrupt.
;
;                   5. Changed send and receive to use the _RCVWAT status bit
;                      as was origenally intended instead of _WAIT.  This 
;                      Corrects the problem where sending a message to a task
;                      that is waiting on any semaphore causes it to wake up
;                      as if that semaphore had been signaled.
;
;                   6. Added an sei to .deque. to protect some critical code
;                      in .wait. (see 3 above)
;
;                   7. In .enque. moved the loading of IY with queue data to
;                      later in the routine closer to where it is actualy used.
;                      This fixes the problem where the backup routine would
;                      subtract 2 from a word other than the tasks PC which
;                      was stored on its stack.
;
;                   8. Minor things realy, shaved off a clock cycle in dispch's
;                      tight loop, and used the _PEND symbol in .pend.
;
;  1.4    12/7/90   1. Changed cmpb to cmpa in task 5 of TEST.AS. Dequeue was
;                      changed to return the value in the A accumulator in V1.1.
;
;                   2. Fixed a problem with nested interrupt. If you had a
;                      nested interrupt that did not require a context switch
;                      followed by an interrupt that did require a context
;                      switch the stack became corrupted and the system would
;                      crash. Eliminated endfast routine.
;
;                   3. Fixed a problem with purging the timers. Once a timer
;                      was purged the next timer in the link was not lengthed
;                      the amount of time of the purged timer.
;
;                   4. Fixed a problem with cyclic timers in CLKDRIVER.AS.
;
; 1.5      1/24/91  1. Fixed a problem that I introduced in 1.4 in purge of
;                      timer. Messed with y index register and didn't restore.
; 1.5a SLF 12/21/94 1. Converted to COSMIC assembler format
;                   2. Converted assembly line from sta to staa instruction
;         staa   hipri              ; If so, then make it the new hipri task.
; 1.5b SLF 12/22/94 1. Made file DEFINE.S included since assembler does not 
;                      remember defines from one file to the next.
; 1.5c SLF 1/3/95   1. Converted assembly line immediate value from #-_SUSPND-1
;                      to hardcoded value #-81H.  Cosmic assembler could not
;                      handle.
; 1.5d SLF 1/16/95  1. Fixed bug in .next loop - ACCA (=task #) automatically
;                      get incremented without checking if it is a valid
;                      task number.
; 1.5e SLF 1/17/95  1. Fixed relative branches (bra) which were overflowing (3).
; 1.5c SLF 1/3/95   1. Converted assembly line immediate value from #81H
;                      to #7FH.  
;
; 1.6      4/3/91   1. Fixed bug resulting from Revision 1.2 #1. This change of
;                      labels on the BEQ is necessary to get around correction
;                      for V1.6 #2.
;
;                   2. The X-reg is not pointing to the current TCB and the esr
;                      stack frame is not in the Y-reg. This added code loads
;                      X-reg with the TCB address and gets Y-reg from the top-
;                      of-stack where it was pushed at SPECIAL.
;
; 1.6a BC  3/7/95      1.5d change removed, not needed.
;
;******************************************************************************
;******************************************************************************
;                           EXTERNAL REFERENCES
;******************************************************************************

        .EXTERNAL FLGTBL, NNAMSEM, NTASKS, QUEDATA, QUEDATL
        .EXTERNAL QADDR, QHADR, RSTSP, SYSTACK, STRTADR, STATLS, TCBDATA
        .EXTERNAL TCBDATL, TCBLEN, TCBADDR, WIDTH

;******************************************************************************

;       OPT    nol           Remove asterisk if you don't want the listing

;******************************************************************************
;                                                                             *
;                             MCX-11 VARIABLES                                *
;                        (These Will Reside in RAM)                           *
;                                                                             *
;******************************************************************************


;PAGE
;******************************************************************************
;                                                                             *
;                       	MCX11 TASK DISPATCHER                             *
;                                                                             *
;******************************************************************************
;                                                                             *
; This is the code that looks for the highest priority task that is in a RUN  *
; state. The tsk's state is contained in the Task Control Block (TCB). The    *
; job of the Dispatcher is to find the task having the highest priority and   *
; is ready to take control, i.e., run. This is a tight loop because every     *
; time a task gives up control, except for a pre-emption, the Dispatcher is   *
; called to find the next task to run.                                        *
;                                                                             *
;******************************************************************************

dispch : clra                      ; Use acc A for the task number
         ldx    #STATLS-TCBLEN     ; Set up base address of TCB list
next  :  clr    curtsk             ; Set current task # to 0
         lds    #SYSTACK           ; Load the system stack pointer address
         ldab   #7FH               ; Load constant 127 for presetting
         stab   hipri              ;   the task # of the highest priority task
;                                     - 1 is the highest priority
;                                     - 127 is the lowest priority
         ldab   #TCBLEN            ; Load a constant for TCB length attribute
         cli                       ; Enable interrupts
testat : 
goodtsk: inca                      ; Bump the task number by 1
         abx                       ; Get address of next TCB
;** (V1.3 #8) ***
         tst    STATE,x            ; See if all bits in task's status are 0
         bne    testat             ; If not, go check the next task.
switch : sei                       ; Task is ready to run. disable interrupts
         staa   hipri              ; Save task number as highest priority
         stx    pritcb             ; Save TCB address as that of hipri task
setcur : staa   curtsk             ;   also save acc A as current task number
         stx    curtcb             ; Save X-reg as address of curtsk's TCB
rtncur : lds    ACTSP,x            ; Load task's stack pointer from TCB
intrtn : rti                       ; Go to task via an interrupt return. The
;                                     task's context is on the task's stack.
;                                     This treats the context switch as if it
;                                     is an interrupt (it is likely to be).

;******************************************************************************
;                                                                             *
;                  	MCX11 COMMON INTERRUPT SERVICE RETURN                     *
;                                                                             *
;******************************************************************************
;                                                                             *
; This is the code that ALL interrupt service routines (ISR) MUST return thru *
; to complete the action of the interrupt. The ISR must have saved the MPU    *
; context, incremented the nested interrupt level (intlvl), and enabled other *
; interrupts before actually servicing the interrupt. Upon completion,  the   *
; ISR, should branch to this common interrupt return routine. The B accumu-   *
; lator  should  contain a semaphore  number or a zero.  The  former  value   *
; indicates that an event has occurred that requires action at the task level *
; and the latter indicates thee is no further action required.                *
;                                                                             *
; When ACCB contains a semaphore number, the semaphore is SIGNALed to inform  *
; any task associated with the event that it has occurred. A context switch   *
; will occur if the task in a WAIT state for that event is then ready to run  *
; and is of higher priority than the interrupted,i.e. current, task, and the  *
; level of nested interrupts is 1. A context switch will not occur if ALL of  *
; these conditions are not true. If the B accumulator contains a zero upon    *
; return from the ISR, there is an immediate return to the interrupted task   *
; if the level of nested interrupts is 1 or a return to the executive or to   *
; an interrupted ISR if the level of nested interrupts is >1.                 *
;                                                                             *
;******************************************************************************
;
isrrtn : tstb                      ; Test ACCB for content
         bne    signal2            ; If ACCB != 0, go signal the event. 
;                                     If ACCB = 0, return to point of interrupt
;
;** (V1.4 #2) ***
         bra    endtsk             ; Eliminated endfast routine.              |

.signal : tab                       ; *** PUT IN COMMENT BLOCK ***
signal2 :
         ldx    #FLGTBL-1          ; Got a semaphore number. Load address-1 of
         abx                       ;   semaphore table and add semaphore # to it
         clrb                      ; Clear ACCB so that we will have a zero
         sei                       ; Turn off interrupts  *** (V1.3 #2) ***
         ldaa   0,x                ; Get the content of the semaphore
         bpl    nowait             ; Branch if semaphore not in WAIT state
         incb                      ; If WAITing, set it to PENDing
nowait : stab   0,x                ; If PENDing or DONE, set to DONE
         tab                       ; Look at original content of semaphore
;** (V1.4 #2) ***
         bpl    immrtn             ; Return immediately if semaphore not WAITing|
         cli                       ; End of critical code  *** (V1.3 #2) ***
         negb                      ; If WAITing, 2's complement = task # WAITing
;                                     for the event.
;** (V1.3 #3) ***
         ldaa   #-_WAIT-1          ; Store the 1's complement of the WAIT
clrstat : pshb                      ; Save the task number.
         psha                      ; Then save the mask again.
         ldaa   #TCBLEN            ; Use the task number to compute the address
         mul                       ;   of its TCB
         addd   #STATLS-TCBLEN
         xgdx                      ; Put the TCB address in IX register
         pula                      ; Get the mask of bits to clear.
clrbits : sei
         anda   0,x                ; Clear the byte as given by the mask.
         staa   0,x                ; Save the updated TCB status byte.
         pula                      ; Get the task number again.
         bne    immrtn             ; Branch to immediate exit if task's status
;                                     indicates it is not ready to run (!=0).
         cmpa   hipri              ; If it is ready to run, is it of higher
;                                     priority than the current highest 
;                                     priority task which is ready to run?
         bge    immrtn             ; Branch if task priority < hipri.         
         staa   hipri              ; If so, then make it the new hipri task.
         stx    pritcb             ; Save its TCB address too.

endtsk : sei                       ; Disable interrupts
immrtn : dec    intlvl             ; Decrement the interrupt level.
         bne    intrtn             ; If it is >0, then the interrupt occurred
;                                     while we were doing system operations.
         ldaa   curtsk             ; See if the current task is 0.
         bne    notdisp            ; If not, branch.
         tsx                       ; If so, the MCX11 Dispatcher was interrupted
;                                     See what task was being checked when the
;                                     interrupt occurred. The context of the
;                                     Dispatcher is found on the system stack.
         ldaa   ACCA,x             ; Get the content of ACCA from the stack as
;                                     it holds the value of the "current task"
;                                     being used by the Dispatcher.
         cmpa   hipri              ; Compare it to hipri task number.
         ble    intrtn             ; The "current task" is of higher priority.
notdisp : ldx    pritcb             ; Get the hipri task's TCB address
         ldaa   hipri               ; and set up the hipri task number.
         brclr  setcur,0FFH,STATE,x ; If task is ready to run, go to it
;** (V1.5e) 1/17/95 SLF ***
         jmp    next                ; Otherwise, look for next lower priority
;                                     task that is ready to run.
;        bra    next                ; Otherwise, look for next lower priority
;                                     task that is ready to run.

;******************************************************************************
;                                                                             *
;                    MCX11 EXECUTIVE SERVICES DISPATCHER                      *
;                                                                             *
;******************************************************************************
;                                                                             *
; The Executive Services Dispatcher is the heart of MCX11. This is the point  *
; to which all Executive Service Requests (ESR) are vectored. The ESR Dis-    *
; patcher takes the function code of the ESR requested by the task and uses   *
; it as an index into a jump table to the various ESR functions. This code is *
; entered by a SWI instruction so it must be treated as a special kind of     *
; interrupt. The nested interrupt level is always =0 upon entry because the   *
; ESR is coming from a task. The ESR function is always found at the byte     *
; immediately following the SWI.                                              *
;                                                                             *
;******************************************************************************

mcxsrv : inc    intlvl             ; Increment the interrupt nest level
         ldx    curtcb
         sts    ACTSP,x            ; Save SP in TCB of current task
         tsy                       ; Save Top-of-Stack (TOS) in IY. This will
;                                     actually point to the CCR byte
         lds    #SYSTACK           ; Load SP with address of system stack
         cli                       ; Now interrupts are turned on
         ldx    PC,y               ; Set up pointer to the ESR function
         ldab   0,x                ; Get the ESR function code
         inx                       ; Bump the return address by 1
         stx    PC,y               ; Save the return address
         aslb                      ; Multiply the ESR function code by 2
         ldx    #jtable            ; Load address of the ESR jump table
         abx                       ; Add function code*2 to base address of
;                                     jump table to get ESR vector
         ldx    0,x                ; Load the ESR vector into IX
         ldd    ACCB,y             ; Load up ACCA and ACCB with the arguments
;                                     as passed by the calling task but
;                                     switch ACCA and ACCB contents.

;******************************************************************************
;                                                                             *
; At this point the processor context is as follows:                          *
;                                                                             *
;              CCR       conditions wrt contents of ACCB & ACCA (don't use)   *
;              ACCA      ACCB as passed from calling task (ESR dependent)     *
;              ACCB      ACCA as passed from calling task (ESR dependent)     *
;              IX        Address of the ESR function                          *
;              IY        Address of task's Top-of-Stack (CCR byte)            *
;              SP        Base of System Stack                                 *
;                                                                             *
; In addition, the nested interrupt level is = 1, curtsk  and hipri are set   *
; to the number of the calling task, and curtcb and hipri both contain the    *
; address of the current task's TCB.                                          *
;                                                                             *
;******************************************************************************

         jmp    0,x                ; Go to the ESR function

;******************************************************************************
;                                                                             *
;                              ESR JUMP TABLE                                 *
;                                                                             *
;******************************************************************************

jtable :
        .ADDR    endtsk             ; 0  = Immediate return (NOP)
        .ADDR    .wait              ; 1  = Wait for an event
        .ADDR    .signal            ; 2  = Signal a semaphore
        .ADDR    .pend              ; 3  = Set a semaphore to PENDing state
        .ADDR    .send              ; 4  = Send a task a message without waiting
        .ADDR    .sendw             ; 5  = Send a task a message with wait
        .ADDR    .recv              ; 6  = Receive a message
        .ADDR    .deque             ; 7  = Dequeue an entry from a FIFO queue
        .ADDR    .enque             ; 8  = Enqueue an entry into a FIFO queue
        .ADDR    .resume            ; 9  = Resume a task in suspension
        .ADDR    .suspnd            ; 10 = Suspend a task's operation
        .ADDR    .termn8            ; 11 = Terminate a task's operaton
        .ADDR    .xeqt              ; 12 = Execute a task
        .ADDR    .delay             ; 13 = Delay a task for a period of time
        .ADDR    .timer             ; 14 = Set up a timer
        .ADDR    .purge             ; 15 = Remove a given timer from active state


; ESR1
;******************************************************************************
;                                                                             *
; WAIT for an event to occur. This ESR is used to place the calling task into *
; a WAIT state where it will remain until the event happens. If the event has *
; already occurred, the wait does not happen and control is immediately re-   *
; turned to the calling task. WAIT is always associated with a semaphore. The *
; semaphore can be explicitly stated or it can be implicit, i.e. the task's   *
; semaphore. Regardless of how the semaphore is defined in the ESR, the sema- *
; phore must be in the PENDing state if the WAIT is to occur. The process     *
; handling the signaling of the event when it occurs must have prior know-    *
; ledge that the event uses a particular semaphore. Only in this way can the  *
; waiting task and the signalling task be connected.                          *
;                                                                             *
; calling sequence:                                                           *
;                                                                             *
;                   ACCB = semaphore number                                   *
;                   swi                                                       *
;                    FCB   .wait.                                                   *
;                   return: all registers unchanged                           *
;                                                                             *
;******************************************************************************

.wait :  tab                      ; Test the semaphore number
         bne    wait2             ; If it is defined, go set up the wait
         ldab   curtsk            ; Otherwise use the task's semaphore
         addb   #NNAMSEM
wait2 :  ldx    #FLGTBL-1
         abx                      ; Compute address of the semaphore
       	 sei			 ; Start of critical code  *** (V1.3 #4) ***
         tst    0,x               ; Test the semaphore contents
         bmi    waitt             ; Branch if already in the WAIT state
;				     This is a safety net, if you should unin-
;                                    tentionally have more than one task wait-
;				     ing on a semaphonre at a time all but the
;				     first will not wake up unless they are re-
;				     exicuted.
         beq    pendset           ; If DONE, go reset to PENDing and return
force :  ldaa   curtsk            ; If state of semaphore is PENDing, the
         nega                     ;   event has not yet occurred so
         staa   0,x               ; Set the semaphore to WAIT on current task.
waitt :  ldx    curtcb            ; Get current task's TCB address
         bset   _WAIT,STATE,x     ; Set the WAIT state in task's status byte.
;** (V1.3 #4) ***
         jmp    immrtn		 ; Critical code ends at actual return to task

;******************************************************************************
;                                                                             *
; Force a semaphore to a PENDing state. This function should be used to make  *
; sure that a semaphore is in the correct state if there is some doubt. A     *
; semaphore must be in a PENDing state before it can transition to a WAITing  *
; state. All semaphores should be initialized to a PENDing state at the time  *
; of system initialization. After that, they are generally maintained by the  *
; .wait and .signal functions. However, it may become necessary at some time  *
; to set a given semaphore PENDing to insure that the event is recognized as  *
; having yet occurred.                                                        *
;                                                                             *
; calling sequence:                                                           *
;                                                                             *
;                   ACCB = semaphore number                                   *
;                   swi                                                       *
;                    FCB   .pend.                                                   *
;                   return: all registers unchanged                           *
;                                                                             *
;******************************************************************************

.pend  : tab                      ; Put semaphore number in ACCB
         bne    dopend            ; Branch if semaphore is defined.
         ldab   curtsk            ; Else use task semaphore.
         addb   #NNAMSEM
dopend : ldx    #FLGTBL-1         ; Compute the address of the given semaphore.
         abx
pendset : ldaa   #_PEND            ; Set the semaphore to pending
         sei
         staa   0,x
         jmp    immrtn            ; All done.

;******************************************************************************
;                                                                             *
; Send a message and wait for reply. This function is the same as the .send.  *
; function except that it puts the sender into a Wait state until the task    *
; receiving the message signals the semaphore to indicate it is finished.     *
;                                                                             *
; calling sequence:                                                           *
;                                                                             *
;                   ACCA = Task number of the receiver                        *
;                   ACCB = Semaphore number                                   *
;                   IX = Message address                                      *
;                   swi                                                       *
;                    FCB   .sendw.                                                  *
;                   return: Registers are unchanged                           *
;                           ACCB will = Current task # if the semaphore       *
;                             number was = 0 at the time of the ESR           *
;                                                                             *
;******************************************************************************

.sendw : clrb                     ; Set switch = 0 for .sendw.

;******************************************************************************
;                                                                             *
; Send a message. This function sends a message but does not wait for a reply *
; to be returned from the receiver. There is a semaphore associated with the  *
; message but it is set to a PENDing state. The Sending task may perform a    *
; .wait. on that semaphore at a later time if necessary. Neither .send. nor   *
; .sendw. moves data from the sender to the receiver. Instead, only the       *
; pointer to the message is moved. Each task has a threaded list of messages  *
; which it can receive. The threaded list entry is the pointer to the message.*
; The message pointer is inserted into the threaded list in the order of the  *
; sending task's priority.                                                    *
;                                                                             *
; calling sequence:                                                           *
;                                                                             *
;                   ACCA = Task number of the receiver                        *
;                   ACCB = Semaphore number                                   *
;                   IX = Message address                                      *
;                   swi                                                       *
;                    FCB   .send.                                                   *
;                   return: Registers are unchanged                           *
;                           ACCB will = Task semaphore # if ACCB was = 0      *
;                                       at the time of the ESR.               *
;                                                                             *
;******************************************************************************

.send :  pshb                     ; Save the switch. Switch > 0 for .send
         ldab   ACCB,y            ; Get the semaphore number for the message.
         bne    havsema           ; Branch if semaphore # > 0.
         ldab   curtsk            ; If semaphore # = 0, use task semaphore.
         addb   #NNAMSEM
         stab   ACCB,y
havsema : ldx    #FLGTBL-1
         abx                      ; Compute address of the semaphore.
         pulb                     ; Get the send switch.
         tstb                     ; Test switch.
         bne    notw              ; Branch if switch set for no waiting.
         ldaa   curtsk              
         nega                     ; If switch set for wait, set -task #.
notw :   staa   0,x               ; Set the semaphore to proper value.
         ldx    curtcb            ; Get address of current task's TCB.
         tstb                     ; Test the switch again.
         bne    waitnot
         bset   _WAIT,STATE,x     ; Set the sender's status to WAITing.
waitnot : ldaa   ACCA,y            ; Get the receiving task #.
         psha                     ; Save it for later use, too.
         ldab   #TCBLEN
         mul                      ; Compute TCB address of receiving task.
         addd   #STATLS-TCBLEN
         xgdx
         pshx                     ; Save TCB address for later use.
         ldab   #MSGTHRD
         abx
         ldaa   ACCB,y            ; Get the semaphore number
         ldy    IX,y              ; Load IY with message address.
         staa   MSEMA,y           ; Set up semaphore # in message header.
         ldaa   curtsk
         staa   MTASK,y           ; Set up current task as sender.
         pshy                     ; Save address of the message header.
nsrtlp : ldy    MLINK,x           ; Look at the receiver's next message address.
         beq    insert            ; If pointer = 0, End-of=List. Go insert here.
         cmpa   MTASK,y           ; If there is a message on the thread, look at
;                                    the task number of its sender and compare
;                                    it to the sender's task number for this
;                                    new message.
;
         blo    insert            ; If the new message's sender has a priority |
;                                    higher than that of the threaded message,|
;                                    go insert the new message pointer in front
;                                    of the one on the list. (V1.1 #4)        |
         ldx    MLINK,x           ; Otherwise, walk the thread.
         bra    nsrtlp            ; Keep looking for a place to insert new msg.
insert : xgdy                     ; Put address of next nessage in ACCD.
         puly                     ; Get address of new message's header.
         std    MLINK,y           ; Link new message into threrad.
         xgdy
         std    MLINK,x
         pulx                     ; Get address of TCB of receiver again. 
;**(V1.3 #5)***
         ldaa   #-_RCVWAT-1
         jmp    clrbits           ; Go clear the receiver wait, if any.

;******************************************************************************
;                                                                             *
; Receive a message. This function is used by a receiving task to get the     *
; next message from the task's message thread. Normally, the next message is  *
; pointed to by the content of the message thread pointer in the task's TCB.  *
; However, it is also possible to get the next message sent from a given task *
; to the receiver task. By this technique, the receiver may effectively ig-   *
; nore other senders and reserve all activity for the one sending task. This  *
; is useful in designing server tasks which need to insure the dedicated use  *
; of a given resource.                                                        *
;                                                                             *
; calling sequence:                                                           *
;                                                                             *
;                   ACCA = task number (Otherwise this must be 0)             *
;                   swi                                                       *
;                    FCB   .recv.                                                   *
;                   return: IX contains the address of the received message   *
;                           All other registers unchanged                     *
;                                                                             *
;******************************************************************************

.recv :  ldx    curtcb            ; Set up to see if there is a special task
         tstb                     ; ACCB holds the task number or 0.
         bne    special           ; If ACCB != 0, it contains the task number.
         ldd    MSGTHRD,x         ; If not given task #, get next message.
;*** (V1.6 #1) ***
         beq    nulthrd           ; See if message thread is null. (=0)        |
         std    IX,y              ; It's not. We have a message. Save address.
         xgdy
         ldd    0,y               ; Then unlink the message from the thread.
         std    MSGTHRD,x
         jmp    endtsk            ; Wrap it up.
;** (V1.3 #5) ***
;** (V1.6 #2) ***
waitrcv : ldx    curtcb           ;  Load address of current TCB                |
         puly                     ; Then load address of stack frame again     |
nulthrd : bset   _RCVWAT,STATE,x  ;  If no message available, set up a RCVWAT   |
;** (V1.2 #1) *** 
         ldd    PC,y              ; Then back up the PC in the stacked         |
         subd   #2                ;  context by 2 bytes                        |
         std    PC,y              ;                                            |
         jmp    endtsk            ; Go to wrap up routine                      |

special : pshy                     ; Save address of the task's stack context.
         ldab   #MSGTHRD          ; ACCB still contains the task number.
         abx                      ; Adjust IX to point to message thread pntr.
         ldab   ACCA,y            ; Get the task number again.
rcvloop : ldy    0,x               ; Get the address of the next message header.
         beq    waitrcv           ; If no message available, go wait for one.
         cmpb   MTASK,y           ; Compare the sender's task number to special
         beq    rcvgo             ;   task number. If a match, go to it.
         blt    waitrcv           ;   If special task number < sender's, wait.
         ldx    0,x               ; If special task number > special, set up IX
         bra    rcvloop           ;   to point to current message header and
;                                    then go check next message.
rcvgo :  xgdy                     ; We have a message. Put its address in ACCD.
         puly                     ; Then set up IY to point to the stack context
         std    IX,y              ; Store the message address in the IX register
;                                    of the receiving task's stack context.
         xgdy                     ; Then put the pointer back into IY.
         ldd    0,y               ; Unlink the message from the thread.
         std    0,x
         jmp    endtsk            ; Go wrap it up.


;******************************************************************************
;                                                                             *
; Dequeue an element from a FIFO queue. This function is used to remove an    *
; element from a FIFO (First in- First out) queue. The queue is divided into  *
; two parts, the queue header and the queue body. The queue header contains   *
; all of the information about the state of the queue body. An attempt to re- *
; move an entry from an empty queue will place the caller into a wait state.  *
; When the queue has something put into it, the waiting task will be resumed. *
; Queue entries can be of any size but each queue handles only one size of    *
; entry.                                                                      *
;                                                                             *
; calling sequence:                                                           *
;                                                                             *
;                   ACCA = Queue number                                       *
;                   IX =   Destination address of dequeued entry of           *
;                            size > 2 bytes. Otherwise ignored.               *
;                   swi                                                       *
;                    FCB   .deque.                                                  *
;                   return: For an entry size of 1 byte:                      *
;                            ACCA = dequeued entry                            *
;                           For an entry size of 2 bytes:                     *
;                            ACCA = byte 1 of entry                           *
;                            ACCB = byte 2 of entry                           *
;                           Other registers unchanged                         *
;                                                                             *
;******************************************************************************

.deque : bsr    qsetup            ; Set up the queue header information
;                                    The queue depth and width are moved to
;                                    RAM work area as is the Not Empty sema-
;                                    phore. The address of the queue body is
;                                    in RAM at location "temp".
         ldab   CURSIZ,x          ; Check current size of queue to see if empty.
         beq    qempty            ; Branch if queue is empty.
         pshx                     ; Save the queue header address.
         ldaa   PIX,x             ; Not empty. Get the Put Index and subtract
         sba                      ;   the Current Size from it.
         bhs    nowrap            ; Check for wrap around.
         adda   _depth            ; If so, add Depth of queue.
nowrap : ldab   _width            ; ACCA = Get Index. Load ACCB with Width of
         pshb                     ;   an entry. Also save the width for later.
         dec    CURSIZ,x          ; Decrement the Current Size of the queue by 1
         mul                      ; Now multiply the Width times the Get Index
         addd   temp              ;   and add the base address of the queue body
         xgdx                     ;   to get address of the entry to dequeue.
         pula                     ; Get the Width again
         cmpa   #2                ; Is it > 2 bytes?
         bgt    dqgt2             ; Branch if width > 2 bytes.
         bge    dq2               ; See if Width = 2 bytes and branch if so.
         ldab   0,x               ; Width = 1 byte. Get it.
;*(V1.1 #1)**
         stab   ACCA,y            ; Store it in ACCA of the stack context.     |
         bra    dqend             ; That's it.
dq2 :    ldab   0,x               ; For Width = 2 bytes, get the 1st byte
         stab   ACCA,y            ;   and store it in ACCA of stack context.
         ldab   1,x               ; Then get the 2nd byte and store in in ACCB
         stab   ACCB,y            ;   of the stack context.
         bra    dqend             ; And that's that for size = 2.
dqgt2 :  ldy    IX,y              ; For Width > 2 bytes, get the destination
;                                    address from IX of the stack context.
dqloop : ldab   0,x               ; Get next byte from queue body.
         stab   0,y               ; Move it to next byte of the destination.
         inx
         iny
         deca                     ; Decrement the Width count.
         bne    dqloop            ; Loop until count is = 0.
dqend :  pulx                     ; When done, get the queue header address.
         ldab   QSEMA,x           ; Get the Queue NOT FULL semaphore #
;** (V1.5e) 1/17/95 SLF ***
         bne    postem
         jmp    nopost            ; See if anyone is waiting for NOT FULL.
;         beq    nopost            ; See if anyone is waiting for NOT FULL.
postem : clra                     ; There is. Go signal the semaphore.
         staa   QSEMA,x           ; But first, set the active semaphore # = 0
         jmp    signal2
qempty : addb   notmt             ; Queue is empty. Set up a wait state for this
         stab   QSEMA,x           ;   task on the Queue NOT EMPTY semaphore
;* (V1.2 #2) **
backup : pshb                     ; Save the semaphore number                  |
         ldd    PC,y              ; Here we have to back up the PC by 2 bytes  |
         subd   #2                ;   so that when the task restarts, it will  |
;                                 ;   be at the SWI used to call MCX11.        |
         std    PC,y              ; Save the decremented PC                    |
         pulb                     ; Get the semaphore number.                  |
         ldx    #FLGTBL-1         ;                                            |
         abx                      ; Calculate address of semaphore.            |
;** (V1.3 #6) ***
         sei			 ; Prepare to enter critical code
         jmp    force             ; Go set the wait state and clean up.        |

qsetup : clra
         aslb                     ; Multiply the queue number by the length
         pshb                     ; Save queue number * 2.
         addb   ACCA,y            ;   of the queue initialization data block.
         aslb
         addd   #QUEDATA-QUEDATL  ; Then add the address of the queue init data
         xgdx
         pulb                     ; Get queue number * 2 and add the number of
         addb   #NNAMSEM+NTASKS-1 ;   named semaphores + the number of tasks
         stab   notmt             ;   to get the NOT EMPTY semaphore #.
         ldd    WIDTH,x           ; Get the width and depth of the queue
         std    _width            ; Store them in the work area of RAM.
         ldd    QADDR,x           ; Get the queue body address
         std    temp              ; Store it in temp word in RAM
         ldx    QHADR,x           ; Get the Queue Header address
         rts                      ; Return

;******************************************************************************
;                                                                             *
; Enqueue an element into a FIFO queue. This function is used to insert an    *
; element into a FIFO (First in- First out) queue. The queue is divided into  *
; two parts, the queue header and the queue body. The queue header contains   *
; all of the information about the state of the queue body. An attempt to in- *
; sert an entry into a full queue  will place  the caller into a wait state.  *
; When the queue has something taken out of it, the waiting task will be      *
; resumed. Queue entries can be of any size but each queue handles only one   *
; size of entry.                                                              *
;                                                                             *
; calling sequence:                                                           *
;                                                                             *
;                   ACCA = Queue number                                       *
;                   IX =   Source address of entry to be enqueued if          *
;                            size > 2 bytes.                                  *
;                          Or, IX8-15 = byte to be enqueued if size = 1       *
;                          Or, IX8-15 = byte 1 and IX0-7 = byte 2 if size = 2 *
;                   swi                                                       *
;                    FCB   .enque.                                            *
;                   return: Registers are unchanged                           *
;                                                                             *
;******************************************************************************

.enque : bsr    qsetup            ; Set up the queue header information
         ldaa   _depth            ; Compare DEPTH of queue to its Current Size
         cmpa   CURSIZ,x          ;   to see if it is full.
         beq    full              ; Branch if queue is full.
         pshx                     ; Save address of the queue header.
         cmpa   PIX,x             ; Compare the depth to the Put Index to see
         bgt    wrapnot           ;   if there is an address wrap around.
         clra                     ;
         staa   PIX,x             ; If a wrap around, reset Put Index to 0.
wrapnot : ldaa   _width            ; Then use the width and the Put index to
         psha                     ;   compute the destination address in the 
         ldab   PIX,x             ;   queue body.
         mul
         addd   temp   
         inc    PIX,x             ; Bump the Put Index
         inc    CURSIZ,x          ; Bump the Current Size
;** (V1.3 #7) ***
         ldy    IX,y              ; Get data to be enqueued or pointer to it.
         xgdx                     ; Move destination address into IX.
         pula                     ; Get the Width again.
         cmpa   #2                ; See if Width > 2 bytes.
         bgt    nqloop            ; Branch if Width > 2 bytes
         xgdy                     ; If not, the data is in IY. Put it in ACCD.
         beq    nq2               ; Branch if Width = 2 bytes.
;* (V1.1 #2) **
         staa   0,x               ; Width is 1 byte. Store it from IX8-15.
         bra    nqend             ; Then go to wrap up routine.
nq2 :    std    0,x               ; For Width of 2 bytes, store from IX0-15.
         bra    nqend
nqloop : ldab   0,y               ; For Width > 2 bytes, move a byte from the
         stab   0,x               ;   source address to the destination address.
         inx
         iny
         deca                     ; Decrement the counter in ACCA.
         bne   nqloop             ; Loop until done.
nqend :  pulx                     ; When all done, get the address of the queue
         ldab  QSEMA,x            ;   header, and test for a waiter on queue
;                                    Not Empty.
         bne   postem             ; If there is a waiting task, go resume it.
nopost : jmp   endtsk             ; If not, just go back to caller.
full   : ldab  #1                 ; If queue is full, set up a wait on it.
;                                  Queue Not Full semaphore = Not Empty + 1.
;** (V1.5e) 1/16/95 SLF ***
         jmp   qempty             ; Go set up the Wait condition.
;         bra   qempty             ; Go set up the Wait condition.

;******************************************************************************
;                                                                             *
; Resume a task. This is the opposite function of .suspnd.. The SUSPEND state *
; is removed from the specified task. If the removal of the suspension makes  *
; the task runnable, it is scheduled. If it is of higher priority than the    *
; resuming task, a context switch will occur. Note that the task to be        *
; resumed must be explicitly specified by the content of ACCA. A task may not *
; resume itself, obviously.                                                   *
;                                                                             *
; calling sequence:                                                           *
;                                                                             *
;                   ACCA = task number                                        *
;                   swi                                                       *
;                    FCB   .resume.                                                 *
;                   return:  All registers unchanged                          *
;                                                                             *
;******************************************************************************

; ** 1/18/95  ACCA is the mask of the bits to clear the specified (ACCB)
;             task's state (1st byte in it's TCB)  - SLF

.resume : ldaa   #7FH              ;  Orginally : #-_SUSPND-1 - 1/18/95 SLF
          jmp    clrstat           ; Go clear the task's suspend status.

;******************************************************************************
;                                                                             *
; Suspend a task. This function is used to set a task into the SUSPEND state. *
; Once in this state, it cannot be made runnable again except by a .resume.   *
; ESR or an .execute. ESR. The task to be suspended is specified by putting   *
; its task number in ACCA. If the current task is suspendeing itself, ACCA    *
; is set to either the task number, if known, or more simply, a 0.            *
;                                                                             *
; calling sequence:                                                           *
;                                                                             *
;                   ACCA = task number (Otherwise this must be 0)             *
;                   swi                                                       *
;                    FCB   .suspend.                                                 *
;                   return:  All registers unchanged                          *
;                                                                             *
;******************************************************************************
.suspnd : tstb
         bne    notcur            ; See if the task to suspend is SELF.
         ldx    curtcb            ; It is. Load TCB address.
         bra    setspnd
notcur : ldaa   #TCBLEN           ; If not current task, compute TCB address
         mul                      ;   of the specified task.
         addd   #STATLS-TCBLEN
         xgdx
setspnd : bset  _SUSPND,STATE,x   ; Set the SUSPEND state in task's status byte.
         jmp    endtsk

;******************************************************************************
;                                                                             *
; Terminate a task. This is a seldom used task but is included for complete-  *
; ness. This function is called when it is desireable to terminate a task. It *
; may be any task in the system including the caller. If the caller is going  *
; to commit suicide, the given task number is set to 0. As a result of this   *
; ESR, all timers which are in process for the terminated task are purged     *
; the active timer list. The terminated task is set to an IDLE state. Once    *
; terminated, a task may be restarted only by the .xeqt. command.             *
;                                                                             *
; calling sequence:                                                           *
;                                                                             *
;                   ACCA = Task number (0 if self)                            *
;                   swi                                                       *
;                    FCB  .terminate.                                                 *
;                   return only if not terminating self. Registers unchanged  *
;                                                                             *
;******************************************************************************

.termn8 : tstb                     ; Test for self.
         bne    selfnot           ; Branch if task to terminate is not self.
         ldab   curtsk            ; If self, get current task number.
         ldx    curtcb            ; Then set up current task's TCB address.
         stab   temp+1            ; Store in lower half of temp
         bra    setidle           ; Then go set up the IDLE state.
selfnot : stab   temp+1            ; Store task # in lower half of temp.
         ldaa   #TCBLEN
         mul                      ; Compute TCB address of task to be terminated
         addd   #STATLS-TCBLEN
         xgdx
setidle : bset   _IDLE,STATE,x     ; Set task's state to IDLE
         clra
         jmp    beginp            ; Then go purge all the task's timers.


;******************************************************************************
;                                                                             *
; Execute a task. This function is used to put a task into the state of being *
; ready to run. Its TCB is initialized so that its stack pointer is set to    *
; the base address assigned to it. A stack frame is allocated on the task's   *
; stack for an initial context. That context is set to contain the task's     *
; starting address as the PC, while CCR, ACCB, and ACCA are cleared.          *
;                                                                             *
; calling sequence:                                                           *
;                                                                             *
;                   ACCA = task number (cannot be SELF)                       *
;                   swi                                                       *
;                    FCB   .execute.                                                   *
;                   return:  All registers unchanged                          *
;                                                                             *
;******************************************************************************
execute:
.xeqt :  pshb                     ; Save the task number
         ldaa   #TCBDATL          ; Load length of TCB init data block
         mul                      ; Multiply it by task number.
         addd   #TCBDATA-TCBDATL
         xgdx
         ldd    RSTSP,x           ; Get the base address of task's stack space.
         subd   #9
         xgdy
         ldd    STRTADR,x         ; Set up PC to be the starting address
         ldx    TCBADDR,x
         std    PC+1,y
         clra
         clrb
         staa   CCR+1,y           ; Clear CCR
         std    ACCB+1,y          ; Clear ACCB and ACCA
         xgdy                     ; Put address of stack context back into D
         std    ACTSP,x           ; Store address of the context in the TCB.
         clra
         jmp    clrbits           ; Go make the task runnable.

;******************************************************************************
;                                                                             *
; Delay a task for a period of time. This function uses the .timer. ESR       *
; to set up a timer and a wait state on the specified task.  When the timer   *
; elapses, the specified task is resumed.                                     *
;                                                                             *
; calling sequence:                                                           *
;                                                                             *
;                   ACCA = Task number (0 if self)                            *
;                   ACCB = Semaphore number (0 if task's semaphore)           *
;                   IX = Number of clock tocks to delay                       *
;                   swi                                                       *
;                    FCB   .delay.                                                  *
;                   return. Registers unchanged                               *
;                                                                             *
;******************************************************************************

.delay : ldaa   #0FFH              ; Set up switch.

;******************************************************************************
;                                                                             *
; Set up a timer for a task. This function is used to set a timer active for  *
; the specified task. The timer is in clock tock units. A clock tock is a     *
; system defined parameter. The timers are 16-bit timers; therefore, the      *
; maximum duration of a timer is determined by the clock tock frequency.      *
;                                                                             *
; calling sequence:                                                           *
;                                                                             *
;                   ACCA = Task number (0 if self)                            *
;                   ACCB = Semaphore number (0 if task's semaphore)           *
;                   IX = Number of clock tocks in timer                       *
;                   swi                                                       *
;                    FCB   .timer.                                                  *
;                   return. Registers unchanged                               *
;                           ACCA will be current task number if ACCA was = 0  *
;                             at the time of the ESR.                         *
;                                                                             *
;******************************************************************************

.timer : inca                     ; Set up switch: 0 = .delay., >0 = .timer.
         psha                     ; Save the switch.
         tstb                     ; Test for task being self.
         bne    notself           ; branch if task # is not 0.
         ldx    curtcb            ; Get address of current task's TCB
         ldaa   curtsk            ; get the current task number.
         staa   ACCA,y            ; Save it in the stack context.
         bra    gotask
notself : ldaa   #TCBLEN
         mul                      ; Compute TCB address of specified task.
         addd   #STATLS-TCBLEN
         xgdx
gotask : ldab   ACCA,y            ; Get the actual task number
         pshb                     ; Save the task number.
         pshx                     ; Save the TCB address of the specified task.
         ldaa   ACCB,y            ; Get the semaphore number to use with timer.
         bne    gotsema           ; Branch if semaphore number is defined.
         ldaa   ACCA,y            ; If semaphore # = 0, Use task semaphore of
         adda   #NNAMSEM
gotsema : psha                     ;   specified task. Save semaphore number.
         ldd    IX,y              ; Get the timer value.
         bmi    savrset           ; If timer < 0, go save the timer reset value.
         clra
         clrb                     ; Timer is a one-shot. Reset time = 0.
savrset : pshb                     ; Save the reset time.
         psha
         ldd    IX,y              ; Get the timer (as specified) again.
         bpl    savtime           ; See if it is cyclic or one-shot.
         coma                     ; If cyclic, be sure it is positive.
         negb
         bcs    savtime
         inca
savtime : ldx    #ACTIVE           ; Let IX contain the address of the active
         sei                      ;   timers.
tloop :  ldy    CLINK,x           ; Get address of next timer.
         beq    append            ; If end-of-list, go append new timer.
         cpd    CTOCKS,y          ; There is a timer that is active. Compare its
;                                    timer counts with those in the new timer.
         blt    nsrtm             ; If there are fewer tocks in the new timer,
;                                    go insert the new timer in front of the
;                                    active one.
         subd   CTOCKS,y          ; If not, subtract the tocks of the active
;                                    timer from the new timer.
         ldx    CLINK,x           ; Then move down the thread.
         bra    tloop
nsrtm :  pshb                     ; Save the remaining time in the timer.
         psha                      
         subd   CTOCKS,y          ; Update the active timer in the list
         coma                     ;   subtracting the new timer's residual
         negb                     ;   from it.
         bcs    updt
         inca
updt   : std    CTOCKS,y          ; Store the updated timer.
         bra    linkit
append : pshb                     ; Save the timer.
         psha
linkit : ldd    CLINK,x           ; Get the address of the current active timer.
         pshb
         psha                     ; Save the address
         ldd    FREE              ; Get the address of the next free timer block
         std    CLINK,x           ; Store it in link of the active timer block.
         xgdy                     ; IY now contains address of the new timer blk
         ldd    CLINK,y           ; Get address of next free timer block.
         std    FREE              ; Save it in pointer.
         pula
         pulb                     ; Get address of the current active timer.
         std    CLINK,y           ; Store it in link word of the new timer block
         pula
         pulb
         std    CTOCKS,y          ; Store timer tocks in new timer block.
         pula
         pulb
         std    CRESET,y          ; Store timer reset value.
         pulb
         stab   CSEMA,y           ; Store semaphore #
         pulx                     ; Get TCB address
         pula
         staa   CTASK,y           ; Store # of the task using the timer.
         pula                     ; Get the switch
         tsta
         beq    dlay              ; Branch if switch set for .delay.
         jmp    dopend            ; ** (V1.1 #3) **
dlay :   bset   _WAIT,STATE,x     ; If .delay., set wait state in task.
         ldx    #FLGTBL-1
         abx                      ; Compute address of semaphore.
         ldaa   CTASK,y           ; Get the # of the task being delayed.
         nega
         staa   0,x               ; Set the semaphore to WAIT state.
         jmp    endtsk

;******************************************************************************
;                                                                             *
; Purge a timer or timers. This function is used to purge one timer for a     *
; given task and semaphore combination or all of the timers for a given task. *
; Each timer so purged has its associated semaphore reset to the PENDing      *
; state.                                                                      *
;                                                                             *
; calling sequence:                                                           *
;                                                                             *
;                   ACCA = Task number (0 if self)                            *
;                   ACCB = Semaphore number (0 if task's semaphore)           *
;                   IX0-7= Switch: 0 = purge all timers for given task        *
;                                  >0 = purge timer matching task and sema    *
;                   swi                                                       *
;                    FCB   .purge.                                                  *
;                   return. Registers unchanged                               *
;                                                                             *
;******************************************************************************

.purge : tstb
         bne    gotsk             ; If ACCB > 0, task is specific.
         ldab   curtsk            ; Otherwise, use current task.
gotsk :  std    temp              ; Save task and semaphore numbers.
chksema : clra
         tst    IXL,y             ; Test the switch for type of purge.
         beq    beginp            ; Branch if switch = 0.
         ldaa   temp              ; Otherwise get the semaphore number to match.
         bne    beginp            ; Branch if it is defined.
         ldaa   temp+1            ; Otherwise use the task semaphore.
         adda   #NNAMSEM
beginp : ldab   temp+1            ; Load the task number to be matched.
         ldx    #ACTIVE           ; Set up pointer to 1st active timer.
         sei                      ; Interrupts off during this next phase.
ploop :  ldy    CLINK,x           ; Set IY = address of next active timer.
         beq    endpurg           ; If end-of-list, purge is complete.
         cmpb   CTASK,y           ; Compare task # in active timer to object #.
         beq    sametsk           ; Branch if both task numbers are equal.
chknxt : ldx    CLINK,x           ; If not equal, walk list to next active timer
         bra    ploop             ; Keep looking.
sametsk : tsta                     ; Tasks are equal. Test if semaphore # = 0.
         beq    remove            ; Branch if semaphore # = 0.
         cmpa   CSEMA,y           ; If semaphore > 0, Check it against semaphore
         bne    chknxt            ;   in active timer. Branch if not the same.
remove : pshb
         psha                     ; Save semaphore and task number.
         pshx                     ; Save IX for a moment.
         ldx    #FLGTBL-1
         ldab   CSEMA,y
         abx                      ; Compute address of timer's semaphore.
         ldaa   #_PEND
         staa   0,x               ; Set timer's semaphore back to PENDing.
         pulx                     ; Restore IX
         ldd    CLINK,y           ; Get the pointer to next active timer block.
         std    CLINK,x           ; Store it in link word of pointer to current
;                                    active timer.
;** (V1.4 #3) *** |
         beq    prgexit           ;                                          |
         ldd    CTOCKS,y          ; Update the next timer in link            |
;** (V1.5 #1) *** +
         pshy                     ; Save y                                 +
         ldy    CLINK,y           ;                                          |
         addd   CTOCKS,y          ;                                          |
         std    CTOCKS,y          ;                                          |
         puly                     ; Restore Y                              +
prgexit : ldd    FREE              ;                                          |
         std    CLINK,y           ; Make current timer block the first free one.
         xgdy
         std    FREE
         pula
         pulb                     ; Restore task and semaphore numbers.
         tsta                     ; Test the semaphore number.
         beq    ploop             ; Branch if semaphore = 0. (purge all timers)
endpurg : jmp    endtsk            ; Otherwise that's all.

        .END


