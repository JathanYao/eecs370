start   lw      0       1       five   # R1 = 5
        add     1       0       2       # needs stall
        add     2       0       3       # needs forwarding
        nor     3       0       4       # needs forwarding
        halt
five    .fill   5
