# Assembly gerado automaticamente
.text
.globl main
    j    main
main:
    addi $sp,$sp,-1
    sw   $ra,0($sp)
    in   $t0
    sw   $t0,1($gp)
    in   $t1
    sw   $t1,2($gp)
    lw   $t2,1($gp)
    lw   $t3,2($gp)
    slt  $t4,$t3,$t2
    beq  $t4,$zero,L0
    lw   $t5,1($gp)
    addi $sp,$sp,-1
    sw   $t5,0($sp)
    out  $t5
    addi $sp,$sp,1
    j    L1
L0:
    lw   $t6,2($gp)
    addi $sp,$sp,-1
    sw   $t6,0($sp)
    out  $t6
    addi $sp,$sp,1
L1:
    # END main
    hlt
