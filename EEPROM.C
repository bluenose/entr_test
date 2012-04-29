/*********************************
* eeprom.c - data
*********************************/
#include "hdrs.h"

@eeprom WORD gMaxPrice;
@eeprom WORD gPrice[4];
@eeprom WORD eepversion;
@eeprom BYTE gSNset;

/****************************************
* Check version number in EEPROM, and
* init EEPROM if it is different.
*****************************************/
void eeprom_init()
{
  int i;

  if (eepversion != progvers)
  {
    eepversion = progvers;    // Set to new version
    gPrice[GAL1]  = 25;
    gPrice[GAL3]  = 75;
    gPrice[GAL5]  = 100;
    gPrice[TOKEN] = 25;
    gMaxPrice = 500;
    gOption[OP_NOCHEAT] = TRUE;

    #if DEBUG_TEST
      gExitType[COUNTER_EXIT] = 0;
      gExitType[TIMER_EXIT] = 0;
      gExitType[DIRTY_EXIT] = 0;
      gExitType[NOPLS_EXIT] = 0;
    #endif
  }
}
