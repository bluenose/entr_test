.processor m68hc11
;
;	MCX interrupt vectors
;
;
        .external       rtiisr, mcxsrv, mcxinit, _rti
        .external       ic1srv, ic2srv, ic3srv, oc1srv
;
	.psect	_text

	.word scisrv	; SCI                       RIE/TIE/TCIE/ILIE */
	.word mcxinit	; SPI Serial Transfer Complete           SPIE */
	.word mcxinit	; Pulse Accumulator Input Edge           PAII */
	.word mcxinit	; Pulse Accumulator Overflow            PAOVI */
	.word mcxinit	; Timer Overflow                          TOI */
	.word mcxinit	; Input Capture 4/Output Compare 5     I4/O5I */
	.word mcxinit	; Output Compare 4                       OC4I */
	.word mcxinit	; Output Compare 3                       OC3I */
	.word mcxinit	; Output Compare 2                       OC2I */
  .word oc1srv  ; Output Compare 1                       OC1I */
  .word ic3srv  ; Input  Capture 3                       IC3I */
  .word ic2srv  ; Input  Capture 2                       IC2I */
  .word ic1srv  ; Input  Capture 1                       IC1I */
  .word rtiisr  ; Real-Time Interrupt                    RTII */
  .word _rti    ; IRQ (External Pin)                      nmi */
  .word mcxinit ; XIRQ                                    nmi */
	.word mcxsrv	; Software Interrupt (SWI)                nmi */
	.word mcxinit	; Illegal Opcode Trap                   NOCOP */
  .word mcxinit ; COP Failure                             nmi */
	.word mcxinit	; Clock Monitor Fail                      CME */
	.word mcxinit	; RESET                                   nmi */

	.end
