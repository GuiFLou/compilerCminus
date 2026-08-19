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
    addi $t0,$zero,0
    sw   $t0,-9($fp)
L0:
    lw   $t1,-9($fp)
    addi $t2,$zero,6
    slt  $t3,$t1,$t2
    beq  $t3,$zero,L1
    in   $t4
    lw   $t5,-9($fp)
    addi $t0,$fp,-6
    add  $t1,$t0,$t5
    sw   $t4,0($t1)
    lw   $t6,-9($fp)
    addi $t7,$zero,1
    add  $t8,$t6,$t7
    sw   $t8,-9($fp)
    j    L0
L1:
    in   $t9
    sw   $t9,-7($fp)
    addi $t0,$zero,0
    addi $t1,$zero,1
    sub  $t2,$t0,$t1
    sw   $t2,-8($fp)
    addi $t3,$zero,0
    sw   $t3,-9($fp)
L2:
    lw   $t4,-9($fp)
    addi $t5,$zero,6
    slt  $t6,$t4,$t5
    beq  $t6,$zero,L3
    lw   $t7,-9($fp)
    addi $t2,$fp,-6
    add  $t3,$t2,$t7
    lw   $t8,0($t3)
    lw   $t9,-7($fp)
    sub  $t0,$t8,$t9
    beq  $t0,$zero,.L_eq_0
    addi $t0,$zero,0
    j    .L_eq_1
.L_eq_0:
    addi $t0,$zero,1
.L_eq_1:
    beq  $t0,$zero,L4
    lw   $t1,-9($fp)
    sw   $t1,-8($fp)
    j    L5
L4:
L5:
    lw   $t2,-9($fp)
    addi $t3,$zero,1
    add  $t4,$t2,$t3
    sw   $t4,-9($fp)
    j    L2
L3:
    lw   $t5,-8($fp)
    addi $sp,$sp,-1
    sw   $t5,0($sp)
    out  $t5
    addi $sp,$sp,1
    # END main
    hlt
