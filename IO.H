/*	IO DEFINITIONS FOR MC68HC11
 *	Copyright (c) 1991 by COSMIC (France)
 */
#define _BASE	0x1000
#define _IO(x)	@_BASE+x
#if _BASE == 0
#define _PORT	@dir
#else
#define _PORT	@port
#endif

_PORT volatile	char PORTA  _IO(0x00);	/* port A */
_PORT		char PIOC   _IO(0x02);	/* parallel control */
_PORT volatile	char PORTC  _IO(0x03);	/* port C */
_PORT volatile	char PORTB  _IO(0x04);	/* port B */
_PORT volatile	char PORTCL _IO(0x05);	/* port C latched */
_PORT		char DDRC   _IO(0x07);	/* data direction port C */
_PORT volatile	char PORTD  _IO(0x08);	/* port D */
_PORT		char DDRD   _IO(0x09);	/* data direction port D */
_PORT volatile	char PORTE  _IO(0x0a);	/* port E */

_PORT		char CFORC  _IO(0x0b);	/* compare force */
_PORT		char OC1M   _IO(0x0c);	/* oc1 mask */
_PORT		char OC1D   _IO(0x0d);	/* oc1 data */
_PORT volatile	int  TCNT   _IO(0x0e);	/* timer counter */
_PORT volatile	int  TIC1   _IO(0x10);	/* timer capture 1 */
_PORT volatile	int  TIC2   _IO(0x12);	/* timer capture 2 */
_PORT volatile	int  TIC3   _IO(0x14);	/* timer capture 3 */
_PORT		int  TOC1   _IO(0x16);	/* output compare 1 */
_PORT		int  TOC2   _IO(0x18);	/* output compare 2 */
_PORT		int  TOC3   _IO(0x1a);	/* output compare 3 */
_PORT		int  TOC4   _IO(0x1c);	/* output compare 4 */
_PORT		int  TOC5   _IO(0x1e);	/* output compare 5 */
_PORT		char TCTL1  _IO(0x20);	/* timer control 1 */
_PORT		char TCTL2  _IO(0x21);	/* timer control 2 */
_PORT		char TMSK1  _IO(0x22);	/* timer interrupt mask 1 */
_PORT volatile	char TFLG1  _IO(0x23);	/* timer interrupt flag 1 */
_PORT		char TMSK2  _IO(0x24);	/* timer interrupt mask 2 */
_PORT volatile	char TFLG2  _IO(0x25);	/* timer interrupt flag 2 */

_PORT		char PACTL  _IO(0x26);	/* pulse accumulator control */
_PORT		char PACNT  _IO(0x27);	/* pulse accumulator count */

_PORT		char SPCR   _IO(0x28);	/* SPI control register */
_PORT volatile	char SPSR   _IO(0x29);	/* SPI status register */
_PORT volatile	char SPDR   _IO(0x2a);	/* SPI data register */

_PORT		char BAUD   _IO(0x2b);	/* SCI baud rate */
_PORT		char SCCR1  _IO(0x2c);	/* SCI control register 1 */
_PORT		char SCCR2  _IO(0x2d);	/* SCI control register 2 */
_PORT volatile	char SCSR   _IO(0x2e);	/* SCI status register */
_PORT volatile	char SCDR   _IO(0x2f);	/* SCI data register */

_PORT volatile	char ADCTL  _IO(0x30);	/* A/D control register */
_PORT volatile	char ADR1   _IO(0x31);	/* A/D result 1 */
_PORT volatile	char ADR2   _IO(0x32);	/* A/D result 2 */
_PORT volatile	char ADR3   _IO(0x33);	/* A/D result 3 */
_PORT volatile	char ADR4   _IO(0x34);	/* A/D result 4 */

_PORT		char BPROT  _IO(0x35);	/* block protect */
_PORT		char PORTG  _IO(0x36);	/* port G */
_PORT		char DDRG   _IO(0x37);	/* data direction port G */
_PORT		char OPTION _IO(0x39);	/* system config options */
_PORT volatile	char COPRST _IO(0x3a);	/* COP arm/reset */
_PORT volatile	char PPROG  _IO(0x3b);	/* EEPROM control register */
_PORT		char HPRIO  _IO(0x3c);	/* highest priority register */
_PORT		char INIT   _IO(0x3d);	/* RAM-IO mapping register */
_PORT		char TEST1  _IO(0x3e);	/* factory test control */
_PORT		char CONFIG _IO(0x3f);	/* (EEPROM) config register */


#define EDG1A   0x10    /* TCTL2 - IC1 rising edge capture  */
#define EDG2A   0x04    /* TCTL2 - IC2 rising edge capture  */
#define EDG3A   0x01    /* TCTL2 - IC3 rising edge capture  */
#define IC1I    0x04    /* TMSK1 - IC1 interrupt enable     */
#define IC2I    0x02    /* TMSK1 - IC2 interrupt enable     */
#define IC3I    0x01    /* TMSK1 - IC3 interrupt enable     */
