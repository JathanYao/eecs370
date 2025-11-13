start   lw      0       1       a
        lw      0       2       b
        add     1       2       3       # needs forwarding from MEM stage
        beq     2       1       skip
        add     1       1       1       # squashed
        nor     2       2       2       # squashed
        add     3       0       4       # squashed
skip    nor     3       1       5       # executed
        halt
a       .fill   6
b       .fill   6
