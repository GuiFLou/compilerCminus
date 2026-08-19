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
    sw   $t1,-3($fp)
L0:
    lw   $t2,-1($fp)
    addi $t3,$zero,0
    slt  $t4,$t3,$t2
    beq  $t4,$zero,L1
    lw   $t5,-1($fp)
    lw   $t6,-1($fp)
    addi $t7,$zero,10
    div  $t6,$t7
    move $t8,$lo
    addi $t9,$zero,10
    mult $t8,$t9
    move $t0,$lo
    sub  $t1,$t5,$t0
    sw   $t1,-2($fp)
    lw   $t2,-3($fp)
    lw   $t3,-2($fp)
    add  $t4,$t2,$t3
    sw   $t4,-3($fp)
    lw   $t5,-1($fp)
    addi $t6,$zero,10
    div  $t5,$t6
    move $t7,$lo
    sw   $t7,-1($fp)
    j    L0
L1:
    lw   $t8,-3($fp)
    addi $sp,$sp,-1
    sw   $t8,0($sp)
    out  $t8
    addi $sp,$sp,1
    # END main
    hlt
