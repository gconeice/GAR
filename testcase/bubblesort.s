BB_BEGIN:
IMM %GR0, 9
J BB1
BB0:
IMM %GR4, 1
SUB %GR0, %GR0, %GR4
IMM %GR4, 0
EQ %GR4, %GR0, %GR4
JE %GR4, BB_HALT
BB1:
COPY %GR1, %GR0
J BB2
BB2:
LOAD %GR2, %GR1
IMM %GR4, 1
SUB %GR1, %GR1, %GR4
LOAD %GR3, %GR1
ADD %GR1, %GR1, %GR4
CMP %GR4, %GR2, %GR3
JB %GR4, BB3
STORE %GR1, %GR3
IMM %GR4, 1
SUB %GR1, %GR1, %GR4
STORE %GR1, %GR2
ADD %GR1, %GR1, %GR4
J BB3
BB3:
IMM %GR4, 1
SUB %GR1, %GR1, %GR4
IMM %GR4, 0
EQ %GR4, %GR1, %GR4
JE %GR4, BB0
J BB2
BB_HALT:
IMM %GR0, 2
LOAD %GR1, %GR0
IMM %GR0, 5
LOAD %GR2, %GR0
HALT