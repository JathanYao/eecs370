	add	0	1	2
start	lw	3	6	6
    add	0	1	4
    lw	0	6	start
    sw	3	6	poop
label	beq	7	7	start
poop    add	0	1	4
	noop
done	halt
 .fill poop
        .fill 7
