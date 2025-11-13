        add     0       1       0
        beq     0       0       skip   # always branch
        add     0       1       0
        add     0       0       1       # should be squashed
        add     0       0       2       # should be squashed
skip    add     0       0       3       # executed
        halt
