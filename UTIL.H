/************************************
* util.h - general utility header
*************************************/

typedef struct {
  BYTE data;
  BYTE ok;
  BYTE newval;
  BYTE oldval;
  BYTE count;
  BYTE max;
} DEBOUNCE;



typedef struct {
  WORD data;
  BYTE ok;
  WORD newval;
  WORD oldval;
  BYTE count;
  BYTE max;
} DEBOUNCE2;


/*************
* Prototypes *
**************/
void debounce_init(DEBOUNCE *deb, BYTE val, BYTE max);
BYTE debounce(DEBOUNCE *deb, BYTE newval);
void debounce2_init(DEBOUNCE2 *deb, WORD val, BYTE max);
WORD debounce2(DEBOUNCE2 *deb, WORD newval);
void disp_btn(WORD num);
char *strdprice(DWORD num);
BYTE GetPassword();
