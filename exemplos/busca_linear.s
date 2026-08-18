# Assembly gerado automaticamente
.text
.globl main
main:
    addi $sp,$sp,-1
    sw   $ra,0($sp)
    addi $sp,$sp,-1
    sw   $fp,0($sp)
    add  $fp,$sp,$zero
    addi $sp,$sp,-9
    addi $t0,$zero,4
    addi $t1,$zero,0
    addi $t2,$fp,-6
    add  $t3,$t2,$t1
    sw   $t0,0($t3)
    addi $t2,$zero,7
    addi $t3,$zero,1
    addi $t4,$fp,-6
    add  $t5,$t4,$t3
    sw   $t2,0($t5)
    addi $t4,$zero,2
    addi $t5,$zero,2
    addi $t6,$fp,-6
    add  $t7,$t6,$t5
    sw   $t4,0($t7)
    addi $t6,$zero,9
    addi $t7,$zero,3
    addi $t8,$fp,-6
    add  $t9,$t8,$t7
    sw   $t6,0($t9)
    addi $t8,$zero,5
    addi $t9,$zero,4
    addi $t0,$fp,-6
    add  $t1,$t0,$t9
    sw   $t8,0($t1)
    addi $t0,$zero,1
    addi $t1,$zero,5
    addi $t2,$fp,-6
    add  $t3,$t2,$t1
    sw   $t0,0($t3)
    addi $t2,$zero,9
    sw   $t2,-7($fp)
    addi $t3,$zero,0
    addi $t4,$zero,1
    sub  $t5,$t3,$t4
    sw   $t5,-8($fp)
    addi $t6,$zero,0
    sw   $t6,-9($fp)
L0:
    lw   $t7,-9($fp)
    addi $t8,$zero,6
    slt  $t9,$t7,$t8
    beq  $t9,$zero,L1
    lw   $t0,-9($fp)
    addi $t4,$fp,-6
    add  $t5,$t4,$t0
    lw   $t1,0($t5)
    lw   $t2,-7($fp)
    sub  $t3,$t1,$t2
    beq  $t3,$zero,.L_eq_0
    addi $t3,$zero,0
    j    .L_eq_1
.L_eq_0:
    addi $t3,$zero,1
.L_eq_1:
    beq  $t3,$zero,L2
    lw   $t4,-9($fp)
    sw   $t4,-8($fp)
    j    L3
L2:
L3:
    lw   $t5,-9($fp)
    addi $t6,$zero,1
    add  $t7,$t5,$t6
    sw   $t7,-9($fp)
    j    L0
L1:
    lw   $t8,-8($fp)
    addi $sp,$sp,-1
    sw   $t8,0($sp)
    out  $t8
    addi $sp,$sp,1
    # END main
    hlt
