	sw	5	1	five
	lw	1	2	3
start	add	1	2	1
	beq	4	1	6
	nor	3	7	2
	noop
done	halt
five	.fill	5
neg1	.fill	-1
stAddr	.fill	start
