        beq     0       0       L1
        add     0       0       1       # squashed
L1      beq     0       0       L2
        add     0       0       2       # squashed
L2      add     0       0       3       # executed
        halt
