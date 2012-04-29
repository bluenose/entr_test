;CLKDRIVR.AS
        .INCLUDE  "DEFINE.S"
        .INCLUDE  "MCXDEF.S"
        .PUBLIC   clkdrv, rtiisr
        .EXTERNAL isrrtn

	.EXTERNAL FLGTBL, QHDRTBL, STATLS, TIMERS
	.EXTERNAL tickcnt, FREE, ACTIVE, curtsk, curtcb, hipri
	.EXTERNAL pritcb, intlvl, temp, _width, _depth, notmt
        .EXTERNAL SYSTACK, TCBDATA, QUEDATA

;******************************************************************************
;                                                                             *
;  MCX11 Clock Driver task and Interrupt Service Routine (ISR) for the RTI    *
;  timer of the HC11 chip.                                                    *
;                                                                             *
; This task should be assembled with the source of MCX11 as it is considered  *
; a system task and does not need to clutter up the application code.         *
;                                                                             *
;                         --- N O T I C E ---                                 *
;                                                                             *
; This code assumes that the clock driver is task 1. If your assignment is    *
; different, be sure you make the necessary changes in this module or things  *
; are not going to work.                                                      *
;                                                                             *
;******************************************************************************

        .DEFINE    CLKTASK = 1       ; Clock driver is normally task # 1.

clkdrv : clrb                     ; Use task semaphore for synchronization.
         swi
        .BYTE  .wait.             ; Wait for a timer to expire.
tmrdone :
        ldx    ACTIVE             ; Get the address of the first active timer.
        beq    clkdrv             ; Make sure there is one.
        ldd    CTOCKS,x           ; If timer exists, check TOCK count to see
;                                     if it has expired. (count <= 0)
        bgt    clkdrv             ; If TOCK count > 0, timer hasn't expired.
        ldd    CLINK,x            ; Timer has expired. Unlink it from active
        std    ACTIVE             ;  timer thread. (ACTIVE <== (NEXT))
        ldd    FREE               ; Then put it into the unused timer thread.
        std    CLINK,x            ;   (NEXT <== (FREE), FREE <== (CURRENT))
        stx    FREE               ;
        ldd    CTASK,x            ; Get waiting task and semaphore numbers.
;** (V1.4) ***
        swi                       ;                                            |
        .BYTE  .signal.           ; In either case, signal the waiting task    |
;                                     that timer has expired.

        ldx    CRESET,x           ; Also get the Cyclic Timer Reset counts.
        bpl    tmrdone            ; If reset counter >= 0, timer is a one-shot.|
        swi
        .BYTE  .timer.            ; Otherwise, time is cyclic and needs to be
;                                 ;   reinserted into the timer thread.
        bra    tmrdone            ; Then go see if another timer expired too.

;******************************************************************************
;                                                                             *
;                      RTI Interrupt Service Routine                          *
;                                                                             *
;******************************************************************************

rtiisr : ldaa   #RTIF             ; Reset the interrupt flag
        staa   TFLG2
        dec    tickcnt            ; Decrement TICK count to see if TOCK
        beq    tockdon
rtixit :  rti                     ; If not a TOCK, exit quickly.
tockdon : ldaa   #TOCK            ; For a TOCK, reset the TICK count.
        staa   tickcnt
        ldx    ACTIVE             ; See if there is an active timer.
        beq    rtixit             ; If not, return immediately.
        ldd    CTOCKS,x           ; Decrement the TOCK count
        subd   #1                 ; If timer didn't expire, quick exit.
        std    CTOCKS,x
        bne    rtixit
        tst    intlvl             ; Here we have an expired timer. Test the
        bne    cnotl1             ;   interrupt nest level. branch if not lvl 1
        tst    curtsk             ; If level 1, test if Dispatcher was
        beq    cnotl1             ;   interrupted. If so, treat as system.
        ldx    curtcb             ; If a task was interrupted, get its TCB
        sts    ACTSP,x            ;   address and save the stack pointer.
        lds    #SYSTACK           ; Set up system stack pointer.
cnotl1 : inc    intlvl             ; Bump interrupt nest level.
        cli                       ; Enable interrupts.
        ldab   #NNAMSEM+CLKTASK   ; Set up task semaphore # for clock driver.
        jmp    isrrtn             ; Do a context switch to clock driver.


        .END


