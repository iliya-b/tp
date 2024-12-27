    addi a0, x0, %N%
    addi a2, x0, 1
    
    beq a0, x0, 12
    mul a2, a2, a0
    addi a0, a0, -1
    beq x0, x0, -16

    add a1, x0, a2
    addi a0, x0, 1
    ecall
    
    addi a0 x0 10
    ecall