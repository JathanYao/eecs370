GLOB	sw	5	1	five
GLOB	lw	1	2	2
start	add	1	2	1
GLOB	beq	4	1	6
GLOB	nor	3	7	2
GLOB	noop
done	halt				end of program
five	.fill	5
neg1	.fill	-1
stAddr	.fill	start			will contain the address of start (2)
