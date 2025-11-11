	lw	0	1	n
	lw	0	2	r
	lw	0	4	Faddr	// load function address
	jalr	4	7		// call function
	halt
recur	lw	0	6	one	//load reg6 = 1
	sw	5	7	Stack	//save return address to Stack
	
	add	5	6	5	//sp++
	sw	5	1	Stack	//save n to Stack
	add	5	6	5	//sp++
	sw	5	2	Stack	//save r to Stack
	add	5	6	5	//sp++
	beq	2	0	base	//if r == 0 go to base case
	beq	1	2	base	//if n == r go to base case
	lw	0	6	neg1	//load reg6 = -1
	add	1	6	1	//n--;
	add	2	6	2	//r--;
	lw	0	4	Faddr	// load function address
	jalr	4	7		//call comb(n-1, r-1)
	lw	0	6	neg1	//load reg6 = -1
	add	5	6	5	//sp--
	lw	5	2	Stack	//restore r
	add	5	6	5	//sp--
	lw	5	1	Stack	//restore n
	add	1	6	1	//n--;
	lw	0	6	one	//load reg6 = 1
	sw	5	3	Stack	//save result of first call in reg3
	add	5	6	5	//sp++
	lw	0	4	Faddr	// load function address
	jalr	4	7		//call comb(n-1, r)
	lw	0	6	neg1	//reg6 = -1
	add	5	6	5	//sp--
	lw	5	4	Stack	//load outut to reg4
	add	3	4	3	//reg3 = reg3 + reg4
	add	5	6	5	//sp--
	lw	5	7	Stack	//restore return address
	jalr	7	4		//jump back to caller
base	lw	0	3	one	//reg3 = 1
	lw	0	6	neg1	//reg6 = -1
	add	5	6	5	//sp--
	add	5	6	5	//sp-- //pop r
	add	5	6	5	//sp-- //pop n
	lw	5	7	Stack	//restore return address
	jalr	7	6		//jump back
n	.fill	7
r	.fill	3
Faddr	.fill	recur
one	.fill	1
neg1	.fill	-1
Stack	.fill	0
