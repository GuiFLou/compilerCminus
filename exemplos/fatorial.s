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
    addi $t0,$zero,5
    sw   $t0,-1($fp)
    addi $t1,$zero,1
    sw   $t1,-2($fp)
L0:
    lw   $t2,-1($fp)
    addi $t3,$zero,0
    slt  $t4,$t3,$t2
    beq  $t4,$zero,L1
    lw   $t5,-2($fp)
    lw   $t6,-1($fp)
    mult $t5,$t6
    move $t7,$lo
    sw   $t7,-2($fp)
    lw   $t8,-1($fp)
    addi $t9,$zero,1
    sub  $t0,$t8,$t9
    sw   $t0,-1($fp)
    j    L0
L1:
    lw   $t1,-2($fp)
    addi $sp,$sp,-1
    sw   $t1,0($sp)
    out  $t1
    addi $sp,$sp,1
    # END main
    hlt
