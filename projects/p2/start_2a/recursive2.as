	lw	0	1	n
	lw	0	2	r
	lw	0	4	Faddr	#load function address
	jalr	4	7		#call function
	halt
recur	lw	0	6	one
	sw	5	7	Stack	#save return address on stack
	add	5	6	5
	sw	5	1	Stack	# save n
	add	5	6	5
	sw	5	2	Stack	# save r
	add	5	6	5	#done saving
	beq	1	2	base
	beq	2	0	base	#check base casees n =r, r=0
	lw	0	6	neg1	#call function w jalr r3=comb(n-1,r)
	add	1	6	1	//n--;
	lw	0	4	Faddr
	jalr	4	7
	lw	0	6	neg1
	add	5	6	5
	sw	5	2	Stack	# recover r
	add	5	6	5
	sw	5	1	Stack	# recover n
	add	2	6	2	#call function w jalr r3=comb(n-1,r-1)
	add	1	6	1
	lw	0	6	one
	lw	5	3	Stack	#put r3 on stack
	add	5	6	5
	lw	0	4	Faddr
	jalr	4	7		# does random shit..
	lw	0	6	neg1
	add	5	6	5
	lw	5	4	Stack	#return 3 from the stack
	add	3	4	3	#caclulate com1 + com2
	add	5	6	5
	lw	5	7	Stack
	jalr	7	4
base	lw	0	3	one	this is definitly wrong
	lw	0	6	neg1
	add	5	6	5
	add	5	6	5
	add	5	6	5	#decrement by 3
	lw	5	7	Stack	#load return value
	jalr	7	6		# return 3
n	.fill	7
r	.fill	3
Faddr	.fill	recur
one	.fill	1
neg1	.fill	-1
Stack	.fill	0
