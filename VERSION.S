; version.s
;
; All comments should reside here
;
; Vers     Date    ID   Description
; ----  ---------  --   --------------------------------------------------------
; 0.01  29-Mar-96  bc   First draft.
; 1.00             bc   First release.
; 1.01  08-May-96  bc   Reversed a few switches.
; 1.03  10-May-96  bc   Gallon ticks are dependent on window (task).
; 1.05  14-May-96  bc   Counter interrupts enabled only when needed.
; 1.06  15-May-96  bc   Reversed sense on Master to Slave "OK to vend" signal.
; 1.07  16-May-96  bc   Slowed down SPI.
; 1.08  16-May-96  bc   Exit vend immediately if water is dirty.
; 1.11  07-Aug-96  bc   Dex mechinism used to xfer user readable data.
; 1.12  18-Sep-96  bc   Update for Pre and Post treatment switches.
; 1.13  18-Sep-96  bc   Remove double gallon count line in dex.
; 1.14  97.04.08   bc   Debounced SW_POST_TREATMENT for 5 seconds in
;                       the task_maintenance routine. (970408)
; 1.15  97.04.21   bc   Wait 2 mins before looking at SW_POST_TREATMENT. (970421)
; 1.19  98.02.19   bjsc Added code to that accepts token bill and coins
; 1.22  98.07.08   bjsc Added code that does not allow the bill validator to accept
;                       any more bills when a token is already present.
;                       Please note the over-complicated code that is added when
;                       tokens are used.  Five seperate one-gallon vends are allowed off
;                       of one token; or one five-gallon vend.
; 1.24  98.09.22   bjsc 3 gallon vend added.  2 inputs for 3 gallon switches were added.
;                       2 outputs for 3 gallon LEDS were added.  Other changes include:
;                       - bill tokens were taken out and replaced with coin tokens
;                       - coin tokens are given a credit value instead of a vend value
; 1.25  98.10.10   bjsc Bug fixed in Reverse Osmosis plant.  If the pressure comes up
;                       in the alotted time then don't sell out.
; 1.26  98.11.01   bjsc Clear credit in five minutes if no activity added; was 90 seconds.
;                       Delay automatic credit return by 5 seconds after a vend completes.
;                       See main.c.
; 1.27  98.12.17   bjsc Moved bramvers from BRAM to E2.
;
; 1.28  98.12.22   bjsc Changed flow counts in bram.
;
; 1.29  99.02.12   bjsc Eliminated 2 second flush after RO plant is complete - see 'vend.c'.
;                       Moved MIS from BRAM to E2.
;
; 1.31  99.04.13   bjsc Moved MIS back to BRAM.
;                       Mcxgen changed.  Added a timer.
;
; 1.32  99.06.03   bjsc Change was always being returned even if credit was
;                       added after a vend.  Changed code to disable the returning
;                       of credit if more credit is added.
;
; 1.33  99.06.30   bjsc CPO taken out of service mode.
; 1.33a 99.07.01   bjsc Added a timing deley for the external relay vend counter.
;
; 1.33b 99.07.16   bjsc DEX locking after a download and trashing MIS / BRAM!!
;                       See fix in main loop - (end of main).
;
; 1.33c 99.11.16   bjsc Changed RO_PLANT shut off delay from 120 secs to 20 secs.
;                       See 'vend.c' - maitenance task.
;
; 1.34  99.11.30   bjsc Changed to version 1.34 for release.
;
; 1.35  00.02.12   bjsc Serial number was not being set properly.  The NVRAM was not
;                       being cleared so code saw that the SN was already set.
;
; 1.36  00.02.21   bjsc Added extra criteria to the vend routine.  The pulses must be changing (at least
;                       once every 5 seconds) to continue to vend.
;                       Service Mode option added.  Able to change the value of a token.
;
; 1.37 00.03.27    bjsc Token prices were not being used when token was accepted.
;                       Vend Failure was not being flagged if water meter was not receiving sufficient
;                       pulses that a vend was ended.
;
; 1.38 00.07.21    bjsc Added debugging "ERRS" to the service mode to help debug a problem in the field.
;                       When vending a 5 gallon selection, intermittently the controller will only vend
;                       4 gallons!!!
;
; 1.39 02.10.01    bjsc Nozzle switches not properly implemented.
;                       Add attempt at 'noise proofing' the software by turning off the
;                       drives when not vending. 
;
; 1.40 02.11.02    bjsc Added 'Shake Time' as a menu item.
;                       Added password protection stuff for MIS in service mode.
;
; 1.41 05.08.08    bjsc Paying out after a token vend.
;
; 1.42 2008.12.08  bjsc	Fix put in place for a cheat where the customer stops the vend counter (plugs nozzle) at the end of the vend.
;                       If the counter only has a few counts left assume the vend was good.
;
; 1.43 2011.09.07  bjsc Mod to 1.42, only assume a failed vend and return change if less than 1/4 of vend is complete.
;

                .public _progvers
                .psect  _text

_progvers:      .word   0143h           ; **** VERSION NUMBER ****

                .end
