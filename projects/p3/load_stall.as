start   lw      0       1       five   # R1 = 5
        add     1       1       2       # R2 = R1 + R1 (needs stall)
        add     2       0       3       # R3 = R2 + 0
        halt
five    .fill   5
