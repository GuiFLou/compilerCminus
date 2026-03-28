# Assembly gerado automaticamente
.text
.globl main
    j    main
minloc:
    addi $sp,$sp,-1
    sw   $ra,0($sp)
    lw   $t0,2($sp)
    sw   $t0,16($gp)
    lw   $t1,2($sp)
    lw   $t0,3($sp)
    add  $t2,$t0,$t1
    lw   $t2,0($t2)
    sw   $t2,15($gp)
    lw   $t3,2($sp)
    addi $t4,$zero,1
    add  $t5,$t3,$t4
    sw   $t5,14($gp)
L0:
    lw   $t6,14($gp)
    lw   $t7,1($sp)
    slt  $t8,$t6,$t7
    beq  $t8,$zero,L1
    lw   $t9,14($gp)
    lw   $t3,3($sp)
    add  $t4,$t3,$t9
    lw   $t0,0($t4)
    lw   $t1,15($gp)
    slt  $t2,$t0,$t1
    beq  $t2,$zero,L2
    lw   $t3,14($gp)
    lw   $t5,3($sp)
    add  $t6,$t5,$t3
    lw   $t4,0($t6)
    sw   $t4,15($gp)
    lw   $t5,14($gp)
    sw   $t5,16($gp)
    j    L3
L2:
L3:
    lw   $t6,14($gp)
    addi $t7,$zero,1
    add  $t8,$t6,$t7
    sw   $t8,14($gp)
    j    L0
L1:
    lw   $t9,16($gp)
    add  $v0,$t9,$zero
    lw   $ra,0($sp)
    addi $sp,$sp,1
    jr   $ra
    # END minloc
sort:
    addi $sp,$sp,-1
    sw   $ra,0($sp)
    lw   $t0,2($sp)
    sw   $t0,21($gp)
L4:
    lw   $t1,21($gp)
    lw   $t2,1($sp)
    addi $t3,$zero,1
    sub  $t4,$t2,$t3
    slt  $t5,$t1,$t4
    beq  $t5,$zero,L5
    lw   $t6,3($sp)
    addi $sp,$sp,-1
    sw   $t6,0($sp)
    lw   $t7,21($gp)
    addi $sp,$sp,-1
    sw   $t7,0($sp)
    lw   $t8,3($sp)
    addi $sp,$sp,-1
    sw   $t8,0($sp)
    jal  minloc
    addi $sp,$sp,3
    add  $t9,$v0,$zero
    sw   $t9,22($gp)
    lw   $t0,22($gp)
    lw   $t7,3($sp)
    add  $t8,$t7,$t0
    lw   $t1,0($t8)
    sw   $t1,23($gp)
    lw   $t2,21($gp)
    lw   $t9,3($sp)
    add  $t0,$t9,$t2
    lw   $t3,0($t0)
    lw   $t4,22($gp)
    lw   $t1,3($sp)
    add  $t2,$t1,$t4
    sw   $t3,0($t2)
    lw   $t5,23($gp)
    lw   $t6,21($gp)
    lw   $t3,3($sp)
    add  $t4,$t3,$t6
    sw   $t5,0($t4)
    lw   $t7,21($gp)
    addi $t8,$zero,1
    add  $t9,$t7,$t8
    sw   $t9,21($gp)
    j    L4
L5:
    # END sort
    lw   $ra,0($sp)
    addi $sp,$sp,1
    jr   $ra
main:
    addi $sp,$sp,-1
    sw   $ra,0($sp)
    addi $t0,$zero,0
    sw   $t0,25($gp)
L6:
    lw   $t1,25($gp)
    addi $t2,$zero,10
    slt  $t3,$t1,$t2
    beq  $t3,$zero,L7
    in   $t4
    lw   $t5,25($gp)
    addi $t6,$gp,0
    add  $t7,$t6,$t5
    sw   $t4,0($t7)
    lw   $t6,25($gp)
    addi $t7,$zero,1
    add  $t8,$t6,$t7
    sw   $t8,25($gp)
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
    sw   $t2,25($gp)
L8:
    lw   $t3,25($gp)
    addi $t4,$zero,10
    slt  $t5,$t3,$t4
    beq  $t5,$zero,L9
    lw   $t6,25($gp)
    addi $t8,$gp,0
    add  $t9,$t8,$t6
    lw   $t7,0($t9)
    addi $sp,$sp,-1
    sw   $t7,0($sp)
    out  $t7
    lw   $t8,25($gp)
    addi $t9,$zero,1
    add  $t0,$t8,$t9
    sw   $t0,25($gp)
    j    L8
L9:
    # END main
    hlt
