# Assembly gerado automaticamente
.text
.globl main
fact:
    addi $sp,$sp,-1
    sw   $ra,0($sp)
    addi $sp,$sp,-1
    sw   $fp,0($sp)
    add  $fp,$sp,$zero
    addi $sp,$sp,-1
    lw   $t0,2($fp)
    addi $t1,$zero,2
    slt  $t2,$t0,$t1
    beq  $t2,$zero,L0
    addi $t3,$zero,1
    add  $v0,$t3,$zero
    add  $sp,$fp,$zero
    lw   $fp,0($sp)
    addi $sp,$sp,1
    lw   $ra,0($sp)
    addi $sp,$sp,1
    jr   $ra
    j    L1
L0:
L1:
    lw   $t4,2($fp)
    addi $t5,$zero,1
    sub  $t6,$t4,$t5
    addi $sp,$sp,-1
    sw   $t6,0($sp)
    jal  fact
    addi $sp,$sp,1
    add  $t7,$v0,$zero
    sw   $t7,-1($fp)
    lw   $t8,2($fp)
    lw   $t9,-1($fp)
    mult $t8,$t9
    move $t0,$lo
    add  $v0,$t0,$zero
    add  $sp,$fp,$zero
    lw   $fp,0($sp)
    addi $sp,$sp,1
    lw   $ra,0($sp)
    addi $sp,$sp,1
    jr   $ra
    # END fact
main:
    addi $sp,$sp,-1
    sw   $ra,0($sp)
    addi $sp,$sp,-1
    sw   $fp,0($sp)
    add  $fp,$sp,$zero
    addi $sp,$sp,-2
    in   $t1
    sw   $t1,-1($fp)
    lw   $t2,-1($fp)
    addi $sp,$sp,-1
    sw   $t2,0($sp)
    jal  fact
    addi $sp,$sp,1
    add  $t3,$v0,$zero
    sw   $t3,-2($fp)
    lw   $t4,-2($fp)
    addi $sp,$sp,-1
    sw   $t4,0($sp)
    out  $t4
    addi $sp,$sp,1
    # END main
    hlt
