/********
* dev.h *
*********/


/****************************
* Version control variables *
*****************************/
#define MINOR 1
#define MAJOR 2
extern WORD progvers;                   /* EPROM version    */
extern BYTE new_version;                /* New version flag */


/********************************
* Motor & selection definitions *
*********************************/
#define SELMAX  4
#define MTR_OFF SELMAX

#define OFF   0
#define ON    1
#define MASK  2

/*********************
* Port E definitions *
**********************/
#define MODESW   0x02
#define MTR_SO   0x04
#define MTR_HOME 0x08
#define DEXSW    0x20


/*********************
* Port A definitions *
**********************/
#define DOORSW   0x04


/*********************
* Port D definitions *
**********************/
#define Refrig_bit  0x20


/******************
* A/D Definitions *
*******************/
#define AD_RTD  0
#define AD_SO   2
#define AD_MTR  4



/**************************************
* Serial number constants & addresses *
***************************************/
#define SN_CODE     0x07FA  // 2 bytes
#define SN_DATA     0x07FC  // 4 bytes
#define SN_UNSET    0x55A0
#define SN_SET      0xAA50


#define TM_PERIODIC TM_8ms /* periodic timer */


#define ADDR_DISPLAY 0x20   /* 5 byte write */
#define ADDR_ICLK    0x50   /* I clock      */
#define ADDR_LOAD    0x60   /* Latch inputs */
#define ADDR_KEYPAD  0x70   /* 2 byte read  */
#define ADDR_OUTP    0x30   /* 2 byte write */
#define ADDR_EEPROM  0x40


#define OUTP_ENB 0x80

enum {
  DEV_KEYPAD,     /* keypad driver */
  DEV_DISPLAY,    /* 4 digit LED display */
  DEV_MOTOR,      /* motor */
  DEV_SCI_SEL,    /* select DEX or MDB */
  DEV_OUTP,
  DEV_MAX
};


/***********************************
* General purpose output addresses *
************************************/
#define OUTP1    0x0040
#define OUTP2    0x0080
#define OUTP3    0x0020
#define OUTP4    0x0010
#define OUTP5    0x0001
#define OUTP6    0x0002
#define OUTP7    0x0004
#define OUTP8    0x0008
#define OUTP9    0x8000
#define OUTP10   0x4000
#define OUTP11   0x2000
#define OUTP12   0x1000
#define OUTP13   0x0100
#define OUTP14   0x0200
#define OUTP15   0x0400
#define OUTP16   0x0800

/*********************
* Output definitions *
**********************/
#define G1_LED1     OUTP1
#define G5_LED1     OUTP2
#define G1_LED2     OUTP3
#define G5_LED2     OUTP4
#define G3_LED1     OUTP12    // Version 1.24
#define G3_LED2     OUTP10    // Version 1.24
#define FMETERCNT1  OUTP5     /* gallon counter about 1300 pulses per gal */
#define FMETERCNT2  OUTP6     /* gallon counter   */
#define VALVE1      OUTP7     /* pinch valves     */
#define VALVE2      OUTP8     /* pinch valves     */
#define EXTEND1     OUTP9     /* shutter extend   */
#define EXTEND2     OUTP11
#define FMETERPWR1  OUTP13    /* Flow meter Power supply */
#define FMETERPWR2  OUTP14
#define RO_ON       OUTP15
#define RO_FLUSH    OUTP16

/*********************
* Switch definitions *
**********************/
#define SW_RO_PRESSURE    0     // J7 pin 9
#define SW_UV_DETECT_SEC  1     // J7 pin 10
#define SW_UV_DETECT_PRI  2     // J7 pin 11
#define SW_TDS_MONITOR    3     // J7 pin 12
#define SW_PRE_TREATMENT  4     // J7 pin 13
#define SW_EXTEND1        5     // J7 pin 14
#define SW_POST_TREATMENT 6     // J7 pin 15
#define SW_EXTEND2        7     // J7 pin 16    !note: J7 pin 17 and 18 don't do anything!
#define SW_MAX            8

/*********************
* Port E definitions *
**********************/
#define PORTE_SLAVE   0x01
#define PORTE_MPULSE1 0x08
#define PORTE_MPULSE2 0x04


/***************************************************
* Generic message structure for spi devices
****************************************************/
typedef struct {
  BYTE devtype;
  BYTE data;
} DEVMSG_GENERIC;

typedef struct {
  BYTE devtype;
  BYTE digit[4];
  BYTE led;
} DEVMSG_DISPLAY;

typedef struct {
  BYTE devtype;
  BYTE ctrl;
  WORD mask;
  WORD select;
} DEVMSG_OUTP;



/***************************************************
* Home message structure for spi devices
****************************************************/
typedef struct {
  BYTE devtype;
  WORD data;
} DEVMSG_KEYPAD;
