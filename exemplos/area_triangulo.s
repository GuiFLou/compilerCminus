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
    addi $t0,$zero,10
    sw   $t0,-1($fp)
    addi $t1,$zero,6
    sw   $t1,-2($fp)
    lw   $t2,-1($fp)
    lw   $t3,-2($fp)
    mult $t2,$t3
    move $t4,$lo
    addi $t5,$zero,2
    div  $t4,$t5
    move $t6,$lo
    sw   $t6,-3($fp)
    lw   $t7,-3($fp)
    addi $sp,$sp,-1
    sw   $t7,0($sp)
    out  $t7
    addi $sp,$sp,1
    # END main
    hlt
