        lw      0       1       val     load reg1 = 123
        sw      0       1       copy    store reg1 into mem[copy]
        lw      0       2       copy    load reg2 from mem[copy]
        halt
val     .fill   123
copy    .fill   0
