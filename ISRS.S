        .INCLUDE "DEFINE.S"
        .INCLUDE "MCXDEF.S"

        .EXTERNAL isrrtn, curtsk, curtcb, SYSTACK, intlvl
        .EXTERNAL _sci_int, _gSlaveGalCount, _gMeterPulse, _gGalSolCount
        .PUBLIC   scisrv, ic1srv, ic2srv, ic3srv, oc1srv
        .PUBLIC _irqrtn, _xirqrtn, _coprtn, _rti, _wait1ms, _wait2ms, _wait100us

        .define TFLG1   = 1023h         ; Timer Interrupt Flag 1
        .define PORTE   = 100Ah         ; Port E
        .define TCNT    = 100Fh         ; Timer count (2 bytes)
        .define TOC1    = 1016h         ; Timer output compare #1 (2 bytes)
        .define IC1F    = 04h           ; Input Capture Flag 1
        .define IC2F    = 02h           ; Input Capture Flag 2
        .define IC3F    = 01h           ; Input Capture Flag 3
        .define OC1F    = 80h           ; Output Compare Flag 1
        .define MPULSE1 = 08h           ; Pulse from flow #1
        .define MPULSE2 = 04h           ; Pulse from flow #2
        .define CLK_MS  = 500           ; Number of ticks per 1 ms


	.psect	_bss

lastmeter:      .word 0
pulse:          .word 0

        .psect	_text

;
;
; Stub code to handle interrupt
;
_irqrtn :       rti
_xirqrtn :      rti
_coprtn :       rti
_rti :          rti


;
;
; Code to handle sci interrupt service
;
scisrv: tst     intlvl          ; test for task level interrupt
        bne     0$              ; branch if not task level int
        tst     curtsk          ; see if dispatcher interrupted
        beq     0$              ; branch if so
        ldx     curtcb          ; get task's TCB address
        sts     ACTSP,x         ; save task's stack pointer
        lds     #SYSTACK        ; change to system stack
0$:     inc     intlvl          ; increment the interrupt level
        tsx                     ; setup C Frame Pointer (FP)
        jsr     _sci_int        ; service sci
        ldab    #0              ; do not signal any semaphore
        jmp     isrrtn          ; jump to exit logic

;
;
; Count IC1 pulses from slave
;
ic1srv: inc     _gSlaveGalCount
        inc     _gGalSolCount
        ldaa    #IC1F
        staa    TFLG1           ; Clear IC1 flag
        rti

;
;
; Count IC2 pulses from flow meter
;
ic2srv: inc     pulse           ; Increment fine pulse control
        ldaa    pulse
        anda    #01h            ; Count every other pulse
        beq     99$
        inc     _gMeterPulse    ; Increment pulse meter
99$:    ldaa    #IC2F
        staa    TFLG1           ; Clear IC2 flag
        rti

;
;
; Count IC3 pulses from flow meter
;
ic3srv: inc     pulse+1         ; Increment fine pulse control
        ldaa    pulse+1
        anda    #01h            ; Count every other pulse
        beq     99$
        inc     _gMeterPulse+1  ; Increment pulse meter
99$:    ldaa    #IC3F
        staa    TFLG1           ; Clear IC3 flag
        rti

;
;
; Count Meter pulses (Obsolete - Not Used)
;
oc1srv: ldd     TCNT            ; Load timer count
        addd    #CLK_MS         ; Add to determine new time
        std     TOC1            ; Set OC timer value
        ldaa    #OC1F
        staa    TFLG1           ; Clear OC1 flag
        ;--------
        tst     intlvl          ; test for task level interrupt
        bne     0$              ; branch if not task level int
        tst     curtsk          ; see if dispatcher interrupted
        beq     0$              ; branch if so
        ldx     curtcb          ; get task's TCB address
        sts     ACTSP,x         ; save task's stack pointer
        lds     #SYSTACK        ; change to system stack
0$:     inc     intlvl          ; increment the interrupt level
        bsr     meter           ; Call meter rtn
        ldab    #0              ; do not signal any semaphore
        jmp     isrrtn          ; jump to exit logic
        ;--------
meter:  ldab    #0              ; index to meter 1
        ldaa    PORTE
        anda    #MPULSE1        ; check pulse polarity
        bsr     met
        ldab    #1              ; index to meter 2
        ldaa    PORTE
        anda    #MPULSE2        ; check pulse polarity
        ;--------
met:    beq     metlow
        ldx     #lastmeter      ; last meter state
        abx                     ; index to correct meter #
        tst     0,x             ; check last state
        bne     99$             ; exit when nothing is different
        inc     0,x             ; last state now high
        ldx     #_gMeterPulse
        abx                     ; index to correct meter #
        inc     0,x             ; incr meter counter
99$:    rts
metlow: ldx     #lastmeter      ; last meter state
        abx                     ; index to correct meter #
        clr     0,x             ; zero last meter state
        rts





;
;
;
;
; Wait for 1 & 2 millisecond
;
_wait2ms:       bsr     _wait1ms
_wait1ms:       ldd     #286
;
wtlp:   subd    #1
        bne     wtlp
        rts

_wait100us:
        ldd     #28
        bra     wtlp



        .end



