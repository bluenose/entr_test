AS     = x6811 -l -s -n -g
LINK   = lnkh11 < mcx.lnk
TOPROM = toprom -o mcx.h11 xeq.h11
TOMOT  = hexh11 -s -o ep.mot mcx.h11
TOHEX  = hexh11    -o ep.hex mcx.h11
CC     = c -dlistcs -dxdebug +o
GREP = grep "error "
DEL  = DEL

mcx.h11 :       mcx.lnk mcxdata.o mcxinit.o mcx.o mcxclock.o mcxnull.o mcxc.o \
		crtsi.o \
		isrs.o \
		vector.o \
		main.o \
                mis.o \
		rsrc.o \
		dbug.o \
                keypad.o \
                display.o \
                mdb.o \
                coin.o \
                bill.o \
                dex.o \
                sci.o \
                dev.o \
                bram.o \
                eeprom.o \
                globals.o \
                util.o \
                vend.o \
                menudef.o \
                menucode.o \
                version.o
        $(LINK)
        $(TOPROM)
        $(TOMOT)
        $(TOHEX)

mcx.o :		mcx.s define.s mcxdef.s
	$(AS)   mcx.s

mcxdata.o :	mcxdata.s define.s mcxdef.s
	$(AS)   mcxdata.s

mcxinit.o :	mcxinit.s define.s mcxdef.s
	$(AS)   mcxinit.s

mcxclock.o :	mcxclock.s define.s mcxdef.s
	$(AS)   mcxclock.s

mcxnull.o :	mcxnull.s define.s mcxdef.s
	$(AS)   mcxnull.s

mcxc.o :        mcxc.s
        $(AS)   mcxc.s

crtsi.o :	crtsi.s
	$(AS)   crtsi.s

vector.o :	vector.s
	$(AS)   vector.s

isrs.o :	isrs.s
	$(AS)   isrs.s

version.o :     version.s
        $(AS)   version.s

dbug.o :	 dbug.c mcxdbug.h mcxdef.h
        @$(CC)   dbug.c

main.o :         main.c mcxdef.h globals.h hdrs.h
        @$(CC)   main.c

mis.o :          mis.c mcxdef.h globals.h hdrs.h mis.h
        @$(CC)   mis.c

util.o :         util.c util.h
        @$(CC)   util.c

vend.o :         vend.c vend.h dev.h
        @$(CC)   vend.c

menudef.o :      menudef.c menu.h
        @$(CC)   menudef.c

menucode.o :     menucode.c menu.h
        @$(CC)   menucode.c

keypad.o :       keypad.c dev.h
        @$(CC)   keypad.c

display.o :      display.c  display.h
        @$(CC)   display.c

sci.o  :         sci.c  sci.h
        @$(CC)   sci.c

dev.o  :         dev.c  dev.h
        @$(CC)   dev.c

mdb.o  :         mdb.c  mdb.h
        @$(CC)   mdb.c

coin.o  :        coin.c  coin.h
        @$(CC)   coin.c

bill.o  :        bill.c  bill.h
        @$(CC)   bill.c

dex.o  :         dex.c  dex.h
        @$(CC)   dex.c

rsrc.o  :        rsrc.c
        @$(CC)   rsrc.c

globals.o :      globals.c globals.h
        @$(CC)   -dnobss globals.c

bram.o :         bram.c  bram.h mis.h coin.h
        @$(CC)   -dnobss bram.c

eeprom.o :       eeprom.c  eeprom.h
        @$(CC)   -dnobss eeprom.c
