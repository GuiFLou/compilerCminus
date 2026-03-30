# Teste 4: J main, JAL foo, em foo ADDI $v0 + JR $ra; OUT
    j    main
foo:
    addi $v0,$zero,99
    jr   $ra
main:
    jal  foo
    add  $t0,$v0,$zero
    out  $t0
    hlt
