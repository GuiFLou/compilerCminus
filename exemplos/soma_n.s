# Assembly gerado automaticamente
.text
.globl main
main:
    addi $sp,$sp,-1
    sw   $ra,0($sp)
    addi $sp,$sp,-1
    sw   $fp,0($sp)
    add  $fp,$sp,$zero
    addi $sp,$sp,-3
    in   $t0
    sw   $t0,-1($fp)
    addi $t1,$zero,0
    sw   $t1,-2($fp)
    addi $t2,$zero,1
    sw   $t2,-3($fp)
L0:
    lw   $t3,-3($fp)
    lw   $t4,-1($fp)
    slt  $t6,$t4,$t3
    addi $t7,$zero,1
    sub  $t5,$t7,$t6
    beq  $t5,$zero,L1
    lw   $t8,-2($fp)
    lw   $t9,-3($fp)
    add  $t0,$t8,$t9
    sw   $t0,-2($fp)
    lw   $t1,-3($fp)
    addi $t2,$zero,1
    add  $t3,$t1,$t2
    sw   $t3,-3($fp)
    j    L0
L1:
    lw   $t4,-2($fp)
    addi $sp,$sp,-1
    sw   $t4,0($sp)
    out  $t4
    addi $sp,$sp,1
    # END main
    hlt
