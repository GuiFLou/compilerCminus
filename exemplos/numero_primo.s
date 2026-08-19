# Assembly gerado automaticamente
.text
.globl main
main:
    addi $sp,$sp,-1
    sw   $ra,0($sp)
    addi $sp,$sp,-1
    sw   $fp,0($sp)
    add  $fp,$sp,$zero
    addi $sp,$sp,-4
    in   $t0
    sw   $t0,-1($fp)
    addi $t1,$zero,2
    sw   $t1,-2($fp)
    addi $t2,$zero,1
    sw   $t2,-4($fp)
L0:
    lw   $t3,-2($fp)
    lw   $t4,-2($fp)
    mult $t3,$t4
    move $t5,$lo
    lw   $t6,-1($fp)
    slt  $t8,$t6,$t5
    addi $t9,$zero,1
    sub  $t7,$t9,$t8
    beq  $t7,$zero,L1
    lw   $t0,-1($fp)
    lw   $t1,-1($fp)
    lw   $t2,-2($fp)
    div  $t1,$t2
    move $t3,$lo
    lw   $t4,-2($fp)
    mult $t3,$t4
    move $t5,$lo
    sub  $t6,$t0,$t5
    sw   $t6,-3($fp)
    lw   $t7,-3($fp)
    addi $t8,$zero,0
    sub  $t9,$t7,$t8
    beq  $t9,$zero,.L_eq_0
    addi $t9,$zero,0
    j    .L_eq_1
.L_eq_0:
    addi $t9,$zero,1
.L_eq_1:
    beq  $t9,$zero,L2
    addi $t0,$zero,0
    sw   $t0,-4($fp)
    j    L3
L2:
L3:
    lw   $t1,-2($fp)
    addi $t2,$zero,1
    add  $t3,$t1,$t2
    sw   $t3,-2($fp)
    j    L0
L1:
    lw   $t4,-4($fp)
    addi $sp,$sp,-1
    sw   $t4,0($sp)
    out  $t4
    addi $sp,$sp,1
    # END main
    hlt
