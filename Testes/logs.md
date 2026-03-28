# Logs de Teste — Compilador C‑ → MIPS‑Lite

Data: 28 de março de 2026 (re-teste final pós-correções)

---

=== LOG: teste2.cms ===

--- ETAPA 1: Módulo Analítico (stdout) ---
STATUS: OK
OBSERVAÇÕES:
  - Árvore sintática correta: Function main (void), Variable x (int), Variable y (int)
  - Atribuições x=5, y=3 presentes
  - Expressão x+y como argumento de output
  - Tabela de símbolos: main (function, void, global), x (variable, integer, main, loc=1), y (variable, integer, main, loc=2), output/input (built-ins)
  - Nenhum erro sintático ou semântico

--- ETAPA 2: Código Intermediário (teste2.tm) ---
STATUS: OK
OBSERVAÇÕES:
  - GOTO main, FUN void main
  - ALLOC x main, ALLOC y main
  - ASSIGN 5 → t0, STORE t0 → x; ASSIGN 3 → t1, STORE t1 → y
  - LOAD x → t2, LOAD y → t3, ADD t2 t3 → t4
  - PARAM t4, CALL_O t4
  - END main, HALT
  - 15 quádruplas, temporários t0-t4 consistentes

--- ETAPA 3: Assembly (teste2.s) ---
STATUS: OK
OBSERVAÇÕES:
  - j main no início
  - Label main: com prologue (addi $sp,-1, sw $ra)
  - addi $t0,$zero,5 / sw $t0,1($gp) — literal 5 em x
  - addi $t1,$zero,3 / sw $t1,2($gp) — literal 3 em y
  - lw $t2,1($gp) / lw $t3,2($gp) — carrega x, y
  - add $t4,$t2,$t3 — soma
  - PARAM push + out $t4 + pop — output correto
  - hlt ao final
  - Offsets numéricos, registradores válidos

--- ETAPA 4: Binário (teste2.txt) ---
STATUS: OK
OBSERVAÇÕES:
  - 15 linhas, todas com exatamente 32 bits (0/1)
  - Decodificação manual de todas as 15 instruções:
    addr 0: j 1 (main) — opcode 010001 ✅
    addr 1: addi $sp,$sp,-1 — opcode 000001, Imm=11111111111111 (-1) ✅
    addr 2: sw $ra,0($sp) — opcode 010000 ✅
    addr 3: addi $t0,$zero,5 — opcode 000001, RD=001000($t0), RS=000000($zero), Imm=5 ✅
    addr 4: sw $t0,1($gp) — opcode 010000, RD=001000($t0), RS=011100($gp), Imm=1 ✅
    addr 5: addi $t1,$zero,3 — Imm=3 ✅
    addr 6: sw $t1,2($gp) — Imm=2 ✅
    addr 7: lw $t2,1($gp) — opcode 001111 ✅
    addr 8: lw $t3,2($gp) ✅
    addr 9: add $t4,$t2,$t3 — opcode 000000, F1: RS=$t2, RT=$t3, RD=$t4, Shamt=0 ✅
    addr 10: addi $sp,$sp,-1 ✅
    addr 11: sw $t4,0($sp) ✅
    addr 12: out $t4 — opcode 011011, Reg=001100($t4) ✅
    addr 13: addi $sp,$sp,1 ✅
    addr 14: hlt — 011000 + 26 zeros ✅

---

=== LOG: teste.cms ===

--- ETAPA 1: Módulo Analítico (stdout) ---
STATUS: OK
OBSERVAÇÕES:
  - Árvore sintática correta: main com x, y e chamadas de input()
  - input() como expressão de atribuição
  - output(x+y) presente
  - Tabela de símbolos inclui input (function, integer) e output (function, void)
  - Nenhum erro

--- ETAPA 2: Código Intermediário (teste.tm) ---
STATUS: OK
OBSERVAÇÕES:
  - CALL_I aparece 2 vezes (para x e y)
  - STORE após cada CALL_I
  - LOAD x, LOAD y, ADD, PARAM, CALL_O
  - Estrutura: GOTO → FUN → ALLOC → body → END → HALT

--- ETAPA 3: Assembly (teste.s) ---
STATUS: OK
OBSERVAÇÕES:
  - in $t0 e in $t1 para as duas leituras
  - sw após cada in para $gp (offsets 1 e 2)
  - Mesmo padrão de soma e output de teste2
  - hlt ao final

--- ETAPA 4: Binário (teste.txt) ---
STATUS: OK
OBSERVAÇÕES:
  - 15 linhas, 32 bits cada
  - Instrução in: opcode 011010, Reg=$t0 (addr 3) e Reg=$t1 (addr 5) ✅
  - Instrução out: opcode 011011, Reg=$t4 (addr 12) ✅
  - Demais instruções idênticas ao padrão de teste2 ✅

---

=== LOG: fatorial.cms ===

--- ETAPA 1: Módulo Analítico (stdout) ---
STATUS: OK
OBSERVAÇÕES:
  - While com condição n > 0 presente
  - Corpo: result * n e n - 1
  - output(result) após while
  - Tabela de símbolos: n e result em main
  - Nenhum erro

--- ETAPA 2: Código Intermediário (fatorial.tm) ---
STATUS: OK
OBSERVAÇÕES:
  - Labels L0 (início loop) e L1 (saída loop)
  - SLT t3,t2,t4 para n > 0 (comparação 0 < n) ✅
  - IFF t4 L1 (sai se falso)
  - MUL t5,t6,t7 (result * n)
  - SUB t8,t9,t10 (n - 1)
  - GOTO L0 (volta ao loop)
  - LOAD result, PARAM, CALL_O
  - 28 quádruplas, temporários consistentes

--- ETAPA 3: Assembly (fatorial.s) ---
STATUS: OK
OBSERVAÇÕES:
  - Labels L0 e L1 presentes
  - slt $t4,$t3,$t2 + beq $t4,$zero,L1 — condição do while ✅
  - mult $t5,$t6 + move $t7,$lo — multiplicação com 2 operandos ✅
  - sub $t0,$t8,$t9 — subtração ✅
  - j L0 — volta ao loop ✅
  - out $t1 + hlt ✅
  - Offsets numéricos via $gp

--- ETAPA 4: Binário (fatorial.txt) ---
STATUS: OK
OBSERVAÇÕES:
  - 27 linhas, 32 bits cada
  - Decodificação de instruções-chave:
    addr 9: SLT — opcode 011001, RS=$t3(11), RT=$t2(10), RD=$t4(12) ✅
    addr 10: BEQ — opcode 010100, RD=$t4, RS=$zero, Imm=10 → target=21=L1 ✅
    addr 13: MULT — opcode 000100, RS=$t5(13), RT=$t6(14), RD=0 ✅
    addr 14: MOVE — opcode 010110, RD=$t7(15), RS=$lo(61) ✅
    addr 18: SUB — opcode 000010, RS=$t8(16), RT=$t9(17), RD=$t0(8) ✅
    addr 20: J — opcode 010001, addr=7=L0 ✅
    addr 24: OUT — opcode 011011, Reg=$t1(9) ✅
    addr 26: HLT — opcode 011000 ✅

---

=== LOG: gcd.cms ===

--- ETAPA 1: Módulo Analítico (stdout) ---
STATUS: OK
OBSERVAÇÕES:
  - gcd: function int com params u (int), v (int)
  - main: function void
  - if com v == 0, then return u, else return gcd(v, u-u/v*v)
  - Expressão u-u/v*v parseada como u - ((u/v) * v) — precedência correta ✅
  - Tabela de símbolos completa: gcd, u, v, main, x, y, input, output
  - Nenhum erro

--- ETAPA 2: Código Intermediário (gcd.tm) ---
STATUS: OK
OBSERVAÇÕES:
  - FUN int gcd com ARG u, ARG v
  - EQ t0,t1,t2 para v == 0 ✅
  - IFF t2 L0 (else)
  - RET t3 (return u) no then
  - Else: DIV t6,t7,t8 / MUL t8,t9,t10 / SUB t5,t10,t11 — u-u/v*v ✅
  - PARAM + CALL gcd 2 (recursão)
  - RET t12
  - FUN void main com CALL_I, CALL gcd, CALL_O
  - Labels L0, L1 para if-else
  - 42 quádruplas, temporários consistentes

--- ETAPA 3: Assembly (gcd.s) ---
STATUS: OK
OBSERVAÇÕES:
  - Label gcd: com prologue ($sp-1, sw $ra)
  - Parâmetros via $sp: lw $t0,1($sp) (v), lw $t3,2($sp) (u) ✅
  - EQ implementado via sub+beq+.L_eq_0/.L_eq_1 ✅
  - Retorno: add $v0,$t3,$zero + lw $ra + addi $sp + jr $ra ✅
  - div $t6,$t7 + move $t8,$lo — 2 operandos ✅
  - mult $t8,$t9 + move $t0,$lo — 2 operandos ✅
  - Chamada recursiva: push args, jal gcd, pop 2, get $v0 ✅
  - main: in×2, push args, jal gcd, pop 2, out, hlt ✅
  - END gcd sem epílogo duplicado ✅

--- ETAPA 4: Binário (gcd.txt) ---
STATUS: OK
OBSERVAÇÕES:
  - 58 linhas, 32 bits cada
  - Decodificação de instruções-chave:
    addr 0: J addr=38 (main) ✅
    addr 5: SUB — RS=$t0(8), RT=$t1(9), RD=$t2(10) ✅
    addr 6: BEQ — RD=$t2, RS=$zero, Imm=2 → target=9=.L_eq_0 ✅
    addr 8: J addr=10 (.L_eq_1) ✅
    addr 10: BEQ — RD=$t2, RS=$zero, Imm=6 → target=17=L0 ✅
    addr 12: ADD — RS=$t3(11), RT=$zero(0), RD=$v0(2) ✅
    addr 15: JR — RS=$ra(31) ✅
    addr 23: DIV — RS=$t6(14), RT=$t7(15), RD=0 ✅
    addr 24: MOVE — RD=$t8(16), RS=$lo(61) ✅
    addr 26: MULT — RS=$t8(16), RT=$t9(17), RD=0 ✅
    addr 27: MOVE — RD=$t0(8), RS=$lo(61) ✅
    addr 31: JAL addr=1 (gcd) ✅
    addr 50: JAL addr=1 (gcd) ✅
    addr 55: OUT — Reg=$t7(15) ✅
    addr 57: HLT ✅
  - Dot-labels (.L_eq_0, .L_eq_1) resolvidos corretamente ✅
  - Endereços de jal apontam para posição correta de gcd ✅

---

=== LOG: sort.cms ===

--- ETAPA 1: Módulo Analítico (stdout) ---
STATUS: OK
OBSERVAÇÕES:
  - vet[10] como vetor global (vector, integer)
  - minloc (int, params a[], low, high), sort (void, params a[], low, high), main (void)
  - Variáveis locais em escopos corretos
  - Acessos indexados a[i], a[low], a[k], vet[i] reconhecidos
  - Chamadas minloc(a,i,high) e sort(vet,0,10) sem erros
  - Nenhum erro

--- ETAPA 2: Código Intermediário (sort.tm) ---
STATUS: OK
OBSERVAÇÕES:
  - ALLOC vet global 10 ✅
  - 3 funções: FUN int minloc, FUN void sort, FUN void main
  - LOADV e STOREV para acessos indexados ✅
  - ADDR vet → t49 para passagem de endereço do vetor ✅
  - PARAM com quantidade correta antes de cada CALL
  - CALL minloc 3, CALL sort 3
  - RET k em minloc
  - Fluxo de controle: LAB/IFF/GOTO corretos para while e if
  - 130 quádruplas, temporários consistentes

--- ETAPA 3: Assembly (sort.s) ---
STATUS: OK
OBSERVAÇÕES:
  - Labels minloc:, sort:, main: presentes
  - Parâmetros via $sp em todas as funções ✅
  - Vetor global vet: acesso via addi $t,$gp,0 + add com índice + lw/sw 0(reg) ✅
  - Vetor parâmetro a: carrega base de $sp, add com índice, lw/sw 0(reg) ✅
  - ADDR: addi $t9,$gp,0 para endereço base de vet ✅
  - Convenção de chamada completa: save/restore $ra, push/pop args, jal, jr $ra ✅
  - Comparações < via slt + beq ✅
  - sort (void sem return) tem END com epílogo implícito ✅
  - hlt ao final ✅
  - 141 instruções assembly (excl. diretivas, labels, comentários)

--- ETAPA 4: Binário (sort.txt) ---
STATUS: OK
OBSERVAÇÕES:
  - 141 linhas, 32 bits cada
  - Decodificação de instruções representativas:
    addr 0: J addr=92 (main) ✅
    addr 16: SLT — RS=$t6(14), RT=$t7(15), RD=$t8(16) ✅
    addr 17: BEQ — RD=$t8, RS=$zero, Imm=20 → target=38=L1 ✅
    addr 39: ADD — RS=$t9(17), RT=$zero(0), RD=$v0(2) ✅
    addr 42: JR — RS=$ra(31) ✅
    addr 52: BEQ — RD=$t5(13), RS=$zero, Imm=36 → target=89=L5 ✅
    addr 62: JAL addr=1 (minloc) ✅
    addr 88: J addr=47 (L4) ✅
    addr 91: JR — RS=$ra(31) ✅
    addr 98: SLT — RS=$t1(9), RT=$t2(10), RD=$t3(11) ✅
    addr 99: BEQ — RD=$t3, RS=$zero, Imm=10 → target=110=L7 ✅
    addr 100: IN — Reg=$t4(12) ✅
    addr 109: J addr=96 (L6) ✅
    addr 119: JAL addr=43 (sort) ✅
    addr 133: OUT — Reg=$t7(15) ✅
    addr 140: HLT ✅
  - Todos os endereços de jal apontam para posições corretas ✅
  - Nenhum opcode desconhecido ✅
  - Formatos F1/F2/F3 corretos em todas as instruções ✅
