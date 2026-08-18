# Assembly gerado automaticamente
.text
.globl main
main:
    addi $sp,$sp,-1
    sw   $ra,0($sp)
    addi $sp,$sp,-1
    sw   $fp,0($sp)
    add  $fp,$sp,$zero
    addi $sp,$sp,-8
    addi $t0,$zero,5
    addi $t1,$zero,0
    addi $t2,$fp,-5
    add  $t3,$t2,$t1
    sw   $t0,0($t3)
    addi $t2,$zero,7
    addi $t3,$zero,1
    addi $t4,$fp,-5
    add  $t5,$t4,$t3
    sw   $t2,0($t5)
    addi $t4,$zero,8
    addi $t5,$zero,2
    addi $t6,$fp,-5
    add  $t7,$t6,$t5
    sw   $t4,0($t7)
    addi $t6,$zero,9
    addi $t7,$zero,3
    addi $t8,$fp,-5
    add  $t9,$t8,$t7
    sw   $t6,0($t9)
    addi $t8,$zero,6
    addi $t9,$zero,4
    addi $t0,$fp,-5
    add  $t1,$t0,$t9
    sw   $t8,0($t1)
    addi $t0,$zero,0
    sw   $t0,-6($fp)
    addi $t1,$zero,0
    sw   $t1,-7($fp)
L0:
    lw   $t2,-7($fp)
    addi $t3,$zero,5
    slt  $t4,$t2,$t3
    beq  $t4,$zero,L1
    lw   $t5,-6($fp)
    lw   $t6,-7($fp)
    addi $t2,$fp,-5
    add  $t3,$t2,$t6
    lw   $t7,0($t3)
    add  $t8,$t5,$t7
    sw   $t8,-6($fp)
    lw   $t9,-7($fp)
    addi $t0,$zero,1
    add  $t1,$t9,$t0
    sw   $t1,-7($fp)
    j    L0
L1:
    lw   $t2,-6($fp)
    addi $t3,$zero,5
    div  $t2,$t3
    move $t4,$lo
    sw   $t4,-8($fp)
    lw   $t5,-8($fp)
    addi $sp,$sp,-1
    sw   $t5,0($sp)
    out  $t5
    addi $sp,$sp,1
    # END main
    hlt
