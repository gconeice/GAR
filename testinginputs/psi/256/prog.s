BB_BEGIN:
	IMM %GR0, 0
	IMM %GR1, 256
	IMM %GR2, 0
BB0:
	LOAD %GR3, %GR0
	LOAD %GR4, %GR1
	CMP %GR5, %GR3, %GR4
	RS1 %GR5
	ADD %GR0, %GR0, %GR5
	CMP %GR5, %GR4, %GR3
	RS1 %GR5
	ADD %GR1, %GR1, %GR5
	EQ %GR5, %GR3, %GR4
	ADD %GR0, %GR0, %GR5
	ADD %GR2, %GR2, %GR5
	EQI %GR5, %GR0, 256
	EQI %GR6, %GR1, 512
	XOR %GR5, %GR5, %GR6
	JNE %GR5, BB0
BB_END:
	HALT
