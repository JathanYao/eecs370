	lw	0	1	funct	load reg1 with address of func
	jalr	1	7		jump to func, save PC+1 in reg7
	halt
func	add	0	0	3	reg3 = 0 (clear)
	jalr	7	6		return to saved PC+1
