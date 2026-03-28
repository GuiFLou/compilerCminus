# Assembly gerado automaticamente
.text
.globl main
    j    main
main:
    addi $sp,$sp,-1
    sw   $ra,0($sp)
    addi $t0,$zero,5
    sw   $t0,1($gp)
    addi $t1,$zero,3
    sw   $t1,2($gp)
    lw   $t2,1($gp)
    lw   $t3,2($gp)
    add  $t4,$t2,$t3
    addi $sp,$sp,-1
    sw   $t4,0($sp)
    out  $t4
    addi $sp,$sp,1
    # END main
    hlt
