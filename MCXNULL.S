        .INCLUDE "DEFINE.S"
        .INCLUDE "MCXDEF.S"

        .PUBLIC  nulltsk, _copduty

        .PSECT  _text

nulltsk :
        ldd    #55AAh           ; Values to arm/reset COP
        staa   COPRST           ; Arm COP
        stab   COPRST           ; Reset COP

        bra     nulltsk         ; This is the null task. It is always runnable
                                ; and always interruptable. It does nothing
                                ; except provide an active task for the
                                ; Dispatcher when all other tasks are busy.
                                ; You may change this any way you wish as long
                                ; it is never put into a non-ready state.
        .END
