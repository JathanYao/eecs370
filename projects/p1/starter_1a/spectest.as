    add     1   5   6
    nor     4   2   3
    lw      0   1   32767
    sw      0   1   -32768
    beq     1   1   -2
    jalr    2   3   
    halt
    noop
