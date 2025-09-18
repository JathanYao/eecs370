// Check if last bit of mplier is 1, if so 
	lw	0	1	mcand	//load mcand to reg1
	lw	0	2	mplier	//load mplier to reg2
	lw	0	3	0	//clear reg3 for product
	lw	0	4	one	//load constant 1 to reg4
	beq	2	0	end	//if mplier is 0, end
loop	beq	2	4	skip	//if mplier is even,

end	halt
mcand	.fill	6203
mplier	.fill	1429
