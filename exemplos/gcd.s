# Assembly gerado automaticamente
.text
.globl main
gcd:
    addi $sp,$sp,-1
    sw   $ra,0($sp)
    addi $sp,$sp,-1
    sw   $fp,0($sp)
    add  $fp,$sp,$zero
    lw   $t0,2($fp)
    addi $t1,$zero,0
    sub  $t2,$t0,$t1
    beq  $t2,$zero,.L_eq_0
    addi $t2,$zero,0
    j    .L_eq_1
.L_eq_0:
    addi $t2,$zero,1
.L_eq_1:
    beq  $t2,$zero,L0
    lw   $t3,3($fp)
    add  $v0,$t3,$zero
    add  $sp,$fp,$zero
    lw   $fp,0($sp)
    addi $sp,$sp,1
    lw   $ra,0($sp)
    addi $sp,$sp,1
    jr   $ra
    j    L1
L0:
    lw   $t4,2($fp)
    addi $sp,$sp,-1
    sw   $t4,0($sp)
    lw   $t5,3($fp)
    lw   $t6,3($fp)
    lw   $t7,2($fp)
    div  $t6,$t7
    move $t8,$lo
    lw   $t9,2($fp)
    mult $t8,$t9
    move $t0,$lo
    sub  $t1,$t5,$t0
    addi $sp,$sp,-1
    sw   $t1,0($sp)
    jal  gcd
    addi $sp,$sp,2
    add  $t2,$v0,$zero
    add  $v0,$t2,$zero
    add  $sp,$fp,$zero
    lw   $fp,0($sp)
    addi $sp,$sp,1
    lw   $ra,0($sp)
    addi $sp,$sp,1
    jr   $ra
L1:
    # END gcd
main:
    addi $sp,$sp,-1
    sw   $ra,0($sp)
    addi $sp,$sp,-1
    sw   $fp,0($sp)
    add  $fp,$sp,$zero
    addi $sp,$sp,-2
    in   $t3
    sw   $t3,-1($fp)
    in   $t4
    sw   $t4,-2($fp)
    lw   $t5,-1($fp)
    addi $sp,$sp,-1
    sw   $t5,0($sp)
    lw   $t6,-2($fp)
    addi $sp,$sp,-1
    sw   $t6,0($sp)
    jal  gcd
    addi $sp,$sp,2
    add  $t7,$v0,$zero
    addi $sp,$sp,-1
    sw   $t7,0($sp)
    out  $t7
    addi $sp,$sp,1
    # END main
    hlt
