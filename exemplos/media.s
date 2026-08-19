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
    addi $t0,$zero,0
    sw   $t0,-6($fp)
    addi $t1,$zero,0
    sw   $t1,-7($fp)
L0:
    lw   $t2,-7($fp)
    addi $t3,$zero,5
    slt  $t4,$t2,$t3
    beq  $t4,$zero,L1
    in   $t5
    lw   $t6,-7($fp)
    addi $t0,$fp,-5
    add  $t1,$t0,$t6
    sw   $t5,0($t1)
    lw   $t7,-6($fp)
    lw   $t8,-7($fp)
    addi $t2,$fp,-5
    add  $t3,$t2,$t8
    lw   $t9,0($t3)
    add  $t0,$t7,$t9
    sw   $t0,-6($fp)
    lw   $t1,-7($fp)
    addi $t2,$zero,1
    add  $t3,$t1,$t2
    sw   $t3,-7($fp)
    j    L0
L1:
    lw   $t4,-6($fp)
    addi $t5,$zero,5
    div  $t4,$t5
    move $t6,$lo
    sw   $t6,-8($fp)
    lw   $t7,-8($fp)
    addi $sp,$sp,-1
    sw   $t7,0($sp)
    out  $t7
    addi $sp,$sp,1
    # END main
    hlt
