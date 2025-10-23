	sw	5	1	five
	lw	1	2	Stack
start	add	1	2	1
	beq	4	1	StAddr
Poop	sw	5	1	Nine
	lw	1	2	3
	add	1	2	1
	beq	4	1	6
	noop
done	halt				
	.fill	-1
	.fill	6578
nine	.fill	Stack
five	.fill	5
neg1	.fill	-1
StAddr	.fill	Nine			
