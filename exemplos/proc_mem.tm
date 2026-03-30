# Teste 2: J, LW/SW em $gp, ADD, OUT, HLT
    j    main
main:
    addi $t0,$zero,12
    sw   $t0,0($gp)
    lw   $t1,0($gp)
    add  $t2,$t1,$zero
    out  $t2
    hlt
