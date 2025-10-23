	sw	5	1	five
	lw	1	2	3
start	add	1	2	1
	beq	4	1	StAddr
	sw	5	1	StAdd
	lw	1	2	3
	add	1	2	1
	beq	4	1	6
	noop
done	halt				
	.fill	-1
	.fill	6578
nine	.fill	Big
five	.fill	5
neg1	.fill	-1
StAddr	.fill	nine			
