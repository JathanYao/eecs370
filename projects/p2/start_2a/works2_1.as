	sw	5	1	Stack
	lw	1	2	3
start	add	1	2	1
	beq	4	1	Nine
	sw	5	1	StAddr
	lw	1	2	3
Nine	add	1	2	1
	beq	4	1	6
	noop
done	halt				
	.fill	StAddr
	.fill	6578
Meep	.fill	Nine
five	.fill	5
neg1	.fill	-1
lofd	.fill	Stack	
