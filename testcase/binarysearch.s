BB_BEGIN:
IMM %GR0, 0
LOAD %GR0, %GR0
IMM %GR1, 0
IMM %GR2, 1
LOAD %GR2, %GR2
SUBI %GR2, %GR2, 1
BB0:
CMP %GR3, %GR2, %GR1
JL %GR3, BB_END
BB1:
ADD %GR3, %GR1, %GR2
RS1 %GR3
ADDI %GR4, %GR3, 2
LOAD %GR4, %GR4
CMP %GR4, %GR4, %GR0
JL %GR4, BB3
BB2:
SUBI %GR2, %GR3, 1
J BB0
BB3:
ADDI %GR1, %GR3, 1
J BB0
BB_END:
HALT