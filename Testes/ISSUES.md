# Issues do Compilador C‑ → MIPS‑Lite

Data do teste: 28 de março de 2026 (re-teste pós-correções)
Arquivos testados: teste2.cms, teste.cms, fatorial.cms, gcd.cms, sort.cms

---

## Issue #1 — ~~[INTERMEDIÁRIO] Comparações geram NOOP em vez de operação real~~ CORRIGIDA

**Status:** CORRIGIDA em 25/03/2026
**Afeta:** fatorial.cms, gcd.cms, sort.cms
**Etapa:** 2, 3
**Arquivo(s) do compilador:** cgen.c, asmgen.c

**Descrição:**
Todos os operadores de comparação (`>`, `<`, `==`, `!=`, `<=`, `>=`) geram a quádrupla `NOOP` no código intermediário. A operação NOOP não computa resultado nenhum — o temporário destinatário nunca recebe o valor da comparação. Consequentemente, `IFF tN label` faz branch baseado em um registrador não inicializado, tornando todos os loops `while` e condições `if` não-determinísticos.

**Correção aplicada:**
Em `cgen.c`, no `case OpK`, cada operador relacional agora emite quádruplas reais:
- `<` → `SLT a b dst`
- `>` → `SLT b a dst` (inverte operandos)
- `==` → `EQ a b dst`
- `!=` → `NEQ a b dst`
- `<=` → `SLT b a tmp` + `ASSIGN 1 one` + `SUB one tmp dst`
- `>=` → `SLT a b tmp` + `ASSIGN 1 one` + `SUB one tmp dst`

Em `asmgen.c`, adicionado `"EQ"` como sinônimo aceito de `"EQUAL"` para compatibilidade.

**Verificação pós-correção (28/03/2026):** ✅ CONFIRMADA
```
fatorial.tm: (SLT, t3, t2, t4) ← correto
gcd.tm: (EQ, t0, t1, t2) ← correto
sort.tm: (SLT, t6, t7, t8), (SLT, t10, t11, t12), etc. ← correto
```

---

## Issue #2 — ~~[INTERMEDIÁRIO] Declarações de variáveis geram código espúrio LOAD+PARAM+CALL~~ CORRIGIDA

**Status:** CORRIGIDA em 25/03/2026
**Afeta:** teste2.cms, teste.cms, fatorial.cms, gcd.cms, sort.cms
**Etapa:** 2
**Arquivo(s) do compilador:** Parser.y, cgen.c, util.c

**Descrição:**
Declarações de variáveis locais (ex: `int x;`) e globais (ex: `int vet[10];`) não geravam quádruplas `ALLOC`. Em vez disso, geravam uma sequência incorreta: `LOAD var`, `PARAM tN`, `CALL integer 1 -`.

**Verificação pós-correção (28/03/2026):** ✅ CONFIRMADA
Todos os 5 programas geram `ALLOC` corretas:
```
teste2.tm: (ALLOC, x, main, -), (ALLOC, y, main, -)
fatorial.tm: (ALLOC, n, main, -), (ALLOC, result, main, -)
sort.tm: (ALLOC, vet, global, 10), (ALLOC, i, minloc, -), ...
```

---

## Issue #3 — ~~[ASSEMBLY] Offsets simbólicos em `lw/sw` quebravam a codificação do imediato~~ CORRIGIDA

**Status:** CORRIGIDA em 25/03/2026
**Afeta:** teste2.cms, teste.cms, fatorial.cms, gcd.cms, sort.cms
**Etapa:** 3, 4
**Arquivo(s) do compilador:** asmgen.c

**Verificação pós-correção (28/03/2026):** ✅ CONFIRMADA
Todos os lw/sw usam offsets numéricos:
```
teste2.s: sw $t0,1($gp) / lw $t2,1($gp)
fatorial.s: sw $t0,1($gp) / lw $t5,2($gp)
gcd.s: lw $t0,1($sp) / sw $t3,4($gp)
sort.s: sw $t0,16($gp) / lw $t7,1($sp)
```

---

## Issue #4 — ~~[ASSEMBLY] `jal` usa número de argumentos em vez de label/nome da função~~ CORRIGIDA

**Status:** CORRIGIDA em 25/03/2026
**Afeta:** gcd.cms, sort.cms
**Etapa:** 3
**Arquivo(s) do compilador:** asmgen.c

**Verificação pós-correção (28/03/2026):** ✅ CONFIRMADA
```
gcd.s: jal gcd ← correto
sort.s: jal minloc, jal sort ← correto
```

---

## Issue #5 — ~~[ASSEMBLY] Programa não inicia em `main` quando há múltiplas funções~~ CORRIGIDA

**Status:** CORRIGIDA em 25/03/2026
**Afeta:** gcd.cms, sort.cms
**Etapa:** 3
**Arquivo(s) do compilador:** cgen.c

**Verificação pós-correção (28/03/2026):** ✅ CONFIRMADA
Todos os programas iniciam com `j main`:
```
teste2.s (linha 4): j main → binário: 010001|addr=1 ✅
gcd.s (linha 4): j main → binário: 010001|addr=38 ✅
sort.s (linha 4): j main → binário: 010001|addr=89 ✅
```

---

## Issue #6 — ~~[ASSEMBLY] Parâmetros de função acessados via `$gp` em vez de `$sp`~~ CORRIGIDA

**Status:** CORRIGIDA em 25/03/2026
**Afeta:** gcd.cms, sort.cms
**Etapa:** 3
**Arquivo(s) do compilador:** asmgen.c

**Verificação pós-correção (28/03/2026):** ✅ CONFIRMADA
```
gcd.s: lw $t0,1($sp) (v), lw $t3,2($sp) (u) ← via $sp ✅
sort.s (minloc): lw $t0,2($sp) (low), lw $t7,1($sp) (high), lw $t0,3($sp) (a) ← via $sp ✅
sort.s (sort): lw $t0,2($sp) (low), lw $t2,1($sp) (high), lw $t6,3($sp) (a) ← via $sp ✅
```
Stack delta tracking funciona corretamente durante preparação de chamadas (offsets ajustados após cada push).

---

## Issue #7 — ~~[ASSEMBLY] `$ra` não é salvo/restaurado em chamadas de função~~ CORRIGIDA

**Status:** CORRIGIDA em 25/03/2026
**Afeta:** gcd.cms, sort.cms
**Etapa:** 3
**Arquivo(s) do compilador:** asmgen.c

**Verificação pós-correção (28/03/2026):** ✅ CONFIRMADA
```
gcd.s: prologue addi $sp,-1 + sw $ra,0($sp); retorno lw $ra,0($sp) + addi $sp,1 + jr $ra ✅
sort.s (minloc): prologue + retorno completo ✅
sort.s (sort): prologue sw $ra ✅ (mas SEM retorno — ver Issue #17)
sort.s (main): prologue sw $ra ✅ (termina com hlt, não precisa retornar)
```

---

## Issue #8 — ~~[ASSEMBLY] Multiplicação e divisão com 3 operandos~~ CORRIGIDA

**Status:** CORRIGIDA em 25/03/2026
**Afeta:** fatorial.cms, gcd.cms
**Etapa:** 3
**Arquivo(s) do compilador:** asmgen.c

**Verificação pós-correção (28/03/2026):** ✅ CONFIRMADA
```
fatorial.s: mult $t5,$t6 + move $t7,$lo ← 2 operandos ✅
gcd.s: div $t6,$t7 + move $t8,$lo ← 2 operandos ✅
gcd.s: mult $t8,$t9 + move $t0,$lo ← 2 operandos ✅
```
Binário confirmado: MULT e DIV codificados com RD=0 (resultado vai para $hi/$lo):
```
fatorial.txt linha 14: 000100|001101|001110|000000|00000000 (RS=$t5, RT=$t6, RD=0) ✅
gcd.txt linha 24: 000110|001110|001111|000000|00000000 (RS=$t6, RT=$t7, RD=0) ✅
gcd.txt linha 27: 000100|010000|010001|000000|00000000 (RS=$t8, RT=$t9, RD=0) ✅
```

---

## Issue #9 — ~~[ENCODER] Instruções `in`/`out` não codificam o registrador~~ CORRIGIDA

**Status:** CORRIGIDA em 28/03/2026
**Afeta:** teste2.cms, teste.cms, fatorial.cms, gcd.cms, sort.cms
**Etapa:** 4
**Arquivo(s) do compilador:** encoder.c

**Verificação pós-correção (28/03/2026):** ✅ CONFIRMADA
```
teste.txt linha 4: 011010|001000|zeros → in $t0 (reg=8=$t0) ✅
teste.txt linha 6: 011010|001001|zeros → in $t1 (reg=9=$t1) ✅
teste2.txt linha 13: 011011|001100|zeros → out $t4 (reg=12=$t4) ✅
fatorial.txt linha 25: 011011|001001|zeros → out $t1 (reg=9=$t1) ✅
gcd.txt linha 56: 011011|001111|zeros → out $t7 (reg=15=$t7) ✅
sort.txt linha 98: 011010|001100|zeros → in $t4 (reg=12=$t4) ✅
sort.txt linha 131: 011011|001111|zeros → out $t7 (reg=15=$t7) ✅
```

---

## Issue #10 — ~~[ENCODER] Formato de instrução usa campos de 5 bits para registradores em vez de 6 bits~~ CORRIGIDA

**Status:** CORRIGIDA em 28/03/2026
**Afeta:** teste2.cms, teste.cms, fatorial.cms, gcd.cms, sort.cms
**Etapa:** 4
**Arquivo(s) do compilador:** encoder.c

**Verificação pós-correção (28/03/2026):** ✅ CONFIRMADA
Formatos agora seguem a ISA:
- F1: `[opcode:6][RS:6][RT:6][RD:6][Shamt:8]` = 32 bits ✅
- F2: `[opcode:6][RD:6][RS:6][Imm:14]` = 32 bits ✅
- F3/IO: `[opcode:6][Reg:6][zeros:20]` = 32 bits ✅

Exemplo: `add $t4,$t2,$t3` (teste2.txt linha 10):
```
000000|001010|001011|001100|00000000
opcode  RS=$t2  RT=$t3  RD=$t4  shamt=0  ← 6 bits por campo ✅
```

Exemplo: `addi $sp,$sp,-1` (teste2.txt linha 2):
```
000001|011101|011101|11111111111111
opcode  RD=$sp  RS=$sp  Imm=-1  ← RD e RS de 6 bits, Imm de 14 bits ✅
```

---

## Issue #11 — ~~[ENCODER] Ordem dos campos F1 difere do ISA~~ CORRIGIDA

**Status:** CORRIGIDA em 28/03/2026
**Afeta:** teste2.cms, teste.cms, fatorial.cms, gcd.cms, sort.cms
**Etapa:** 4
**Arquivo(s) do compilador:** encoder.c

**Verificação pós-correção (28/03/2026):** ✅ CONFIRMADA
Ordem F1 agora é `[opcode][RS][RT][RD][Shamt]` conforme ISA.
```
sub $t2,$t0,$t1 (gcd.txt linha 6): 000010|001000|001001|001010|00000000
                                     SUB     RS=$t0  RT=$t1  RD=$t2     ✅
slt $t8,$t6,$t7 (sort.txt linha 17): 011001|001110|001111|010000|00000000
                                       SLT    RS=$t6  RT=$t7  RD=$t8    ✅
jr $ra (gcd.txt linha 16): 010010|011111|000000|000000|00000000
                            JR     RS=$ra  RT=0    RD=0              ✅
```

---

## Issue #12 — ~~[ENCODER] `$v0` não está no mapeamento de registradores~~ CORRIGIDA

**Status:** CORRIGIDA em 28/03/2026
**Afeta:** gcd.cms, sort.cms
**Etapa:** 4
**Arquivo(s) do compilador:** encoder.c

**Verificação pós-correção (28/03/2026):** ✅ CONFIRMADA
```
gcd.txt linha 13: 000000|001011|000000|000010|00000000 → add $v0,$t3,$zero (RD=000010=$v0=2) ✅
sort.txt linha 40: 000000|010001|000000|000010|00000000 → add $v0,$t9,$zero (RD=000010=$v0=2) ✅
sort.txt linha 65: 000000|000010|000000|010001|00000000 → add $t9,$v0,$zero (RS=000010=$v0=2) ✅
```

---

## Issue #13 — ~~[FRONT-END] Variáveis locais não inseridas na tabela de símbolos~~ CORRIGIDA

**Status:** CORRIGIDA em 25/03/2026
**Afeta:** teste2.cms, teste.cms, fatorial.cms, gcd.cms, sort.cms
**Etapa:** 1
**Arquivo(s) do compilador:** analyze.c, Parser.y

**Verificação pós-correção (28/03/2026):** ✅ CONFIRMADA
```
teste2.cms: x (variable, integer, main, loc=1), y (variable, integer, main, loc=2) ✅
fatorial.cms: n (variable, integer, main, loc=1), result (variable, integer, main, loc=2) ✅
gcd.cms: u (variable, integer, gcd, loc=1), v (variable, integer, gcd, loc=2), x (variable, integer, main, loc=4), y (variable, integer, main, loc=5) ✅
sort.cms: i/x/k em minloc, i/k/t em sort, i em main — todos presentes ✅
```

---

## Issue #14 — ~~[INTERMEDIÁRIO] `ADDR` não gerado para passagem de array como parâmetro~~ CORRIGIDA

**Status:** CORRIGIDA em 25/03/2026
**Afeta:** sort.cms
**Etapa:** 2
**Arquivo(s) do compilador:** cgen.c

**Verificação pós-correção (28/03/2026):** ✅ CONFIRMADA
```
sort.tm linha 104: (ADDR, vet, -, t49) ← endereço base do vetor ✅
sort.tm linha 105: (PARAM, t49, -, -) ← empilha endereço ✅
```

---

## Issue #15 — ~~[ASSEMBLY] `sll` usa nome de variável em vez de registrador para indexação~~ CORRIGIDA

**Status:** CORRIGIDA em 25/03/2026
**Afeta:** sort.cms
**Etapa:** 3
**Arquivo(s) do compilador:** asmgen.c

**Verificação pós-correção (28/03/2026):** ✅ CONFIRMADA
```
sort.s: lw $t0,3($sp) / add $t2,$t0,$t1 / lw $t2,0($t2) ← registradores corretos, sem sll ✅
```

---

## Issue #16 — ~~[ASSEMBLY] Passagem de vetor carrega valor em vez de endereço~~ CORRIGIDA

**Status:** CORRIGIDA em 25/03/2026
**Afeta:** sort.cms
**Etapa:** 3
**Arquivo(s) do compilador:** asmgen.c

**Verificação pós-correção (28/03/2026):** ✅ CONFIRMADA
```
sort.s: addi $t9,$gp,0 ← calcula endereço base de vet ($gp + memloc=0) ✅
        addi $sp,$sp,-1
        sw $t9,0($sp) ← empilha endereço, não valor ✅
```

---

## Issue #17 — ~~[ENCODER] Labels com prefixo `.` (dot-labels) não resolvidos~~ CORRIGIDA

**Status:** CORRIGIDA em 28/03/2026
**Afeta:** gcd.cms
**Etapa:** 4
**Arquivo(s) do compilador:** encoder.c

**Verificação pós-correção (28/03/2026):** ✅ CONFIRMADA
```
gcd.s:
    beq  $t2,$zero,.L_eq_0
    j    .L_eq_1

gcd.txt:
    Linha 7 (addr 6): 01010000101000000000000000000010  ← BEQ com Imm=2 ✅
    Linha 9 (addr 8): 01000100000000000000000000001010  ← J com Addr=10 ✅
```

**Descrição:**
O encoder ignora completamente labels que começam com `.` (dot-labels), como `.L_eq_0:` e `.L_eq_1:`. Estes são gerados pelo `asmgen.c` na tradução de comparações de igualdade (`EQ`/`NEQ`). O problema é duplo:

1. **Primeira passagem (coleta de labels):** a linha 114 de `encoder.c` contém `if(*p=='.'...) continue;`, que pula linhas iniciando com `.` — destinado a ignorar diretivas `.text`/`.globl`, mas também pula labels `.L_eq_*:`.
2. **Segunda passagem (resolução de operandos):** quando um operando como `.L_eq_0` é encontrado, `isalpha('.')` retorna false, e o encoder chama `atoi(".L_eq_0")` que retorna 0.

**Evidência:**
```
gcd.s:
    sub  $t2,$t0,$t1
    beq  $t2,$zero,.L_eq_0     ← encoder gera Imm=0 (deveria ser 2)
    addi $t2,$zero,0
    j    .L_eq_1               ← encoder gera Addr=0 (deveria ser 10)
.L_eq_0:
    addi $t2,$zero,1
.L_eq_1:
    beq  $t2,$zero,L0

gcd.txt:
    Linha 7 (addr 6): 01010000101000000000000000000000
    → BEQ $t2,$zero, Imm=0 ❌ (target = PC+1+0 = 7, mas .L_eq_0 está em addr 9)

    Linha 9 (addr 8): 01000100000000000000000000000000
    → J addr=0 ❌ (vai para j main no início do programa, .L_eq_1 está em addr 10)
```

**Impacto:**
A comparação `v == 0` no GCD fica completamente quebrada:
- Quando v == 0: o beq desvia para addr 7 (instrução errada), depois j addr=0 reinicia o programa
- Quando v != 0: o j addr=0 reinicia o programa
- Em TODOS os casos, a execução volta ao início do programa em vez de seguir a lógica correta

**Esperado:**
```
beq $t2,$zero,.L_eq_0 → Imm = addr(.L_eq_0) - PC - 1 = 9 - 6 - 1 = 2
j .L_eq_1             → Addr = 10
```

**Ajuste necessário:**
Em `encoder.c`:
1. Na primeira passagem (linha ~114): alterar a condição para distinguir diretivas (`.text`, `.globl`, etc.) de labels (`.L_*:`). Uma solução é verificar se a linha contém `:` — se sim, é um label e não deve ser pulado.
2. Na resolução de operandos (linhas ~171, ~176): alterar `isalpha(*op3)` para aceitar também labels iniciando com `.` (ex: `isalpha(*op3) || *op3 == '.'`).

---

## Issue #18 — ~~[ASSEMBLY] Funções void sem `return` explícito não emitem sequência de retorno~~ CORRIGIDA

**Status:** CORRIGIDA em 28/03/2026
**Afeta:** sort.cms
**Etapa:** 3
**Arquivo(s) do compilador:** asmgen.c, cgen.c

**Descrição:**
A função `sort` é `void` e não possui instrução `return` explícita no código-fonte C-. O código intermediário termina com `(END, sort, -, -)` sem uma quádrupla `RET` precedendo-a. Antes da correção, o assembly gerado para `sort` não possuía a sequência de retorno (restaurar `$ra`, ajustar `$sp`, `jr $ra`) ao final da função.

Após o loop while de `sort` terminar (branch para `L5`), a execução caía diretamente na próxima função (`main:`), re-executando `main` desde o início e criando um loop infinito.

**Evidência:**
```
sort.tm:
  83: (GOTO, L4, -, -)
  84: (LAB, L5, -, -)
  85: (END, sort, -, -)     ← sem RET antes do END

sort.s:
    j    L4
L5:
    # END                    ← nenhuma instrução de retorno
main:                        ← execução cai aqui em vez de retornar
    addi $sp,$sp,-1
    sw   $ra,0($sp)
```

**Correção aplicada:**
Em `asmgen.c`, a quádrupla `END` passou a emitir encerramento implícito de função:
- para funções comuns, epílogo com `lw $ra,0($sp)`, `addi $sp,$sp,1` e `jr $ra`;
- para `main`, `hlt`.

**Verificação pós-correção (28/03/2026):** ✅ CONFIRMADA
```
sort.s:
    j    L4
L5:
    # END sort
    lw   $ra,0($sp)          ← restaurar $ra
    addi $sp,$sp,1           ← ajustar $sp
    jr   $ra                  ← retornar ao chamador

main:
    ...

main (final):
    # END main
    hlt
```

---

## Issue #19 — ~~[ASSEMBLY] `END` emite encerramento duplicado após `RET` explícito e em `main`~~ CORRIGIDA

**Status:** CORRIGIDA em 28/03/2026
**Afeta:** gcd.cms, sort.cms
**Etapa:** 3
**Arquivo(s) do compilador:** asmgen.c, cgen.c

**Descrição:**
Após a correção da Issue #18, a quádrupla `END` passou a emitir automaticamente o encerramento da função. Porém, isso agora acontece mesmo quando a função já terminou antes com `RET` explícito, gerando epílogo duplicado e código inalcançável no assembly.

No caso de `main`, `END main` emite `hlt`, mas o pipeline ainda gera a quádrupla final `HALT`, produzindo um segundo `hlt` redundante logo em seguida.

**Evidência:**
```
gcd.s:
    add  $v0,$t2,$zero
    lw   $ra,0($sp)
    addi $sp,$sp,1
    jr   $ra
L1:
    # END gcd
    lw   $ra,0($sp)      ← epílogo duplicado/inalcançável
    addi $sp,$sp,1
    jr   $ra

sort.s:
L9:
    # END main
    hlt
    hlt                  ← duplicado pelo HALT final
```

**Esperado:**
```
gcd.s:
    add  $v0,$t2,$zero
    lw   $ra,0($sp)
    addi $sp,$sp,1
    jr   $ra

sort.s:
L9:
    # END main
    hlt
```

**Ajuste necessário:**
Evitar que `END` emita encerramento quando a função já foi finalizada por `RET`, e evitar duplicação de `hlt` em `main`.

**Correção aplicada:**
Em `asmgen.c`, a emissão de `END` passou a depender da alcançabilidade real do fim da função no fluxo de quádruplas:
- se existe caminho executável até `END`, o fechamento implícito continua sendo emitido;
- se todos os caminhos terminam antes em `RET`, o `END` fica apenas como marcador e não gera epílogo duplicado.

Além disso, a emissão do `HALT` global passou a ser suprimida quando `main` já foi encerrada pelo tratamento de `RET`/`END`.

**Verificação pós-correção (28/03/2026):** ✅ CONFIRMADA
```
gcd.s:
    add  $v0,$t2,$zero
    lw   $ra,0($sp)
    addi $sp,$sp,1
    jr   $ra
L1:
    # END gcd              ← sem epílogo duplicado ✅

sort.s:
L9:
    # END main
    hlt                    ← único hlt no final ✅
```

---

## Issue #20 — [ASSEMBLY] `output()` mantém `PARAM` por contrato, mas `CALL_O` não consome a pilha~~ CORRIGIDA

**Status:** CORRIGIDA
**Afeta:** gcd.cms, sort.cms, qualquer programa com `output()` em laços ou dentro de funções
**Etapa:** 3
**Arquivo(s) do compilador:** cgen.c, asmgen.c

**Descrição:**
Pela documentação do projeto, chamadas para `output()` **devem** gerar a sequência `(PARAM, ..., -, -)` seguida de `(CALL_O, ..., -, -)`. Portanto, a presença de `PARAM` antes de `CALL_O` não é o defeito.

O problema real está no `asmgen.c`: `PARAM` faz `push` no `$sp`, mas `CALL_O` apenas emite `out` e não consome/desempilha o argumento correspondente. Isso deixa palavras extras na pilha a cada `output()`.

Em `main`, o efeito pode ficar mascarado porque o programa termina em `hlt`, mas em funções comuns esse crescimento artificial do frame desloca offsets de parâmetros acessados via `$sp`, podendo corromper leituras, escritas e retorno da função.

`CALL_O` precisa manter a informação do valor enviado para `output()` por dois motivos documentados:
1. `Relatórios/Quadruplas_CI.md` define `PARAM` como o mecanismo padrão de passagem de argumentos e registra explicitamente que `CALL_O` é **precedido de `PARAM` com o mesmo valor**.
2. `Relatórios/CONTEXT.md` repete a mesma convenção no resumo do backend: `PARAM arg - -` para argumentos de chamada e `CALL_O` para `output` com um argumento.
3. `Relatórios/CONTEXTO_PROCESSADOR.md` fixa que `Out` no ISA indica **de qual registrador** o valor será lido; logo, `CALL_O` precisa continuar carregando o operando que identifica o valor a imprimir, enquanto `PARAM` preserva a convenção de chamada na IR.

**Evidência anterior:**
```
gcd.tm:
  38: (PARAM, t17, -, -)
  39: (CALL_O, t17, -, -)

gcd.s:
    addi $sp,$sp,-1
    sw   $t7,0($sp)
    out  $t7               ← argumento permanece empilhado

sort.tm:
 120: (PARAM, t57, -, -)
 121: (CALL_O, t57, -, -)

sort.s:
    addi $sp,$sp,-1
    sw   $t7,0($sp)
    out  $t7               ← ocorre dentro do loop, vazando 1 palavra por iteração
```

**Esperado:**
```
`output()` deve continuar gerando:
(PARAM, x, -, -)
(CALL_O, x, -, -)

No assembly, após emitir `out`, o backend deve consumir o argumento empilhado por `PARAM`
(por exemplo, com `popArgs(1)` ou ajuste equivalente de `$sp`).
```

**Correção aplicada:**
Manter o contrato documentado do intermediário e corrigir apenas o backend:
1. Em `cgen.c`, preservar a emissão de `PARAM` antes de `CALL_O`.
2. Em `asmgen.c`, fazer `CALL_O` consumir o argumento empilhado logo após o `out`.
3. Revalidar offsets de parâmetros em funções que chamam `output()` antes de novos acessos via `$sp`.

**Validação após correção:**
```
gcd.tm:
  38: (PARAM, t17, -, -)
  39: (CALL_O, t17, -, -)

gcd.s:
    addi $sp,$sp,-1
    sw   $t7,0($sp)
    out  $t7
    addi $sp,$sp,1        ← argumento agora é consumido

sort.tm:
 120: (PARAM, t57, -, -)
 121: (CALL_O, t57, -, -)

sort.s:
    addi $sp,$sp,-1
    sw   $t7,0($sp)
    out  $t7
    addi $sp,$sp,1        ← loop não acumula mais palavras extras na pilha
```

---

## Resumo das Issues por Status

### Corrigidas (confirmadas no re-teste de 28/03/2026)

| # | Categoria | Descrição | Arquivo | Status |
|---|-----------|-----------|---------|--------|
| 1 | INTERMEDIÁRIO | Comparações geram NOOP | cgen.c | ✅ Corrigida |
| 2 | INTERMEDIÁRIO | Declarações geram código espúrio | cgen.c, Parser.y | ✅ Corrigida |
| 3 | ASSEMBLY | Offsets simbólicos em lw/sw | asmgen.c | ✅ Corrigida |
| 4 | ASSEMBLY | jal usa nargs em vez de label | asmgen.c | ✅ Corrigida |
| 5 | ASSEMBLY | Sem jump para main no início | cgen.c | ✅ Corrigida |
| 6 | ASSEMBLY | Parâmetros via $gp em vez de $sp | asmgen.c | ✅ Corrigida |
| 7 | ASSEMBLY | $ra não salvo/restaurado | asmgen.c | ✅ Corrigida |
| 8 | ASSEMBLY | mult/div com 3 operandos | asmgen.c | ✅ Corrigida |
| 9 | ENCODER | In/Out sem registrador | encoder.c | ✅ Corrigida |
| 10 | ENCODER | Registradores 5-bit em vez de 6-bit | encoder.c | ✅ Corrigida |
| 11 | ENCODER | Ordem F1 invertida (RD antes de RS) | encoder.c | ✅ Corrigida |
| 12 | ENCODER | $v0 não mapeado | encoder.c | ✅ Corrigida |
| 13 | FRONT-END | Variáveis locais fora da tabela | analyze.c, Parser.y | ✅ Corrigida |
| 14 | INTERMEDIÁRIO | ADDR não gerado para arrays | cgen.c | ✅ Corrigida |
| 15 | ASSEMBLY | sll usa nome de variável | asmgen.c | ✅ Corrigida |
| 16 | ASSEMBLY | Passagem de vetor carrega valor | asmgen.c | ✅ Corrigida |
| 17 | ENCODER | Dot-labels (.L_eq_*) não resolvidos | encoder.c | ✅ Corrigida |
| 18 | ASSEMBLY | Void functions sem retorno | asmgen.c/cgen.c | ✅ Corrigida |
| 19 | ASSEMBLY | `END` gera terminação duplicada | asmgen.c/cgen.c | ✅ Corrigida |
| 20 | ASSEMBLY | `output()` vaza pilha por `PARAM` sem consumo em `CALL_O` | cgen.c/asmgen.c | ✅ Corrigida |

### Abertas (encontradas neste re-teste)

| # | Categoria | Descrição | Arquivo | Prioridade |
|---|-----------|-----------|---------|------------|

### Resultado por programa

| Programa | Etapa 1 | Etapa 2 | Etapa 3 | Etapa 4 | Status |
|----------|---------|---------|---------|---------|--------|
| teste2.cms | ✅ OK | ✅ OK | ✅ OK | ✅ OK | **PASS** |
| teste.cms | ✅ OK | ✅ OK | ✅ OK | ✅ OK | **PASS** |
| fatorial.cms | ✅ OK | ✅ OK | ✅ OK | ✅ OK | **PASS** |
| gcd.cms | ✅ OK | ✅ OK | ✅ OK | ✅ OK | **PASS** |
| sort.cms | ✅ OK | ✅ OK | ✅ OK | ✅ OK | **PASS** |
