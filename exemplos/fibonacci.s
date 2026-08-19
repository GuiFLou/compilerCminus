# Assembly gerado automaticamente
.text
.globl main
main:
    addi $sp,$sp,-1
    sw   $ra,0($sp)
    addi $sp,$sp,-1
    sw   $fp,0($sp)
    add  $fp,$sp,$zero
    addi $sp,$sp,-5
    in   $t0
    sw   $t0,-1($fp)
    addi $t1,$zero,0
    sw   $t1,-2($fp)
    addi $t2,$zero,1
    sw   $t2,-3($fp)
    addi $t3,$zero,0
    sw   $t3,-5($fp)
L0:
    lw   $t4,-5($fp)
    lw   $t5,-1($fp)
    slt  $t6,$t4,$t5
    beq  $t6,$zero,L1
    lw   $t7,-2($fp)
    addi $sp,$sp,-1
    sw   $t7,0($sp)
    out  $t7
    addi $sp,$sp,1
    lw   $t8,-2($fp)
    lw   $t9,-3($fp)
    add  $t0,$t8,$t9
    sw   $t0,-4($fp)
    lw   $t1,-3($fp)
    sw   $t1,-2($fp)
    lw   $t2,-4($fp)
    sw   $t2,-3($fp)
    lw   $t3,-5($fp)
    addi $t4,$zero,1
    add  $t5,$t3,$t4
    sw   $t5,-5($fp)
    j    L0
L1:
    # END main
    hlt
