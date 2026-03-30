# Teste 3: contador 3->0 com SLT, BEQ, SUB; OUT = 3
    j    main
main:
    addi $t0,$zero,3
loop:
    slt  $t1,$zero,$t0
    beq  $t1,$zero,done
    addi $t0,$t0,-1
    beq  $zero,$zero,loop
done:
    addi $t2,$zero,3
    out  $t2
    hlt
