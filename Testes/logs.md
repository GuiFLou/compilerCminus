# Logs de Teste — Compilador C‑ → MIPS‑Lite

Data: 28 de março de 2026 (pós-correções Issues #1–#16)

---

=== LOG: teste2.cms ===

--- ETAPA 1: Módulo Analítico (stdout) ---
STATUS: OK
OBSERVAÇÕES:
  - Árvore sintática contém `StmtK: Function main` com tipo `void` ✅
  - Declaração de `x` e `y` como `ExpK: Type integer → StmtK: Variable x/y` ✅
  - Statements de atribuição `x = 5` e `y = 3` presentes ✅
  - Expressão `x + y` como argumento de `output` ✅
  - Chamada de `output` presente ✅
  - Tabela de símbolos: `main` (function, void, global), `x` (variable, integer, main, loc=1), `y` (variable, integer, main, loc=2), `output` e `input` (built-ins) ✅
  - Nenhum erro semântico ou sintático ✅

--- ETAPA 2: Código Intermediário (teste2.tm) ---
STATUS: OK
OBSERVAÇÕES:
  - `GOTO main` na linha 0 (salto para main) ✅
  - `FUN void main -` na linha 1 ✅
  - `ALLOC x main -` e `ALLOC y main -` (linhas 2, 3) ✅
  - `ASSIGN 5 - t0` + `STORE t0 - x` (linhas 4, 5) ✅
  - `ASSIGN 3 - t1` + `STORE t1 - y` (linhas 6, 7) ✅
  - `LOAD x - t2` e `LOAD y - t3` (linhas 8, 9) ✅
  - `ADD t2 t3 t4` (linha 10) ✅
  - `PARAM t4` + `CALL_O t4` (linhas 11, 12) ✅
  - `END main` + `HALT` (linhas 13, 14) ✅
  - Temporários usados de forma consistente ✅

--- ETAPA 3: Assembly (teste2.s) ---
STATUS: OK
OBSERVAÇÕES:
  - `j main` como primeira instrução ✅
  - Label `main:` presente ✅
  - Prologue: `addi $sp,$sp,-1` + `sw $ra,0($sp)` ✅
  - `addi $t0,$zero,5` e `addi $t1,$zero,3` para literais ✅
  - `sw $t0,1($gp)` e `sw $t1,2($gp)` com offsets numéricos ✅
  - `lw $t2,1($gp)` e `lw $t3,2($gp)` ✅
  - `add $t4,$t2,$t3` para soma ✅
  - `out $t4` para output ✅
  - `hlt` ao final ✅
  - Registradores válidos, offsets numéricos, instruções ISA-compatíveis ✅

--- ETAPA 4: Binário (teste2.txt) ---
STATUS: OK
OBSERVAÇÕES:
  - 14 linhas, cada uma com 32 caracteres (0/1) ✅
  - Nenhuma linha vazia ou malformada ✅
  - Decodificação completa das 14 instruções:
    - Linha 1: `010001|00000000000000000000000001` → j addr=1 (main) ✅
    - Linha 2: `000001|011101|011101|11111111111111` → addi $sp,$sp,-1 ✅
    - Linha 3: `010000|011111|011101|00000000000000` → sw $ra,0($sp) ✅
    - Linha 4: `000001|001000|000000|00000000000101` → addi $t0,$zero,5 ✅
    - Linha 5: `010000|001000|011100|00000000000001` → sw $t0,1($gp) ✅
    - Linha 6: `000001|001001|000000|00000000000011` → addi $t1,$zero,3 ✅
    - Linha 7: `010000|001001|011100|00000000000010` → sw $t1,2($gp) ✅
    - Linha 8: `001111|001010|011100|00000000000001` → lw $t2,1($gp) ✅
    - Linha 9: `001111|001011|011100|00000000000010` → lw $t3,2($gp) ✅
    - Linha 10: `000000|001010|001011|001100|00000000` → add $t4,$t2,$t3 (F1: RS=$t2, RT=$t3, RD=$t4) ✅
    - Linha 11: `000001|011101|011101|11111111111111` → addi $sp,$sp,-1 ✅
    - Linha 12: `010000|001100|011101|00000000000000` → sw $t4,0($sp) ✅
    - Linha 13: `011011|001100|00000000000000000000` → out $t4 (reg=12=$t4) ✅
    - Linha 14: `011000|00000000000000000000000000` → hlt ✅
  - Todos os opcodes corretos conforme tabela ISA ✅
  - Campos de 6 bits para registradores ✅
  - Formatos F1/F2/F3/IO corretos ✅
  - Instrução `hlt` = `011000` + 26 zeros ✅


=== LOG: teste.cms ===

--- ETAPA 1: Módulo Analítico (stdout) ---
STATUS: OK
OBSERVAÇÕES:
  - Árvore sintática mostra `main` com chamadas de `input()` como expressão de atribuição ✅
  - Chamada de `output(x + y)` presente ✅
  - `input` (function, integer) e `output` (function, void) como built-ins ✅
  - `x` e `y` declarados como variáveis inteiras no escopo de `main` ✅
  - Nenhum erro sintático ou semântico ✅

--- ETAPA 2: Código Intermediário (teste.tm) ---
STATUS: OK
OBSERVAÇÕES:
  - `GOTO main` + `FUN void main -` ✅
  - `ALLOC x main -` e `ALLOC y main -` ✅
  - `CALL_I` aparece duas vezes (linhas 4, 6) ✅
  - `STORE t0 - x` e `STORE t1 - y` após cada CALL_I ✅
  - `LOAD x`, `LOAD y`, `ADD t2 t3 t4` ✅
  - `PARAM t4` + `CALL_O t4` ✅
  - `END main` + `HALT` ✅

--- ETAPA 3: Assembly (teste.s) ---
STATUS: OK
OBSERVAÇÕES:
  - `j main` + label `main:` ✅
  - `in $t0` e `in $t1` (duas instruções input) ✅
  - Após cada `in`, `sw` armazena via $gp com offset numérico ✅
  - `lw`, `add`, `out`, `hlt` seguem o padrão correto ✅

--- ETAPA 4: Binário (teste.txt) ---
STATUS: OK
OBSERVAÇÕES:
  - 14 linhas, 32 bits cada ✅
  - Decodificação de instruções-chave:
    - Linha 4: `011010|001000|00000000000000000000` → in $t0 (reg=8=$t0) ✅
    - Linha 6: `011010|001001|00000000000000000000` → in $t1 (reg=9=$t1) ✅
    - Linha 10: `000000|001010|001011|001100|00000000` → add $t4,$t2,$t3 ✅
    - Linha 13: `011011|001100|00000000000000000000` → out $t4 (reg=12=$t4) ✅
    - Linha 14: `011000|00000000000000000000000000` → hlt ✅
  - Todos os opcodes, formatos e registradores corretos ✅


=== LOG: fatorial.cms ===

--- ETAPA 1: Módulo Analítico (stdout) ---
STATUS: OK
OBSERVAÇÕES:
  - Árvore sintática inclui `while` com condição `n > 0` ✅
  - Corpo do while: `result * n` e `n - 1` ✅
  - `output(result)` após o while ✅
  - Tabela de símbolos: `n` (variable, integer, main, loc=1), `result` (variable, integer, main, loc=2) ✅
  - `output` reconhecido como built-in ✅
  - Nenhum erro ✅

--- ETAPA 2: Código Intermediário (fatorial.tm) ---
STATUS: OK
OBSERVAÇÕES:
  - `GOTO main` + `FUN void main -` ✅
  - `ALLOC n main -` e `ALLOC result main -` ✅
  - Labels L0 (início loop) e L1 (saída loop) ✅
  - Condição: `LOAD n`, `ASSIGN 0`, `SLT t3 t2 t4` (0 < n → n > 0) ✅
  - `IFF t4 L1` → branch se falso ✅
  - `MUL t5 t6 t7` para `result * n` ✅
  - `SUB t8 t9 t10` para `n - 1` ✅
  - `GOTO L0` de volta ao início ✅
  - `LOAD result` + `CALL_O` após o loop ✅
  - `END main` + `HALT` ✅

--- ETAPA 3: Assembly (fatorial.s) ---
STATUS: OK
OBSERVAÇÕES:
  - `j main` + label `main:` ✅
  - Labels L0: e L1: presentes ✅
  - Condição: `slt $t4,$t3,$t2` + `beq $t4,$zero,L1` ✅
  - **Multiplicação:** `mult $t5,$t6` (2 operandos) + `move $t7,$lo` ✅
  - **Subtração:** `sub $t0,$t8,$t9` ✅
  - `j L0` para voltar ao loop ✅
  - `out $t1` com resultado + `hlt` ✅
  - Offsets numéricos em lw/sw ✅

--- ETAPA 4: Binário (fatorial.txt) ---
STATUS: OK
OBSERVAÇÕES:
  - 26 linhas, 32 bits cada ✅
  - Decodificação de instruções-chave:
    - Linha 1: `010001|addr=1` → j main (addr 1 = label main) ✅
    - Linha 10: `011001|001011|001010|001100|00000000` → slt $t4,$t3,$t2 (F1: RS=$t3, RT=$t2, RD=$t4) ✅
    - Linha 11: `010100|001100|000000|00000000001010` → beq $t4,$zero,offset=10 (PC+1+10=21=L1) ✅
    - Linha 14: `000100|001101|001110|000000|00000000` → mult $t5,$t6 (F1: RS=$t5, RT=$t6, RD=0) ✅
    - Linha 15: `010110|001111|111101|00000000000000` → move $t7,$lo (F2: RD=$t7, RS=$lo=61) ✅
    - Linha 19: `000010|010000|010001|001000|00000000` → sub $t0,$t8,$t9 ✅
    - Linha 21: `010001|addr=7` → j L0 (addr 7 = label L0) ✅
    - Linha 25: `011011|001001|00000000000000000000` → out $t1 (reg=9=$t1) ✅
    - Linha 26: `011000|zeros` → hlt ✅
  - MULT e DIV com RD=0 ✅ (resultado em $hi/$lo, conforme ISA)
  - Branch offset correto (PC+1+offset) ✅
  - Jump address correto (endereço absoluto do label) ✅


=== LOG: gcd.cms ===

--- ETAPA 1: Módulo Analítico (stdout) ---
STATUS: OK
OBSERVAÇÕES:
  - Função `gcd` declarada como `int` com parâmetros `u` (int) e `v` (int) ✅
  - Função `main` como `void` ✅
  - `if` com condição `v == 0`, ramo then `return u`, ramo else `return gcd(v, u-u/v*v)` ✅
  - Expressão `u-u/v*v` parseada com precedência correta: `u - ((u/v) * v)` ✅
  - Tabela de símbolos: `gcd` (function, int, global), `u`/`v` (variable, int, gcd), `x`/`y` (variable, int, main) ✅
  - Built-ins presentes ✅
  - Nenhum erro ✅

--- ETAPA 2: Código Intermediário (gcd.tm) ---
STATUS: OK
OBSERVAÇÕES:
  - `GOTO main` + `FUN int gcd -` + `ARG u` + `ARG v` ✅
  - `EQ t0 t1 t2` para comparação `v == 0` ✅
  - `IFF t2 L0` para desviar ao else ✅
  - Ramo then: `LOAD u` + `RET t3` ✅
  - Ramo else: `DIV t6 t7 t8`, `MUL t8 t9 t10`, `SUB t5 t10 t11` ✅
  - `PARAM t4` + `PARAM t11` + `CALL gcd 2 t12` + `RET t12` ✅
  - `FUN void main -`, `ALLOC x/y`, `CALL_I` ×2, `CALL gcd 2`, `CALL_O` ✅
  - `END gcd`, `END main`, `HALT` ✅
  - Precedência `(u/v)*v` respeitada ✅

--- ETAPA 3: Assembly (gcd.s) ---
STATUS: OK
OBSERVAÇÕES:
  - `j main` + labels `gcd:`, `main:` ✅
  - Parâmetros via $sp: `lw $t0,1($sp)` (v), `lw $t3,2($sp)` (u) ✅
  - Prologue $ra: `addi $sp,$sp,-1` + `sw $ra,0($sp)` ✅
  - EQ implementado como: `sub $t2,$t0,$t1` + `beq $t2,$zero,.L_eq_0` + `addi $t2,$zero,0` + `j .L_eq_1` + `.L_eq_0: addi $t2,$zero,1` + `.L_eq_1: beq $t2,$zero,L0` ✅
  - `div $t6,$t7` (2 operandos) + `move $t8,$lo` ✅
  - `mult $t8,$t9` (2 operandos) + `move $t0,$lo` ✅
  - Chamada recursiva: push args + `jal gcd` + pop + `add $t2,$v0,$zero` ✅
  - Retorno: `add $v0,$tN,$zero` + `lw $ra,0($sp)` + `addi $sp,$sp,1` + `jr $ra` ✅
  - `in`/`out` em main ✅, `hlt` ao final ✅

--- ETAPA 4: Binário (gcd.txt) ---
STATUS: PROBLEMA
OBSERVAÇÕES:
  - 57 linhas, 32 bits cada ✅
  - hlt correto (última linha) ✅
  - Verificações corretas:
    - Linha 1: `010001|addr=38` → j main (addr 38 = label main) ✅
    - Linha 11: `010100|001010|000000|imm=6` → beq $t2,$zero,L0 (PC=10, 10+1+6=17=L0) ✅
    - Linha 13: `000000|001011|000000|000010|00000000` → add $v0,$t3,$zero (RD=$v0=2) ✅
    - Linha 16: `010010|011111|000000|000000|00000000` → jr $ra (RS=31=$ra) ✅
    - Linha 24: `000110|001110|001111|000000|00000000` → div $t6,$t7 (RS=$t6, RT=$t7, RD=0) ✅
    - Linha 25: `010110|010000|111101|00000000000000` → move $t8,$lo ✅
    - Linha 27: `000100|010000|010001|000000|00000000` → mult $t8,$t9 (RD=0) ✅
    - Linha 32: `010011|addr=1` → jal gcd (addr 1 = label gcd) ✅
    - Linha 51: `010011|addr=1` → jal gcd (chamada de main) ✅
    - Linha 56: `011011|001111|zeros` → out $t7 (reg=15=$t7) ✅
  - **PROBLEMA CRÍTICO:** Labels com prefixo `.` (dot-labels) não resolvidos pelo encoder:
    - Linha 7 (addr 6): `beq $t2,$zero,.L_eq_0` → Imm=0 ❌ (deveria ser 2, target=addr 9)
    - Linha 9 (addr 8): `j .L_eq_1` → Addr=0 ❌ (deveria ser 10, target=addr 10)
  - CAUSA: encoder.c linha 114 pula linhas iniciando com `.` (destinado para diretivas `.text`/`.globl`), mas também pula labels `.L_eq_*:`. Na segunda passagem, `isalpha('.')` retorna false, fazendo `atoi(".L_eq_0")` = 0.
  - IMPACTO: A comparação `v == 0` fica completamente quebrada. O `beq` com offset=0 desvia para a instrução errada, e o `j` com addr=0 volta ao início do programa (j main). O GCD nunca funciona corretamente.


=== LOG: sort.cms ===

--- ETAPA 1: Módulo Analítico (stdout) ---
STATUS: OK
OBSERVAÇÕES:
  - Declaração do vetor global `vet[10]` tipo `int` ✅
  - Funções `minloc` (int), `sort` (void), `main` (void) ✅
  - Parâmetros `a[]`, `low`, `high` de minloc e sort ✅
  - Variáveis locais: `i`/`x`/`k` (minloc), `i`/`k`/`t` (sort), `i` (main) na tabela ✅
  - `vet` como vector na tabela com loc=0 ✅
  - Acessos indexados reconhecidos ✅
  - Chamadas sem erros ✅

--- ETAPA 2: Código Intermediário (sort.tm) ---
STATUS: OK
OBSERVAÇÕES:
  - `GOTO main` + `ALLOC vet global 10` ✅
  - Três funções: `FUN int minloc`, `FUN void sort`, `FUN void main` ✅
  - `LOADV` e `STOREV` para acessos indexados ✅
  - `ADDR vet - t49` para passagem de array (linha 104) ✅
  - `PARAM` com quantidades corretas antes de CALL ✅
  - `RET k` em minloc ✅
  - Loops while com IFF/GOTO/LAB corretos ✅
  - `HALT` ao final ✅
  - NOTA: Não há `RET` antes de `END sort` — funções void sem return explícito não emitem RET.

--- ETAPA 3: Assembly (sort.s) ---
STATUS: PROBLEMA
OBSERVAÇÕES:
  - `j main` + labels `minloc:`, `sort:`, `main:` ✅
  - Parâmetros via $sp: `lw $t0,2($sp)` (low), `lw $t7,1($sp)` (high), `lw $t0,3($sp)` (a) ✅
  - Prologue $ra em todas as funções ✅
  - Acesso a vetor global: `addi $t6,$gp,0` + `add $t7,$t6,$t5` + `lw/sw 0($t7)` ✅
  - Acesso a vetor parâmetro: `lw base,$sp` + `add addr,base,index` + `lw/sw 0(addr)` ✅
  - `ADDR vet` traduzido para `addi $t9,$gp,0` ✅
  - Convenção de chamada: save $ra, push args, jal, pop, restore $ra, $v0 ✅
  - Comparações `<` via `slt` + `beq` ✅
  - `in`, `out`, `hlt` ✅
  - **PROBLEMA:** A função `sort` (void, sem return explícito) NÃO emite sequência de retorno (lw $ra / addi $sp / jr $ra) antes de `# END`. Após o loop while terminar (branch para L5), a execução cai diretamente no label `main:` em vez de retornar ao chamador via `jr $ra`.
  - IMPACTO: Quando `main` chama `jal sort`, após sort terminar, a execução recomeça main desde o início, criando loop infinito.

--- ETAPA 4: Binário (sort.txt) ---
STATUS: PROBLEMA (herdado da Etapa 3)
OBSERVAÇÕES:
  - 137 linhas, 32 bits cada ✅
  - Formato correto em todas as linhas ✅
  - hlt presente na última linha ✅
  - Decodificação de instruções representativas:
    - Linha 1: `010001|addr=89` → j main (addr 89 = label main) ✅
    - Linha 18: `010100|010000|000000|imm=20` → beq $t8,$zero,L1 (17+1+20=38=L1) ✅
    - Linha 25: `010100|001010|000000|imm=8` → beq $t2,$zero,L2 (24+1+8=33=L2) ✅
    - Linha 33: `010001|addr=33` → j L3 (addr 33) ✅
    - Linha 38: `010001|addr=14` → j L0 (addr 14) ✅
    - Linha 40: `000000|010001|000000|000010|00000000` → add $v0,$t9,$zero (RD=2=$v0) ✅
    - Linha 43: `010010|011111|000000|000000|00000000` → jr $ra (RS=31=$ra) ✅
    - Linha 53: `010100|001101|000000|imm=36` → beq $t5,$zero,L5 (52+1+36=89) ✅
    - Linha 63: `010011|addr=1` → jal minloc (addr 1 = label minloc) ✅
    - Linha 89: `010001|addr=47` → j L4 (addr 47) ✅
    - Linha 97: `010100|001011|000000|imm=10` → beq $t3,$zero,L7 (96+1+10=107=L7) ✅
    - Linha 98: `011010|001100|zeros` → in $t4 (reg=12=$t4) ✅
    - Linha 107: `010001|addr=93` → j L6 (addr 93) ✅
    - Linha 117: `010011|addr=43` → jal sort (addr 43 = label sort) ✅
    - Linha 124: `010100|001101|000000|imm=12` → beq $t5,$zero,L9 (123+1+12=136=L9) ✅
    - Linha 131: `011011|001111|zeros` → out $t7 (reg=15=$t7) ✅
    - Linha 136: `010001|addr=120` → j L8 (addr 120) ✅
    - Linha 137: `011000|zeros` → hlt ✅
  - Todos os opcodes válidos ✅
  - Registradores 6-bit corretos ✅
  - Endereços de jal corretos (minloc=1, sort=43) ✅
  - Offsets de branch corretos (PC+1+offset) ✅
  - **PROBLEMA HERDADO DA ETAPA 3:** sort termina em L5=addr 89=main, sem retorno. O binário codifica fielmente o assembly, mas o assembly está errado.
