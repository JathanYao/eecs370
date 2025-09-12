	lw      0       1       one     reg1 = 1
        lw      0       2       two     reg2 = 2
loop    add     1       2       1       reg1 += reg2
        beq     1       2       done    if reg1 == reg2 -> done
        beq     0       0       loop    unconditional jump back
done    halt
one     .fill   1
two     .fill   2
