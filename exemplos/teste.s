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
    add  $t4,$t2,$t3
    addi $sp,$sp,-1
    sw   $t4,0($sp)
    out  $t4
    addi $sp,$sp,1
    # END main
    hlt
