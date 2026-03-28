=== LOG: teste2.cms ===

--- ETAPA 1: Módulo Analítico (stdout) ---
STATUS: PROBLEMA
OBSERVAÇÕES:
  - Árvore sintática contém a função `main` com tipo `void` ✅
  - Expressão `x + y` como argumento de `output` ✅
  - Chamada de `output` presente ✅
  - Nenhum erro sintático reportado ✅
  - Nenhum erro semântico reportado ✅
  - PROBLEMA: Variáveis `x` e `y` NÃO aparecem na tabela de símbolos. Apenas `main`, `output`, `input` estão listados.
  - PROBLEMA: Na árvore sintática, declarações de variáveis locais aparecem como `ExpK: Type integer → StmtK: Assign` em vez de `ExpK: Type integer → StmtK: Variable`. Isso indica que o parser não distingue declaração de variável local de atribuição.

--- ETAPA 2: Código Intermediário (teste2.tm) ---
STATUS: PROBLEMA
OBSERVAÇÕES:
  - `FUN void main -` presente ✅
  - `ASSIGN 5 - t2` + `STORE t2 - x` ✅
  - `ASSIGN 3 - t3` + `STORE t3 - y` ✅
  - `LOAD x - t4`, `LOAD y - t5`, `ADD t4 t5 t6` ✅
  - `PARAM t6` + `CALL_O t6` ✅
  - `END main` + `HALT` ✅
  - PROBLEMA: Não existem quádruplas `ALLOC x main -` e `ALLOC y main -`. As variáveis não são alocadas.
  - PROBLEMA: Linhas 1-6 contêm código espúrio. As declarações `int x;` e `int y;` geram `LOAD x - t0`, `PARAM t0`, `CALL integer 1 -` — como se fossem chamadas de função. Isso resulta em instruções desnecessárias e incorretas.

--- ETAPA 3: Assembly (teste2.s) ---
STATUS: PROBLEMA
OBSERVAÇÕES:
  - Label `main:` presente ✅
  - `addi $t2,$zero,5` e `addi $t3,$zero,3` para literais ✅
  - `add $t6,$t4,$t5` para soma ✅
  - `out $t6` para output ✅
  - `hlt` ao final ✅
  - PROBLEMA: Offsets em lw/sw usam nomes de variáveis: `lw $t0,x($gp)`, `sw $t2,x($gp)`. Deveriam ser numéricos: `lw $t0,0($gp)`.
  - PROBLEMA: `jal 1` — usa o número de argumentos (1) como endereço de salto em vez do label de função. Vem do código espúrio `CALL integer 1 -`.
  - PROBLEMA: Código espúrio das declarações de variáveis (linhas 5-14 do .s) é executado desnecessariamente.

--- ETAPA 4: Binário (teste2.txt) ---
STATUS: PROBLEMA
OBSERVAÇÕES:
  - 21 linhas, cada uma com 32 bits ✅
  - Instrução `hlt` (última linha): `01100000000000000000000000000000` = opcode `011000` + 26 zeros ✅
  - PROBLEMA: Instrução `out $t6` (linha 20): `01101100000000000000000000000000`. O campo de registrador é `000000` (= $zero) em vez de $t6 (14). O encoder não codifica o registrador de In/Out.
  - PROBLEMA: Campos de registradores usam 5 bits em vez dos 6 bits da especificação ISA. Formato F1 observado: [opcode:6][RD:5][RS:5][RT:5][Shamt:11]. Formato ISA: [opcode:6][RS:6][RT:6][RD:6][Shamt:8]. Formato F2 observado: [opcode:6][RD:5][RS:5][Imm:16]. Formato ISA: [opcode:6][RD:6][RS:6][Imm:14].
  - PROBLEMA: Ordem dos campos em F1 difere do ISA — encoder coloca RD primeiro, ISA especifica RS primeiro.
  - PROBLEMA: Offsets de lw/sw no binário contêm valores incorretos (nomes de variáveis parseados como garbage pelo encoder).
  - EVIDÊNCIA: Instrução `add $t6,$t4,$t5` (linha 17): `00000001110011000110100000000000`. Decodificando com 5 bits: opcode=000000(ADD) ✅, RD=01110(14=$t6) ✅, RS=01100(12=$t4) ✅, RT=01101(13=$t5) ✅. Mas com 6 bits (ISA): RS=011100(28=$gp) ❌, RT=110001(49=?) ❌, RD=101000(40=?) ❌.


=== LOG: teste.cms ===

--- ETAPA 1: Módulo Analítico (stdout) ---
STATUS: PROBLEMA
OBSERVAÇÕES:
  - Árvore sintática mostra `main` com chamadas de `input()` como expressão de atribuição ✅
  - Chamada de `output(x + y)` presente ✅
  - Built-ins `input` e `output` presentes na tabela ✅
  - Nenhum erro sintático ou semântico ✅
  - PROBLEMA: Variáveis `x` e `y` NÃO aparecem na tabela de símbolos (mesmo problema de teste2.cms).

--- ETAPA 2: Código Intermediário (teste.tm) ---
STATUS: PROBLEMA
OBSERVAÇÕES:
  - `CALL_I` aparece duas vezes (linhas 7, 9) ✅
  - Resultado do `CALL_I` é armazenado com `STORE` em `x` e `y` ✅
  - `LOAD x`, `LOAD y`, `ADD`, `CALL_O` presentes ✅
  - Estrutura geral FUN → body → END → HALT ✅
  - PROBLEMA: Sem `ALLOC` para x e y.
  - PROBLEMA: Linhas 1-6: código espúrio de declarações de variáveis (LOAD+PARAM+CALL integer).

--- ETAPA 3: Assembly (teste.s) ---
STATUS: PROBLEMA
OBSERVAÇÕES:
  - Instrução `in` presente duas vezes ($t2, $t3) ✅
  - Após `in`, valor armazenado com `sw` ✅
  - Carregamento, soma e `out` seguem padrão ✅
  - `hlt` ao final ✅
  - PROBLEMA: Offsets simbólicos em lw/sw (`x($gp)`, `y($gp)`).
  - PROBLEMA: `jal 1` espúrio das declarações de variáveis.

--- ETAPA 4: Binário (teste.txt) ---
STATUS: PROBLEMA
OBSERVAÇÕES:
  - 21 linhas, 32 bits cada ✅
  - hlt correto ✅
  - PROBLEMA: Instrução `in $t2` (linha 11): `01101000000000000000000000000000`. Opcode `011010` (In) ✅, mas campo registrador = `000000` ($zero) em vez de $t2 (10).
  - PROBLEMA: Instrução `out $t6` (linha 20): `01101100000000000000000000000000`. Campo registrador = `000000` em vez de $t6 (14).
  - PROBLEMA: Mesmos problemas de formato (5-bit vs 6-bit, ordem de campos, offsets simbólicos).


=== LOG: fatorial.cms ===

--- ETAPA 1: Módulo Analítico (stdout) ---
STATUS: PROBLEMA
OBSERVAÇÕES:
  - Árvore sintática inclui `while` com condição `n > 0` ✅
  - Corpo do while contém `result * n` e `n - 1` ✅
  - `output(result)` após o while ✅
  - `output` reconhecido como built-in ✅
  - Nenhum erro ✅
  - PROBLEMA: Variáveis `n` e `result` NÃO aparecem na tabela de símbolos.

--- ETAPA 2: Código Intermediário (fatorial.tm) ---
STATUS: PROBLEMA
OBSERVAÇÕES:
  - Labels L0 e L1 para o loop while presentes ✅
  - `MUL t7 t8 t9` para multiplicação ✅
  - `SUB t10 t11 t12` para subtração ✅
  - `GOTO L0` e `IFF t6 L1` para fluxo de controle ✅
  - `LOAD result`, `CALL_O` após o loop ✅
  - PROBLEMA: Sem ALLOC para `n` e `result`.
  - PROBLEMA: Código espúrio nas linhas 1-6 (LOAD+PARAM+CALL para declarações).
  - PROBLEMA: Linha 14: `NOOP t4 t5 t6` — a comparação `n > 0` gera NOOP em vez de operação de comparação real. O resultado `t6` nunca recebe o valor da comparação, tornando o branch na linha 15 (`IFF t6 L1`) baseado em valor não inicializado.

--- ETAPA 3: Assembly (fatorial.s) ---
STATUS: PROBLEMA
OBSERVAÇÕES:
  - Labels L0 e L1 presentes ✅
  - `j L0` para voltar ao loop ✅
  - `sub $t2,$t0,$t1` para `n - 1` ✅
  - `out $t3` e `hlt` presentes ✅
  - PROBLEMA: `nop` (linha 22) em vez de instrução de comparação (slt). `beq $t6,$zero,L1` na linha 23 testa $t6 que nunca foi setado pela comparação.
  - PROBLEMA: `mul $t9,$t7,$t8` (linha 26) com 3 operandos. Deveria ser `mult $t7,$t8` + `move $t9,$lo`.
  - PROBLEMA: Offsets simbólicos em lw/sw (`n($gp)`, `result($gp)`).
  - PROBLEMA: `jal 1` espúrio.

--- ETAPA 4: Binário (fatorial.txt) ---
STATUS: PROBLEMA
OBSERVAÇÕES:
  - 32 linhas, 32 bits cada ✅
  - hlt correto (última linha) ✅
  - Instrução `j L0` (linha 27): `01000100000000000000000000001110`. Opcode `010001` (Jump) ✅, endereço = 14 = posição de L0 ✅.
  - Instrução `beq $t6,$zero,L1` (linha 18): `01010001110000000000000000001001`. Opcode `010100` (beq) ✅. Com 5 bits: RS=01110(14=$t6) ✅, RT=00000(0=$zero) ✅, offset=9 (PC+1=18, 18+9=27=L1) ✅.
  - `mult` (linha 21): `00010010001011111000000000000000`. Opcode `000100` (Mult) ✅. Com 5 bits: RD=10001(17=$t9), RS=01111(15=$t7), RT=10000(16=$t8). RD é preenchido, mas ISA diz que mult não tem RD (resultado vai para Hi/Lo).
  - PROBLEMA: `out $t3` codificado sem registrador.
  - PROBLEMA: Formato 5-bit registradores / ordem de campos diverge do ISA.
  - PROBLEMA: Offsets de lw/sw com valores garbage.


=== LOG: gcd.cms ===

--- ETAPA 1: Módulo Analítico (stdout) ---
STATUS: PROBLEMA
OBSERVAÇÕES:
  - Função `gcd` declarada como `int` com parâmetros `u` e `v` ✅
  - Função `main` como `void` ✅
  - Árvore sintática inclui `if` com condição `v == 0` ✅
  - Ramo then: `return u` ✅
  - Ramo else: `return gcd(v, u-u/v*v)` ✅
  - Expressão `u-u/v*v` parseada corretamente: `u - ((u/v) * v)` com precedência correta ✅
  - `u` e `v` presentes na tabela de símbolos no escopo `gcd` ✅
  - Built-ins `input` e `output` presentes ✅
  - Nenhum erro ✅
  - PROBLEMA: Variáveis `x` e `y` de `main` NÃO na tabela de símbolos.

--- ETAPA 2: Código Intermediário (gcd.tm) ---
STATUS: PROBLEMA
OBSERVAÇÕES:
  - `FUN int gcd -` com `ARGS` ✅
  - `DIV t6 t7 t8`, `MUL t8 t9 t10`, `SUB t5 t10 t11` para `u-u/v*v` ✅
  - `PARAM v`, `PARAM t11`, `CALL gcd 2 t12` para chamada recursiva ✅
  - `RET t3` e `RET t12` presentes ✅
  - `FUN void main`, `CALL_I` ×2, `CALL gcd 2 t19`, `CALL_O t19` ✅
  - `END gcd`, `END main`, `HALT` ✅
  - Expressão `u/v*v` respeita precedência (DIV antes de MUL) ✅
  - PROBLEMA: Linha 4: `NOOP t0 t1 t2` para `v == 0` — deveria ser comparação real (EQ ou SUB).
  - PROBLEMA: Linhas 25-30: código espúrio para declarações de `x` e `y` em main.

--- ETAPA 3: Assembly (gcd.s) ---
STATUS: PROBLEMA
OBSERVAÇÕES:
  - Label `gcd:` presente ✅
  - `add $v0,$t3,$zero` para retorno de valor ✅
  - `jr $ra` para retornar ✅
  - Label `main:` presente ✅
  - `in` para input ✅
  - `out` para output ✅
  - `hlt` ao final ✅
  - PROBLEMA: Parâmetros `u` e `v` acessados via `$gp` (`lw $t0,v($gp)`) em vez de `$sp`. Parâmetros de função devem ser acessados via offsets relativos a `$sp`.
  - PROBLEMA: `nop` (linha 8) em vez de comparação `v == 0`. `beq $t2,$zero,L0` testa $t2 não inicializado.
  - PROBLEMA: `div $t8,$t6,$t7` com 3 operandos — deveria ser `div $t6,$t7` + `move $t8,$lo`.
  - PROBLEMA: `mul $t0,$t8,$t9` com 3 operandos — deveria ser `mult $t8,$t9` + `move $t0,$lo`.
  - PROBLEMA: `jal 2` (linhas 27, 55) usa número de argumentos em vez de `jal gcd`.
  - PROBLEMA: `$ra` não é salvo na pilha antes da chamada recursiva `jal 2`. Após retorno, $ra contém o endereço pós-jal interno, não o do chamador original. `jr $ra` retornará ao lugar errado.
  - PROBLEMA: `jr $ra` seguido de `j L1` (linha 13) — o `j L1` é inalcançável (dead code).
  - PROBLEMA: Execução começa no endereço 0 = label `gcd:`, não em `main`. Não há `j main` no início do programa.
  - PROBLEMA: Offsets simbólicos em lw/sw.

--- ETAPA 4: Binário (gcd.txt) ---
STATUS: PROBLEMA
OBSERVAÇÕES:
  - 52 linhas, 32 bits cada ✅
  - hlt correto ✅
  - `jal 2` (linha 21): `01001100000000000000000000000010`. Opcode `010011` (Jal) ✅, endereço=2. Deveria apontar para a posição do label `gcd` (endereço 0), não para o endereço 2.
  - `jr $ra` (linha 7): `01001011111000000000000000000000`. Opcode `010010` (JumpR) ✅. Com 5 bits: campo=11111=31=$ra ✅.
  - `add $v0,$t3,$zero` (linha 6): `00000000000010110000000000000000`. Com 5 bits: RD=00000(0=$zero) ❌. Deveria ser $v0. O encoder não reconhece $v0 — codifica como registrador 0.
  - PROBLEMA: Formato 5-bit, offsets garbage, in/out sem registrador.
  - PROBLEMA: $v0 codificado como registrador 0 ($zero).


=== LOG: sort.cms ===

--- ETAPA 1: Módulo Analítico (stdout) ---
STATUS: PROBLEMA
OBSERVAÇÕES:
  - Declaração do vetor global `vet[10]` tipo `int` ✅
  - Funções `minloc` (int), `sort` (void), `main` (void) ✅
  - Parâmetros `a[]`, `low`, `high` presentes na tabela para minloc e sort ✅
  - `vet` presente como variável global ✅
  - Acessos indexados (`a[i]`, `a[low]`, `vet[i]`) reconhecidos ✅
  - Chamadas `minloc(a,i,high)` e `sort(vet,0,10)` sem erros ✅
  - Nenhum erro sintático ou semântico ✅
  - PROBLEMA: Variáveis locais ausentes na tabela: `i`, `x`, `k` (minloc); `i`, `k`, `t` (sort); `i` (main).

--- ETAPA 2: Código Intermediário (sort.tm) ---
STATUS: PROBLEMA
OBSERVAÇÕES:
  - Três funções presentes: `FUN int minloc`, `FUN void sort`, `FUN void main` ✅
  - `LOADV` e `STOREV` para acessos indexados presentes ✅
  - `PARAM` com quantidades corretas antes de `CALL minloc 3` e `CALL sort 3` ✅
  - `RET k` em minloc ✅
  - Loops while com IFF/GOTO/LAB presentes ✅
  - PROBLEMA: Linhas 0-2: `LOAD vet`, `PARAM t0`, `CALL integer 1 -` — declaração global `int vet[10]` gera código espúrio em vez de `ALLOC vet global -`.
  - PROBLEMA: Múltiplas instâncias de código espúrio para declarações de variáveis locais em cada função.
  - PROBLEMA: `NOOP` para todas as comparações (linhas 26, 31, 65, 104, 127) — condições `<` e `==` não são computadas.
  - PROBLEMA: Linha 115: `LOAD vet` para passagem de array a `sort()` — deveria ser `ADDR vet` para passar o endereço base do vetor, não o valor do primeiro elemento.

--- ETAPA 3: Assembly (sort.s) ---
STATUS: PROBLEMA
OBSERVAÇÕES:
  - Labels `minloc:`, `sort:`, `main:` presentes ✅
  - LOADV/STOREV traduzidos para cálculos de endereço + lw/sw ✅
  - `in` e `out` presentes ✅
  - `hlt` ao final ✅
  - PROBLEMA: Código espúrio antes de `minloc:` (linhas 4-8) — executado no início do programa (PC=0). O programa nunca chega a `main`.
  - PROBLEMA: Parâmetros (`low`, `high`, `a`, etc.) acessados via `$gp` em vez de `$sp`.
  - PROBLEMA: `sll $t0,a,2` — usa nome de variável "a" em vez de registrador. Deveria ser `sll $t0,$tN,2` com o registrador contendo o índice.
  - PROBLEMA: `sll` multiplica índice por 4 (shift left 2), mas se a memória de dados é endereçada por palavras, não é necessário multiplicar por 4.
  - PROBLEMA: `jal 1` e `jal 3` — usa número de argumentos em vez de labels de função.
  - PROBLEMA: `$ra` não salvo/restaurado antes de `jal` em sort (que chama minloc) e em main (que chama sort).
  - PROBLEMA: `lw $t7,vet($gp)` para passagem do vetor a `sort()` — carrega valor em vez de endereço.
  - PROBLEMA: Offsets simbólicos em todos os lw/sw.

--- ETAPA 4: Binário (sort.txt) ---
STATUS: PROBLEMA
OBSERVAÇÕES:
  - 168 linhas, 32 bits cada — tamanho razoável ✅
  - hlt correto (última linha) ✅
  - `in` (linha 129): `01101000000000000000000000000000`. Registrador = 0 ❌.
  - `out` (linha 162): `01101100000000000000000000000000`. Registrador = 0 ❌.
  - `jal 3` (linha 91): `01001100000000000000000000000011`. Endereço = 3, deveria apontar para label `minloc` ou `sort`.
  - PROBLEMA: Mesmo formato 5-bit registradores, offsets simbólicos, in/out sem registrador, $v0 como $zero.
  - PROBLEMA: Endereços de `jal` não apontam para labels corretos.
