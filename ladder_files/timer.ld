LDmicro0.1
MICRO=ANSI C Code
CYCLE=50000
CRYSTAL=4000000
BAUD=2400

IO LIST
    X1_0 at 2
    X1_1 at 3
    Y1_0 at 0
END

PROGRAM
RUNG
    PARALLEL
        SERIES
            CONTACTS X1_0 0
            OSR
            TOF Timer1 1000000
        END
        SERIES
            CONTACTS X1_1 0
            OSR
            TOF Timer2 100000
        END
    END
    COIL Y1_0 0 0 0
END
