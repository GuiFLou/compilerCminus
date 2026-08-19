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
    in   $t1
    sw   $t1,-2($fp)
    addi $t2,$zero,1
    sw   $t2,-3($fp)
    addi $t3,$zero,0
    sw   $t3,-4($fp)
L0:
    lw   $t4,-4($fp)
    lw   $t5,-2($fp)
    slt  $t6,$t4,$t5
    beq  $t6,$zero,L1
    lw   $t7,-3($fp)
    lw   $t8,-1($fp)
    mult $t7,$t8
    move $t9,$lo
    sw   $t9,-3($fp)
    lw   $t0,-4($fp)
    addi $t1,$zero,1
    add  $t2,$t0,$t1
    sw   $t2,-4($fp)
    j    L0
L1:
    lw   $t3,-3($fp)
    addi $sp,$sp,-1
    sw   $t3,0($sp)
    out  $t3
    addi $sp,$sp,1
    # END main
    hlt
