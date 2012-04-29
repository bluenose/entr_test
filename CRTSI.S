.processor m68hc11
;
;	C STARTUP FOR MC68HC11
;	WITH AUTOMATIC DATA INITIALISATION
;	COPYRIGHT (c) 1988 by COSMIC
;
        .external       mcxgo, __memory, __pdata
        .external       _progvers, _eepversion
        .public         __stext, _copduty
        .public         __return, _new_version
;
;          *** Register definitions
        .define PORTA   = 1000h         ; Port A
        .define PACTL   = 1026h         ; Data direction
        .define OPTION  = 1039h         ; OPTION register
        .define COPRST  = 103Ah         ; COPRST register
        .define BPROT   = 1035h         ; BPROT register
        .define INIT    = 103Dh         ; INIT register
;
        .define MINOR   = 1             ; Minor revision
        .define MAJOR   = 2             ; Major revision
;
	.psect	_bss
__sbss:
newrev:         .word   0
prec:           .word   0
lastp:          .word   0
_new_version:   .byte   0

	.psect	_text
__stext:
        ldaa    #13h            ; Slow COP
        staa    OPTION          ; Write to OPTION register
        ldaa    #21h            ; RAM mapped to 2000 & Registers mapped to 1000
        staa    INIT            ; Write to INIT register
        ldaa    #0              ; zero
        staa    BPROT           ; Un-protect all EEPROM space
        ldd     #55AAh          ; Values to arm/reset COP
        ldx     #0              ; loop 65536 times
wait:   staa    COPRST          ; Arm COP
        stab    COPRST          ; Reset COP
        dex
        bne     wait
        ;--------
        ; Clear NVRAM           /*** TAKE-OUT ***/
;        ldx     #07FFh
;clrnv:  clr     0,x
;        ldd     #55AAh          ; Values to arm/reset COP
;        staa    COPRST          ; Arm COP
;        stab    COPRST          ; Reset COP
;        dex
;        cpx     #0040h
;        bne     clrnv
;        ; Clear MCU RAM
;        ldx     #21FFh
;clrsr:  clr     0,x
;        ldd     #55AAh          ; Values to arm/reset COP
;        staa    COPRST          ; Arm COP
;        stab    COPRST          ; Reset COP
;        dex
;        cpx     #2000h
;        bne     clrsr
;        ; Loop forever
;lp4evr: staa    COPRST          ; Arm COP
;        stab    COPRST          ; Reset COP
;        bra     lp4evr
        ;--------
        clra
        staa    _new_version    ; Clear new version flag
        ldd     _progvers       ; load new program version #
        cpd     _eepversion     ; compare with current version # in E2
        beq     rev_ok          ; No new revision detected
        ;--------
        bra     maj             ; Any change is a major rev. (dead code below)
        ;--------
maj:    ldaa    #MAJOR
        staa    _new_version    ; Set major rev flag
        ;--------
        ldx     #07FFh
clrnv:  clr     0,x
        ldd     #55AAh          ; Values to arm/reset COP
        staa    COPRST          ; Arm COP
        stab    COPRST          ; Reset COP
        dex
        cpx     #0040h
        bne     clrnv
;-------------------------------
rev_ok: ldx     #__pdata        ; descriptor address
ploop:  stx     prec            ; save pdata record address
        ldy     3,x             ; * accy = load ram data address
        ldaa    2,x             ; load test flag byte
        beq     zbss            ; no more segment
        ldd     5,x             ; load last prom data address
        std     lastp           ; save last prom data address
        ldaa    2,x             ; * acca = load test flag byte
        ldx     0,x             ; * accx = load rom data address
        ;--------
        cmpa    #'E'            ; EEPROM data?
        beq     noinit          ; yes, do not init.
        cmpa    #'B'            ; BATTERY Backed ram?
        bne     init            ; Not battery backed, init memory
        ldd     _progvers       ; load program new version #
        beq     init            ; if version==0 then always init bram
        ldaa    _new_version    ; load new version flags
        cmpa    #MAJOR          ; Major revision?
        bne     noinit          ; No major rev. Do not init bram.
        ;--------
init:   nop
copy:   ldab    0,x             ; copy from prom
        stab    0,y             ;   to ram
        ;--------
        ldd     #55AAh          ; Values to arm/reset COP
        staa    COPRST          ; Arm COP
        stab    COPRST          ; Reset COP
        ;--------
        inx                     ; inc to next byte
        iny                     ; inc to next byte
        cpx     lastp           ; last data from rom?
        bne     copy            ; no, loop again
        ;--------
noinit: ldx     prec            ; load last pdata record address
        ldab    #5              ; size of pdata record
        abx                     ; point to next pdata record
        bra     ploop           ; and loop
;-------------------------------
zbss:   ldab    _new_version    ; save new version flag
        clra                    ; zero
        ldx     #__sbss         ; start of bss
        bra     loop            ; start loop
zbcl:   staa    0,x             ; clear byte
        inx                     ; next byte
loop:   cpx     #__memory       ; up to the end
        bne     zbcl            ; and loop
        stab    _new_version    ; restore new version flag
        ;--------
        jmp     mcxgo           ; execute MCX
        ;--------
__return:
	rti			; Return from undefined interrupt
;
;
;
;
_copduty :
        ldd    #55AAh           ; Values to arm/reset COP
        staa   COPRST           ; Arm COP
        stab   COPRST           ; Reset COP
        rts
;
;
;
;
	.psect _data
__sdata:
	.word	0		; avoid any pointer to be null

        .end
