	sw	5	1	five
	lw	1	2	3
start	add	1	2	1
	beq	4	1	StAddr
	sw	5	1	3
	lw	1	2	3
StAddr	add	1	2	1
	beq	4	1	6
	noop
done	halt				
	.fill	-1
	.fill	6578
nine	.fill	-6578
five	.fill	5
Nine	.fill	-1
	.fill	nine			
