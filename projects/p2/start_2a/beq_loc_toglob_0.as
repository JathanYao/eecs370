	sw	5	1	five
	lw	1	2	3
start	add	1	2	1
	beq	4	1	neg1
	nor	3	7	2
	noop
done	halt				end of program
five	.fill	4
neg1	.fill	Neg1
stAddr	.fill	start			will contain the address of start (2)
