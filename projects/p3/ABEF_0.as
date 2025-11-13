	sw	5	1	five branch goes past the halt, halt never gets read -> seg fault
	lw	1	2	3
start	add	1	2	1
	beq	4	1	6
Nine	nor	3	7	2
	noop
done	halt
five	.fill	5
neg1	.fill	-1
StAddr	.fill	start
