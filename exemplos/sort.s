# Assembly gerado automaticamente
.text
.globl main
minloc:
    addi $sp,$sp,-1
    sw   $ra,0($sp)
    addi $sp,$sp,-1
    sw   $fp,0($sp)
    add  $fp,$sp,$zero
    addi $sp,$sp,-3
    lw   $t0,3($fp)
    sw   $t0,-3($fp)
    lw   $t1,3($fp)
    lw   $t0,4($fp)
    add  $t2,$t0,$t1
    lw   $t2,0($t2)
    sw   $t2,-2($fp)
    lw   $t3,3($fp)
    addi $t4,$zero,1
    add  $t5,$t3,$t4
    sw   $t5,-1($fp)
L0:
    lw   $t6,-1($fp)
    lw   $t7,2($fp)
    slt  $t8,$t6,$t7
    beq  $t8,$zero,L1
    lw   $t9,-1($fp)
    lw   $t3,4($fp)
    add  $t4,$t3,$t9
    lw   $t0,0($t4)
    lw   $t1,-2($fp)
    slt  $t2,$t0,$t1
    beq  $t2,$zero,L2
    lw   $t3,-1($fp)
    lw   $t5,4($fp)
    add  $t6,$t5,$t3
    lw   $t4,0($t6)
    sw   $t4,-2($fp)
    lw   $t5,-1($fp)
    sw   $t5,-3($fp)
    j    L3
L2:
L3:
    lw   $t6,-1($fp)
    addi $t7,$zero,1
    add  $t8,$t6,$t7
    sw   $t8,-1($fp)
    j    L0
L1:
    lw   $t9,-3($fp)
    add  $v0,$t9,$zero
    add  $sp,$fp,$zero
    lw   $fp,0($sp)
    addi $sp,$sp,1
    lw   $ra,0($sp)
    addi $sp,$sp,1
    jr   $ra
    # END minloc
sort:
    addi $sp,$sp,-1
    sw   $ra,0($sp)
    addi $sp,$sp,-1
    sw   $fp,0($sp)
    add  $fp,$sp,$zero
    addi $sp,$sp,-3
    lw   $t0,3($fp)
    sw   $t0,-1($fp)
L4:
    lw   $t1,-1($fp)
    lw   $t2,2($fp)
    addi $t3,$zero,1
    sub  $t4,$t2,$t3
    slt  $t5,$t1,$t4
    beq  $t5,$zero,L5
    lw   $t6,4($fp)
    addi $sp,$sp,-1
    sw   $t6,0($sp)
    lw   $t7,-1($fp)
    addi $sp,$sp,-1
    sw   $t7,0($sp)
    lw   $t8,2($fp)
    addi $sp,$sp,-1
    sw   $t8,0($sp)
    jal  minloc
    addi $sp,$sp,3
    add  $t9,$v0,$zero
    sw   $t9,-2($fp)
    lw   $t0,-2($fp)
    lw   $t7,4($fp)
    add  $t8,$t7,$t0
    lw   $t1,0($t8)
    sw   $t1,-3($fp)
    lw   $t2,-1($fp)
    lw   $t9,4($fp)
    add  $t0,$t9,$t2
    lw   $t3,0($t0)
    lw   $t4,-2($fp)
    lw   $t1,4($fp)
    add  $t2,$t1,$t4
    sw   $t3,0($t2)
    lw   $t5,-3($fp)
    lw   $t6,-1($fp)
    lw   $t3,4($fp)
    add  $t4,$t3,$t6
    sw   $t5,0($t4)
    lw   $t7,-1($fp)
    addi $t8,$zero,1
    add  $t9,$t7,$t8
    sw   $t9,-1($fp)
    j    L4
L5:
    # END sort
    add  $sp,$fp,$zero
    lw   $fp,0($sp)
    addi $sp,$sp,1
    lw   $ra,0($sp)
    addi $sp,$sp,1
    jr   $ra
main:
    addi $sp,$sp,-1
    sw   $ra,0($sp)
    addi $sp,$sp,-1
    sw   $fp,0($sp)
    add  $fp,$sp,$zero
    addi $sp,$sp,-1
    addi $t0,$zero,0
    sw   $t0,-1($fp)
L6:
    lw   $t1,-1($fp)
    addi $t2,$zero,10
    slt  $t3,$t1,$t2
    beq  $t3,$zero,L7
    in   $t4
    lw   $t5,-1($fp)
    addi $t6,$gp,0
    add  $t7,$t6,$t5
    sw   $t4,0($t7)
    lw   $t6,-1($fp)
    addi $t7,$zero,1
    add  $t8,$t6,$t7
    sw   $t8,-1($fp)
    j    L6
L7:
    addi $t9,$gp,0
    addi $sp,$sp,-1
    sw   $t9,0($sp)
    addi $t0,$zero,0
    addi $sp,$sp,-1
    sw   $t0,0($sp)
    addi $t1,$zero,10
    addi $sp,$sp,-1
    sw   $t1,0($sp)
    jal  sort
    addi $sp,$sp,3
    addi $t2,$zero,0
    sw   $t2,-1($fp)
L8:
    lw   $t3,-1($fp)
    addi $t4,$zero,10
    slt  $t5,$t3,$t4
    beq  $t5,$zero,L9
    lw   $t6,-1($fp)
    addi $t8,$gp,0
    add  $t9,$t8,$t6
    lw   $t7,0($t9)
    addi $sp,$sp,-1
    sw   $t7,0($sp)
    out  $t7
    addi $sp,$sp,1
    lw   $t8,-1($fp)
    addi $t9,$zero,1
    add  $t0,$t8,$t9
    sw   $t0,-1($fp)
    j    L8
L9:
    # END main
    hlt
