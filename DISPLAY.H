/************
* display.h *
*************/

typedef struct {
	BYTE dp;
	BYTE digit[4];
    BYTE led;
} DISP;


extern DISP gDisp;


#define LED1 0x40
#define LED2 0x80

#define LED_SOLD  LED1
#define LED_EXACT LED2
