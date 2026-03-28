# Assembly gerado automaticamente
.text
.globl main
    j    main
main:
    addi $sp,$sp,-1
    sw   $ra,0($sp)
    addi $t0,$zero,5
    sw   $t0,1($gp)
    addi $t1,$zero,1
    sw   $t1,2($gp)
L0:
    lw   $t2,1($gp)
    addi $t3,$zero,0
    slt  $t4,$t3,$t2
    beq  $t4,$zero,L1
    lw   $t5,2($gp)
    lw   $t6,1($gp)
    mult $t5,$t6
    move $t7,$lo
    sw   $t7,2($gp)
    lw   $t8,1($gp)
    addi $t9,$zero,1
    sub  $t0,$t8,$t9
    sw   $t0,1($gp)
    j    L0
L1:
    lw   $t1,2($gp)
    addi $sp,$sp,-1
    sw   $t1,0($sp)
    out  $t1
    addi $sp,$sp,1
    # END main
    hlt
