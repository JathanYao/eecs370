	lw	0	1	five
	lw	0	4	SubAdr
start	jalr	4	7
	beq	0	1	done
	beq	0	0	start
	beq	0	1	done
	beq	0	0	start
	beq	0	1	done
	beq	0	0	start
	beq	0	1	done
	beq	0	0	start
	beq	0	1	done
	beq	0	0	start
done	halt
	.fill	5
	.fill	5
	.fill	5
five	.fill	5
