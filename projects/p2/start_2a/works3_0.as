	sw	5	1	five
	lw	1	2	2
start	add	1	2	1
GLOB	beq	4	1	6
	nor	3	7	2
	noop
done	halt				end of program
five	.fill	5
Nine	.fill	-1
stAddr	.fill	start			will contain the address of start (2)
