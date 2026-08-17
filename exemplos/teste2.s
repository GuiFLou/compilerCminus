# Assembly gerado automaticamente
.text
.globl main
main:
    addi $sp,$sp,-1
    sw   $ra,0($sp)
    addi $sp,$sp,-1
    sw   $fp,0($sp)
    add  $fp,$sp,$zero
    addi $sp,$sp,-2
    in   $t0
    sw   $t0,-1($fp)
    in   $t1
    sw   $t1,-2($fp)
    lw   $t2,-1($fp)
    lw   $t3,-2($fp)
    slt  $t4,$t3,$t2
    beq  $t4,$zero,L0
    lw   $t5,-1($fp)
    addi $sp,$sp,-1
    sw   $t5,0($sp)
    out  $t5
    addi $sp,$sp,1
    j    L1
L0:
    lw   $t6,-2($fp)
    addi $sp,$sp,-1
    sw   $t6,0($sp)
    out  $t6
    addi $sp,$sp,1
L1:
    # END main
    hlt
