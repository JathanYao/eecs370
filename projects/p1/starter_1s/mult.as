	lw	0	1	mcand	//load mcand to reg1
	lw	0	2	mplier	//load mplier to reg2
	lw	0	3	ans	//reg3 is the answer / product
	lw	0	4	shift	//use for shifting nor mask one bit to the left
	lw	0	5	compl	//nor of compl isolates bit -> if LSB is 1, reg7 = 0, if LSB is 0, reg7 = 1
	lw	0	6	last	//used to check if shift has reached the last bit or not
	lw	0	7	lsb	//current lsb of mcand
loop	beq	4	6	end	//if shift == last, we are done checking all bits
	nor	1	5	7	//load the LSB of mcand to reg7
	beq	0	7	one	//
zero	add	2	2	2 	//mplier *= 2
	add	4	5	5	//shifts bit mask one bit left (-2 -> -3 -> -5 -> -9 -> -17 etc.)
	add	4	4	4	//doubles shift for next time
	beq	0	0	loop	// go back to beginning
one	add	2	3	3	//ans += mplier
	add	2	2	2	//shift mplier left 1 bit mplier *= 2
	add	4	5	5	//shifts bit mask one bit left (-2 -> -3 -> -5 -> -9 -> -17 etc.)
	add	4	4	4	//doubles shift for next time
	beq	0	0	loop	// go back to beginning	
end	halt
mcand	.fill	62
mplier	.fill	19
ans	.fill 	0
compl 	.fill	-2
double	.fill	1
shift	.fill 	-1
last	.fill	4294965248
lsb	.fill	1
