BB_BEGIN:
# %GR5 = plength 	
	IMM %GR5, 8000
	LOAD %GR5, %GR5
# %GR6 = tlength	
	IMM %GR6, 8001	
	LOAD %GR6, %GR6
# %GR0 = i	
	IMM %GR0, 1
BB0:
# %GR1 = j = lps[i-1] first	
	SUBI %GR1, %GR0, 1
	LOAD %GR1, %GR1
BB0.5:
	ADDI %GR4, %GR0, 500
	LOAD %GR4, %GR4
BB1:	
	ADDI %GR3, %GR1, 500
	LOAD %GR3, %GR3
	EQI %GR8, %GR1, 0
	EQ %GR2, %GR4, %GR3
	ADD %GR2, %GR2, %GR8
	EQI %GR2, %GR2, 0
	JNE %GR2, BB2
	SUBI %GR1, %GR1, 1
	LOAD %GR1, %GR1
	J BB1
BB2:	
	EQ %GR2, %GR4, %GR3
	ADD %GR2, %GR2, %GR1
	STORE %GR0, %GR2
	ADDI %GR0, %GR0, 1
	EQ %GR2, %GR0, %GR5
	JNE %GR2, BB0
BB3:
# %GR7 = now
	IMM %GR7, 0
# %GR1 = res
	IMM %GR1, 0
# %GR0 = i	
	IMM %GR0, 0
BB3.5:
	ADDI %GR3, %GR0, 1000
	LOAD %GR3, %GR3
BB4:
	ADDI %GR4, %GR7, 500
	LOAD %GR4, %GR4
	EQI %GR8, %GR7, 0
	EQ %GR2, %GR3, %GR4
	ADD %GR2, %GR2, %GR8
	EQI %GR2, %GR2, 0
	JNE %GR2, BB5
	SUBI %GR7, %GR7, 1
	LOAD %GR7, %GR7
	J BB4
BB5:
	EQ %GR2, %GR3, %GR4
	ADD %GR7, %GR7, %GR2
	EQ %GR2, %GR7, %GR5
	ADD %GR1, %GR1, %GR2
	SUBI %GR8, %GR7, 1
	LOAD %GR8, %GR8
	CCOPY %GR7, %GR2, %GR8
	ADDI %GR0, %GR0, 1
	EQ %GR2, %GR0, %GR6
	JNE %GR2, BB3.5
BB_END:
	HALT