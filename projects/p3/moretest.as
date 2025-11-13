	add	6	5	7
	add	7	5	7
	lw	1	2	num
	lw	2	2	num
	lw	0	1	num
	sw	4	3	num
	lw	0	6	num
	nor	6	6	6
	beq	1	6	skip
	lw	1	2	num
	lw	2	2	num
	lw	0	1	num
skip	sw	4	3	num
	lw	0	6	num
	nor	6	6	6
	halt
num	.fill 23
