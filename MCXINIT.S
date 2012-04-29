;SYSTEM.AS                                                                    *
        .INCLUDE "DEFINE.S"
	.INCLUDE "MCXDEF.S"

        .EXTERNAL clkdrv, __stext, dispch, nulltsk
        .PUBLIC   mcxinit, mcxgo

	.EXTERNAL FLGTBL, QHDRTBL, STATLS, TIMERS
	.EXTERNAL tickcnt, FREE, ACTIVE, curtsk, curtcb, hipri
	.EXTERNAL pritcb, intlvl, temp, _width, _depth, notmt
        .EXTERNAL SYSTACK, TCBDATA, QUEDATA

;******************************************************************************
;******************************************************************************
;******************************************************************************

        .PSECT  _text   ; ORG MCX11

;******************************************************************************
;******************************************************************************
;******************************************************************************
;                                                                             *
;                            MCX11 INITIALIZATION                             *
;                                                                             *
; This is the MCX11 Initialization routine in which all of the 68HC11 RAM and *
; internal registers get initialized. This routine is entered upon a Power On *
; RESET; therefore the first 64 cycles must be spent initializing those areas *
; of the 68HC11 control register space which must be set up during that brief *
; period. Then all of the system tables must be set to their initial content. *
; The user should also include any initialization operatons unique to his own *
; application. Upon completion, the routine should jump to the start of the   *
; MCX11 Dispatcher at lable "dispch".                                         *
;                                                                             *
;******************************************************************************

;******************************************************************************
;     Put the special startup code here which must be done within the first   *
;     64 cycles after Power-on/Reset & set up the internal registers with     *
;     respect to the application requirements.                                *
;******************************************************************************



;        .SET  mcxinit = .      ; This is MCX11 entry point
mcxinit :

        lds    #SYSTACK
        ldaa   #RTII
        staa   TMSK2             ; Enable RTI Interrupt
        ldaa   #RTR
        staa   PACTL             ; Set up RTI clock rate

; Do C Environment Setup/Initialization - 1/11/95 SLF
        jmp    __stext

;******************************************************************************
;                                                                             *
; The following instruction is included for use with an EVM board. If you are *
; going to want to initialize the EVM in such a manner that the MPU will use  *
; the first 64 cycles to initialize the control registers, leave this one     *
; instruction in place and perform the following startup procedure:           *
;                                                                             *
;            1. Press the USER RESET button on EVM board.
;            2. Press ABORT SWITCH on EVM board.
;            3. Ignore any ERROR MESSAGE from the EVM and press ENTER on your *
;               debug console's keyboard. This should show the Register set.  *
;            4. Change the content of Stack Pointer Register to legal value   *
;            5. Start the program with a G command at the address of 'mcxgo'. *
;                                                                             *
; If this is not used on an EVM board, and a normal RESET sequence is desired *
; simply remove the following instruction.
;                                                                             *
;******************************************************************************

; Commented - using emulator - 1/11/95 SLF
;        bra     .                ; Just wait for the ABORT.

;******************************************************************************
; The second job in the initialization process is to link all of the system   *
; timer blocks. The address of the first timer block is stored in the word    *
; in RAM labled "FREE". The word labled "ACTIVE" is set to a 0.               *
;******************************************************************************

mcxgo : ldaa   #TOCK
        staa   tickcnt           ; Set up Tick counter
        ldy    #NTIMERS          ; Set up counter for the number of timers
        ldd    #TIMERS           ; Get address of Timer Blocks
        std    FREE              ; Store it in the pointer to 1st free block
        ldx    #TIMERS           ; Set up pointer to 1st Timer Block
tmlp :  dey                      ; Decrement timer counter and see if done.
        beq    tmdone
        addd   #TIMRLEN          ; If not done, calculate address of next block
        std    0,x               ; Store it in link word of current block.
        ldx    0,x               ; Then set up pointer to the next block.
        bra    tmlp
tmdone : clrb
        clra                     ;When done linking the timer blocks, set
        std    0,x               ;   the link word of the last timer to 0.
        std    ACTIVE            ; Set up 0 entry for no active timers.

;******************************************************************************
; The third thing to do during initialization is to set up the queues. The    *
; queues have a Header and a Body and they are not contiguous. The Queue      *
; Headers are found immediately below the Timer Blocks in RAM. The Bodies are *
; found immediately above the Task Control Blocks.                            *
;******************************************************************************

        ldx    #QHDRTBL          ; Set up pointer to the 1st queue header
qlp :   clr    0,x               ; Clear next byte in the queue header area.
        inx
        cpx    #TIMERS           ; See if all queue headers are cleared.
        bne    qlp               ; keep looping until done

;******************************************************************************
; The fourth function during initialization is to set all of the semaphores   *
; to a PENDing state. The number of semaphores is equal to the number of the  *
; named semaphores + the number of tasks + twice the number of queues.        *
;******************************************************************************

        ldaa   #NQUEUES          ; Get the number of queues in system.
        asla                     ; Multiply it by 2.
        adda   #NNAMSEM          ; Add the number of named semaphores
        adda   #NTASKS           ; Then add the number of tasks.
        ldx    #FLGTBL           ; Set up pointer to base of the semaphores.
        ldab   #_PEND
smlp :  stab   0,x               ; Store the PENDing value in the semaphore.
        inx
        deca                     ; Decrement the counter
        bne    smlp              ; Keep looping until all are set to PENDing.


;******************************************************************************
; The fifth thing to do in the initialization is to set up the RAM resident   *
; portions of the Task Control Blocks (TCB). The TCBs are found immediately   *
; above the semaphores.                                                       *
;******************************************************************************

        ldy    #TCBDATA          ; Set up pointer to User Defined list of
;                                    TCB initialization data block.
        ldaa   #NTASKS           ; Set up task counter too.
        staa   notmt
tcblp : ldx    TCBADDR,y         ; Set up a pointer to the TCB.
        clr    MSGTHRD,x         ; Clear the message thread.
        clr    MSGTHRD+1,x
        ldab   INITST,y          ; Get the initial status of the task.
        stab   STATE,x           ; Store it as the status byte of the TCB.
        bne    nostart           ; Branch if task state show it is not ready.
        ldd    RSTSP,y
        subd   #9
        std    ACTSP,x           ; Set up pointer to the stack context.
        xgdx                     ; Put stacked context address into IX.
        ldd    STRTADR,y
        std    PC+1,x            ; Put task's starting address into PC word
        clra                     ;  of the context. Then clear ACCB, ACCA,
        clrb                     ;  and CCR.
        staa   CCR+1,x
        std    ACCB+1,x
nostart : ldab   #TCBDATL
        aby                      ; Bump pointer to next TCB initialization
        dec    notmt             ;  data block.
        bne    tcblp             ; Loop until done.

; Now set up the null task

        ldx    #SYSTACK+9
        ldd    #nulltsk          ; Get the starting address of the null task.
        std    PC+1,x            ; Put task's starting address into PC word
        clr    CCR+1,x           ;   of the context. Then clear ACCB, ACCA,
        clr    ACCB+1,x          ;   and CCR.
        clr    ACCA+1,x
        ldd    #NTASKS*TCBLEN+STATLS ; Get TCB address of the null task
        xgdx
        std    ACTSP,x           ; Save address of null task's context
        clr    STATE,x           ; Make sure null task is READY.
        clr    intlvl            ; Clear interrupt level.  ** (V1.1 #5) **

; Uncommented this jump - since dispch does not immediately follow this - 1/11/95 slf
        jmp    dispch            ; Start the MCX Dispatcher

;******************************************************************************
;  The next location should contain the entry point to the Dispatcher.        *
;  The file "MCX.AS" should follow this file. If not, then you should remove  *
;  the asterisk on the line above which causes a direct jump to the label of  *
;  the Dispatcher, "dispch".                                                  *
;******************************************************************************

        .end
