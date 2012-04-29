; mcxdata.s

        .INCLUDE "DEFINE.S"
        .INCLUDE "MCXDEF.S"

        .PUBLIC   FLGTBL, QHDRTBL, STATLS, TIMERS
        .PUBLIC   tickcnt, FREE, ACTIVE, curtsk, curtcb, hipri
        .PUBLIC   pritcb, intlvl, temp, _width, _depth, notmt
        .PUBLIC   SYSTACK, TCBDATA, QUEDATA, STKBASE
        .PUBLIC   _TCBDATA

        .EXTERNAL clkdrv
        .EXTERNAL _task_dev
        .EXTERNAL _task_maintenance
        .EXTERNAL _task_keypad
        .EXTERNAL _task_mdb
        .EXTERNAL _task_main
        .EXTERNAL _task_initmdb
        .EXTERNAL _task_pollmdb
        .EXTERNAL _task_display
        .EXTERNAL _task_periodic
        .EXTERNAL _task_vend1
        .EXTERNAL _task_vend2


;************************
;  Stack Areas
;************************
        .PSECT _bss
        .BYTE [64]
SYSTACK: .BYTE [19]
        .PSECT _bss
        .BYTE [18]
STAK1: 
        .PSECT _data
        .BYTE [96]
STAK2: 
        .PSECT _data
        .BYTE [72]
STAK3: 
        .PSECT _data
        .BYTE [100]
STAK4: 
        .PSECT _data
        .BYTE [72]
STAK5: 
        .PSECT _data
        .BYTE [224]
STAK6: 
        .PSECT _data
        .BYTE [100]
STAK7: 
        .PSECT _data
        .BYTE [136]
STAK8: 
        .PSECT _data
        .BYTE [64]
STAK9: 
        .PSECT _data
        .BYTE [104]
STAK10:
        .PSECT _data
        .BYTE [120]
STAK11:
        .PSECT _data
        .BYTE [120]
STAK12:
STKBASE:

        .PSECT _bss

;************************
;  Queue Headers
;************************
QHDRTBL:
Q1HDR:  .BYTE [QHDRLEN]

;************************
;  Timers
;************************
TIMERS: .BYTE [NTIMERS*TIMRLEN]


;************************
;  System Variables
;************************
MCXVAR:
tickcnt: .BYTE [1]      ; Tick counter
FREE:   .WORD [1]       ; Address of first free timer block
ACTIVE: .WORD [1]       ; Address of first active timer in list
curtsk: .BYTE [1]       ; Current task (i.e. the active task)
curtcb: .WORD [1]       ; Address of current task's TCB
hipri:  .BYTE [1]       ; Highest priority task ready to run
pritcb: .WORD [1]       ; Address of TCB of highest priority task
intlvl: .BYTE [1]       ; Depth of nested interrupts
temp:   .WORD [1]       ; Temporary area
_width: .BYTE [1]       ; Work area for queue width
_depth: .BYTE [1]       ; Work area for queue depth
notmt:  .BYTE [1]       ; Work area for queue not empty semaphore

;************************
;  Semaphore table
;************************
FLGTBL: .BYTE [NQUEUES*2 + NTASKS + NNAMSEM]

;************************
;  Task Control Block
;************************
STATLS:
TASK1:  .BYTE [TCBLEN]
TASK2:  .BYTE [TCBLEN]
TASK3:  .BYTE [TCBLEN]
TASK4:  .BYTE [TCBLEN]
TASK5:  .BYTE [TCBLEN]
TASK6:  .BYTE [TCBLEN]
TASK7:  .BYTE [TCBLEN]
TASK8:  .BYTE [TCBLEN]
TASK9:  .BYTE [TCBLEN]
TASK10: .BYTE [TCBLEN]
TASK11: .BYTE [TCBLEN]
TASK12: .BYTE [TCBLEN]

;************************
;  Queue Bodies
;************************
Q1BODY: .BYTE [1*8]


        .PSECT _text

;************************
;  Queue data in ROM
;************************
QUEDATA:
        .BYTE   1,8
        .WORD   Q1HDR,Q1BODY


;************************
;  TCB data in ROM
;************************
TCBDATA:
_TCBDATA:
        .BYTE   _RUN
        .WORD   clkdrv, STAK1-1, TASK1

        .BYTE   _IDLE
        .WORD   _task_dev, STAK2-1, TASK2

        .BYTE   _IDLE
        .WORD   _task_maintenance, STAK3-1, TASK3

        .BYTE   _IDLE
        .WORD   _task_keypad, STAK4-1, TASK4

        .BYTE   _IDLE
        .WORD   _task_mdb, STAK5-1, TASK5

        .BYTE   _RUN
        .WORD   _task_main, STAK6-1, TASK6

        .BYTE   _IDLE
        .WORD   _task_initmdb, STAK7-1, TASK7

        .BYTE   _IDLE
        .WORD   _task_pollmdb, STAK8-1, TASK8

        .BYTE   _IDLE
        .WORD   _task_display, STAK9-1, TASK9

        .BYTE   _IDLE
        .WORD   _task_periodic, STAK10-1, TASK10

        .BYTE   _IDLE
        .WORD   _task_vend1, STAK11-1, TASK11

        .BYTE   _IDLE
        .WORD   _task_vend2, STAK12-1, TASK12


        .END
