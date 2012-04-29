.processor m68hc11
;
;
	.external	__memory
	.public		_stackcheck
	.public		_unused
	.public		_copreset
        .public         _setconfig, _eewr, _wait1ms, _wait2ms;
;
;
.define TMSK2 =  1024h		; timer prescaler reg
.define COPRST = 103Ah		; COP watchdog reset register
.define PPROG =  103Bh		; EEPROM programming control
.define CONFIG = 103Fh		; MCU configuration register
;
.define EELOW  = 0B600h
.define EEHIGH = 0B7FFh
;
;
;
	.psect	_text
;
;
;
;
; Return # of bytes left for stack
;
_stackcheck:
	pshx
	ldx	#__memory	; setup ix to point to un-init memory
agn:	ldaa	0,x		; get byte
	inx			; inc pointer
	inx			; inc pointer
	inx			; inc pointer
	inx			; inc pointer
	cmpa	#0FFh		; did stack overwrite this?
	beq	agn		; no, try next byte
	xgdx			; yes, load iy into accd
	subd	#__memory+4	; change pointer to offset
	pulx
	std	_unused
	rts
;
;
_copreset:
	psha
	ldaa	#55h
	staa	COPRST
	ldaa	#0AAh
	staa	COPRST
	pula
	rts
;
;
;
; Write MCU configuration register with value in acc.B
;
_setconfig:
	ldx	#CONFIG
	tba
	cmpa	CONFIG
	bne	_eewr
	rts


;=======================================================================;
; 	68HC11 onchip eeprom write acc.A to [ind.X] routine		;
;-----------------------------------------------------------------------;
;Write one byte to on-chip EEPROM memory.  There is no corresponding
;utility for reading from EEPROM because it is read as regular memory.
;If the write address is not in the range of eeprom memory, just write data.
;Data to write is in acc.A, returns unaltered.
;Write address is in ind.X, returns unaltered.
;No write to eeprom if the data is the same.
;Erase is done only if necessary.
;
_eewr:	pshb				;save acc.B
	psha				;save data byte
	cpx	#CONFIG			;eeprom configuration register
	beq	ebw			;(erase before write config to eeprom)
	cpx	#EELOW			;lowest eeprom address
	blo	wram
	cpx	#EEHIGH			;highest eeprom address
	bls	wree			;address is eeprom
wram:	staa	0,x			;not an eeprom address, just write it
	bra	e4			;done with write
	;--------
wree:	cmpa	0,x			;compare data and eeprom
	beq	e4			;data is the same, no write
	;- - - -
	;a write will be done
;	psha				;save data byte
	tab				;put data in acc.B
	andb	0,x			;look for bits that must be prog.hi
	cba
	beq	e2			;all ones in new data are ones in old
	;- - - -
	;erase must be done before write
ebw:	ldab	#16h			;set register for erase
	stab	PPROG ;+IOb
	stab	0,x			;dummy data for addressing
	ldab	#17h			;turn on high voltage
	stab	PPROG ;+IOb

	ldd	#2858			;set delay approx. 10msec
e1:
  bsr _copreset
	subd	#1			;subd=2us bne=1.5us
	bne	e1			;branch=not count down

	clr	PPROG ;+IOb
	;- - - -
	;do the eeprom write
e2:	ldab	#02h			;set register for write
	stab	PPROG ;+IOb
	pula				;get data byte
	psha
	staa	0,x			;address with data
	ldab	#03h			;set register for write
	stab	PPROG ;+IOb
	ldd	#2858			;set delay approx. 10msec
e3:
  bsr _copreset
	subd	#1			;subtract 3usec
	bne	e3			;branch=not count down
	clr	PPROG ;+IOb		;clear config register
	;- - - -
e4:	pula				;get back data byte
	pulb				;get back acc.B
	rts				;exit from 'eewr', write to eeprom
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
;
;
;
		.psect _data
_unused:	.word 0

.end
