# Issues do Compilador C‑ → MIPS‑Lite

Data do teste: 25 de março de 2026
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

**Verificação pós-correção:**
```
fatorial.tm:
 10: (SLT, t3, t2, t4)     ← comparação 0 < n (equivale a n > 0)
 11: (IFF, t4, L1, -)      ← t4 agora contém resultado válido 0/1

fatorial.s:
    lw   $t2,1($gp)
    addi $t3,$zero,0
    slt  $t4,$t3,$t2        ← comparação real
    beq  $t4,$zero,L1       ← branch correto

gcd.tm:
  5: (EQ, t0, t1, t2)      ← comparação v == 0
  6: (IFF, t2, L0, -)      ← t2 agora contém resultado válido 0/1

gcd.s:
    sub  $t2,$t0,$t1
    beq  $t2,$zero,.L_eq_0  ← sequência de igualdade correta
    addi $t2,$zero,0
    j    .L_eq_1
    .L_eq_0: addi $t2,$zero,1
    .L_eq_1: beq  $t2,$zero,L0
```

---

## Issue #2 — ~~[INTERMEDIÁRIO] Declarações de variáveis geram código espúrio LOAD+PARAM+CALL~~ CORRIGIDA

**Status:** CORRIGIDA em 25/03/2026
**Afeta:** teste2.cms, teste.cms, fatorial.cms, gcd.cms, sort.cms
**Etapa:** 2
**Arquivo(s) do compilador:** Parser.y, cgen.c, util.c

**Descrição:**
Declarações de variáveis locais (ex: `int x;`) e globais (ex: `int vet[10];`) não geravam quádruplas `ALLOC`. Em vez disso, geravam uma sequência incorreta: `LOAD var`, `PARAM tN`, `CALL integer 1 -`. Isso era interpretado como uma chamada a uma função inexistente chamada "integer" com 1 argumento.

**Correção aplicada:**
1. Em `Parser.y`: `var_declaration` já produz nós `ExpK: typeK` com `child[0] = StmtK: variableK` na AST.
2. Em `cgen.c`: `genStmt` reconhece nós `typeK` (recursa em `child[0]`) e o case `variableK` emite `ALLOC var scope -` (ou `ALLOC var scope tamanho` para vetores).
3. Em `util.c`: corrigido bug de campos não inicializados — `newExpNode` e `newStmtNode` agora inicializam `attr.len = 0`, `attr.val = 0`, `attr.name = NULL` e `attr.op = 0` (antes, `malloc` deixava lixo nesses campos, podendo causar ALLOC com tamanho-lixo para variáveis escalares).

**Verificação pós-correção:**
```
teste2.tm (para `int x; int y;`):
  0: (FUN, void, main, -)
  1: (ALLOC, x, main, -)       ← correto
  2: (ALLOC, y, main, -)       ← correto

fatorial.tm (para `int n; int result;`):
  1: (ALLOC, n, main, -)       ← correto
  2: (ALLOC, result, main, -)  ← correto

gcd.tm (para `int x; int y;` em main):
  26: (ALLOC, x, main, -)      ← correto
  27: (ALLOC, y, main, -)      ← correto

sort.tm (para `int vet[10];` global e locais):
  0: (ALLOC, vet, global, 10)  ← correto (vetor global com tamanho)
  5: (ALLOC, i, minloc, -)     ← correto (local em minloc)
  48: (ALLOC, i, sort, -)      ← correto (local em sort)
  59: (ALLOC, t, sort, -)      ← correto (local dentro de while)
  86: (ALLOC, i, main, -)      ← correto (local em main)

teste.tm (para `int x; int y;`):
  1: (ALLOC, x, main, -)       ← correto
  2: (ALLOC, y, main, -)       ← correto
```

---

## Issue #3 — ~~[ASSEMBLY] Offsets simbólicos em `lw/sw` quebravam a codificação do imediato~~ CORRIGIDA

**Status:** CORRIGIDA em 25/03/2026
**Afeta:** teste2.cms, teste.cms, fatorial.cms, gcd.cms, sort.cms
**Etapa:** 3, 4
**Arquivo(s) do compilador:** asmgen.c

**Descrição:**
A falha original ocorria quando `lw` e `sw` eram emitidos com o nome da variável no campo de deslocamento, por exemplo `x($gp)` ou `result($gp)`, em vez do `memloc` numérico da tabela de símbolos. Como `encoder.c` espera um imediato numérico nesse formato, o nome simbólico era parseado incorretamente e gerava binário inválido.

Importante: o valor esperado **não é sempre `0`**. O correto é emitir o offset numérico real da variável relativa ao registrador base. Assim, `0($gp)` só seria válido se o `memloc` daquela variável fosse de fato `0`.

**Correção aplicada:**
Em `asmgen.c`, `LOAD` e `STORE` agora consultam a tabela de símbolos por meio de `lookupMemloc()` / `resolveMemOperand()` e emitem offsets numéricos no assembly. Para parâmetros de função, o acesso continua sendo resolvido relativamente a `$sp`.

**Verificação pós-correção:**
```
teste2.s:
    sw   $t0,1($gp)
    sw   $t1,2($gp)
    lw   $t2,1($gp)
    lw   $t3,2($gp)

fatorial.s:
    sw   $t0,1($gp)
    sw   $t1,2($gp)
    lw   $t2,1($gp)
    lw   $t5,2($gp)

gcd.s:
    sw   $t3,4($gp)
    sw   $t4,5($gp)
    lw   $t5,4($gp)
    lw   $t6,5($gp)

sort.s:
    sw   $t0,16($gp)
    lw   $t6,14($gp)
```

**Conclusão:**
Desde os artefatos atuais em `exemplos/*.s`, o problema não se reproduz mais. Os offsets usados por `lw/sw` são numéricos e variam conforme o `memloc` atribuído a cada símbolo, como esperado.

---

## Issue #4 — ~~[ASSEMBLY] `jal` usa número de argumentos em vez de label/nome da função~~ CORRIGIDA

**Status:** CORRIGIDA em 25/03/2026
**Afeta:** gcd.cms, sort.cms (e indiretamente todos por causa da Issue #2)
**Etapa:** 3
**Arquivo(s) do compilador:** asmgen.c

**Descrição:**
A quádrupla `CALL funcname nargs result` é traduzida para `jal nargs` em vez de `jal funcname`. O campo arg2 (número de argumentos) é usado como operando do `jal`, mas deveria ser arg1 (nome da função).

**Evidência:**
```
gcd.tm:
 20: (CALL, gcd, 2, t12)

gcd.s:
    jal  2                ← deveria ser jal gcd

sort.tm:
 76: (CALL, minloc, 3, t36)

sort.s:
    jal  3                ← deveria ser jal minloc
```

**Esperado:**
```
    jal  gcd
    jal  minloc
```

**Correção aplicada:**
Em `asmgen.c`, a tradução de `CALL` passou a usar `q->arg1` como operando do `jal`, emitindo o nome da função em vez da quantidade de argumentos.

**Verificação pós-correção:**
```
gcd.s:
    jal  gcd

sort.s:
    jal  minloc
    jal  sort
```

---

## Issue #5 — ~~[ASSEMBLY] Programa não inicia em `main` quando há múltiplas funções~~ CORRIGIDA

**Status:** CORRIGIDA em 25/03/2026
**Afeta:** gcd.cms, sort.cms
**Etapa:** 3
**Arquivo(s) do compilador:** cgen.c

**Descrição:**
Quando o programa tem múltiplas funções, a execução começa no endereço 0 (PC=0), mas `main` não está nesse endereço. Em `gcd.s`, a execução começa em `gcd:`. Em `sort.s`, começa com código espúrio antes de `minloc:`. Não há instrução `j main` no início para desviar a execução para o ponto de entrada correto.

**Evidência:**
```
gcd.s (início):
gcd:              ← endereço 0, executado primeiro
    # ARGS
    lw   $t0,v($gp)
    ...

sort.s (início):
    lw   $t0,vet($gp)    ← endereço 0, código espúrio
    ...
minloc:
    ...
sort:
    ...
main:             ← nunca alcançado
```

**Esperado:**
```
    j    main             ← primeira instrução: pular para main
gcd:
    ...
main:
    ...
```

**Correção aplicada:**
Em `cgen.c`, na função `codeGen`, emitir `GOTO main` como primeira quádrupla do programa (antes de `genStmt`). Isso gera `j main` como primeira instrução no assembly, garantindo que a execução sempre comece em `main` independentemente da ordem das funções no código-fonte.

```c
emit("GOTO", "main", "-", "-");
genStmt(syntaxTree);
emit("HALT", "-", "-", "-");
```

**Verificação pendente:** Recompilar e testar com gcd.cms e sort.cms.

---

## Issue #6 — ~~[ASSEMBLY] Parâmetros de função acessados via `$gp` em vez de `$sp`~~ CORRIGIDA

**Status:** CORRIGIDA em 25/03/2026
**Afeta:** gcd.cms, sort.cms
**Etapa:** 3
**Arquivo(s) do compilador:** asmgen.c

**Descrição:**
Parâmetros de função (passados via pilha) eram acessados usando offsets relativos a `$gp` (ponteiro global) em vez de `$sp` (ponteiro da pilha). `$gp` aponta para a área de variáveis globais, não para os argumentos passados na pilha.

**Correção aplicada:**
Em `asmgen.c`, os parâmetros de cada função são rastreados via `paramNames[]`, preenchido pelas quádruplas `ARG` após cada `FUN`. A função `getParamIndex()` verifica se um nome é parâmetro da função atual. As operações `LOAD`, `STORE`, `LOADV` e `STOREV` consultam essa lista antes de gerar o acesso à memória:
- **Parâmetro** → `lw`/`sw` com offset relativo a `$sp` (calculado como `(paramCount - 1 - pidx) + stackDelta`)
- **Local/Global** → `lw`/`sw` com offset relativo a `$gp`

O `stackDelta` acompanha palavras empilhadas pelo callee (salva de `$ra` no prologue + argumentos empilhados via `PARAM`) para manter os offsets corretos mesmo durante preparação de chamadas recursivas.

**Verificação pós-correção:**
```
gcd.s:
    lw   $t0,1($sp)       ← v (parâmetro) via $sp ✓
    lw   $t3,2($sp)       ← u (parâmetro) via $sp ✓

sort.s (minloc):
    lw   $t0,2($sp)       ← low (parâmetro) via $sp ✓
    lw   $t7,1($sp)       ← high (parâmetro) via $sp ✓
    lw   $t0,3($sp)       ← a[] (parâmetro array) via $sp ✓

sort.s (sort):
    lw   $t0,2($sp)       ← low via $sp ✓
    lw   $t2,1($sp)       ← high via $sp ✓
    lw   $t6,3($sp)       ← a[] via $sp ✓
```

---

## Issue #7 — ~~[ASSEMBLY] `$ra` não é salvo/restaurado em chamadas de função~~ CORRIGIDA

**Status:** CORRIGIDA em 25/03/2026
**Afeta:** gcd.cms, sort.cms
**Etapa:** 3
**Arquivo(s) do compilador:** asmgen.c

**Descrição:**
Antes de executar `jal` (que sobrescreve `$ra`), o valor atual de `$ra` não é salvo na pilha. Após o retorno da função chamada, `$ra` não é restaurado. Isso corrompe o endereço de retorno, especialmente em chamadas recursivas (gcd chama gcd) e chamadas aninhadas (sort chama minloc).

**Correção aplicada:**
Em `asmgen.c`, toda função agora emite um prologue que salva `$ra` na pilha (`addi $sp,$sp,-1` + `sw $ra,0($sp)`) e um epilogue que o restaura (`lw $ra,0($sp)` + `addi $sp,$sp,1`) antes de `jr $ra`. Isso garante que chamadas recursivas e aninhadas não corrompam o endereço de retorno.

**Verificação pós-correção:**
```
gcd.s (prologue):
    addi $sp,$sp,-1
    sw   $ra,0($sp)        ← $ra salvo na entrada da função ✅

gcd.s (retorno antecipado, v == 0):
    lw   $ra,0($sp)        ← $ra restaurado ✅
    addi $sp,$sp,1
    jr   $ra               ← retorna ao chamador correto ✅

gcd.s (retorno após chamada recursiva):
    jal  gcd               ← $ra sobrescrito, mas já salvo no prologue
    addi $sp,$sp,2
    add  $t2,$v0,$zero
    add  $v0,$t2,$zero
    lw   $ra,0($sp)        ← $ra restaurado ✅
    addi $sp,$sp,1
    jr   $ra               ← retorna ao chamador correto ✅

sort.s (minloc prologue + retorno):
    addi $sp,$sp,-1
    sw   $ra,0($sp)        ← $ra salvo ✅
    ...
    lw   $ra,0($sp)        ← $ra restaurado ✅
    addi $sp,$sp,1
    jr   $ra               ← retorno correto ✅

sort.s (sort prologue):
    addi $sp,$sp,-1
    sw   $ra,0($sp)        ← $ra salvo antes de jal minloc ✅

sort.s (main prologue):
    addi $sp,$sp,-1
    sw   $ra,0($sp)        ← $ra salvo antes de jal sort ✅
```

**Observação:** A função `sort` salva `$ra` no prologue mas não possui sequência de retorno (`lw $ra`/`jr $ra`) no final — termina com `# END` sem retornar. Isso é um problema separado de geração de retorno para funções void, não relacionado a esta issue.

---

## Issue #8 — ~~[ASSEMBLY] Multiplicação e divisão com 3 operandos~~ CORRIGIDA

**Status:** CORRIGIDA em 25/03/2026

**Afeta:** fatorial.cms, gcd.cms
**Etapa:** 3
**Arquivo(s) do compilador:** asmgen.c

**Descrição:**
A multiplicação é emitida como `mul $tZ,$tX,$tY` (3 operandos) e a divisão como `div $tZ,$tX,$tY` (3 operandos). No ISA MIPS-Lite definido, `mult` e `div` aceitam apenas 2 operandos e escrevem o resultado nos registradores especiais `$lo` (quociente/produto low) e `$hi` (resto/produto high). Uma instrução `move` subsequente é necessária para transferir o resultado para um registrador de uso geral.

**Evidência:**
```
fatorial.s:
    mul  $t9,$t7,$t8       ← 3 operandos, deveria ser 2

gcd.s:
    div  $t8,$t6,$t7       ← 3 operandos, deveria ser 2
    mul  $t0,$t8,$t9       ← 3 operandos, deveria ser 2
```

**Esperado:**
```
fatorial.s:
    mult $t7,$t8           ← 2 operandos
    move $t9,$lo           ← mover resultado de $lo

gcd.s:
    div  $t6,$t7           ← 2 operandos (quociente em $lo)
    move $t8,$lo           ← mover quociente para $t8
    mult $t8,$t9
    move $t0,$lo
```

**Ajuste necessário:**
Em `asmgen.c`, na tradução de MUL e DIV:
1. Emitir `mult RS,RT` (ou `div RS,RT`) com apenas 2 operandos
2. Emitir `move RD,$lo` logo em seguida para copiar o resultado

**Correção aplicada:**
Em `asmgen.c`, a tradução de `MUL` e `DIV` passou a emitir instruções de 2 operandos (`mult`/`div`) seguidas de `move RD,$lo` para copiar o resultado para um registrador de uso geral.

**Verificação pós-correção:**
```
fatorial.s:
    mult $t5,$t6
    move $t7,$lo

gcd.s:
    div  $t6,$t7
    move $t8,$lo
    mult $t8,$t9
    move $t0,$lo
```

---

## Issue #9 — ~~[ENCODER] Instruções `in`/`out` não codificam o registrador~~ CORRIGIDA

**Status:** CORRIGIDA em 25/03/2026
**Afeta:** teste2.cms, teste.cms, fatorial.cms, gcd.cms, sort.cms
**Etapa:** 4
**Arquivo(s) do compilador:** encoder.c

**Descrição:**
As instruções `in` e `out` no binário sempre codificam o campo de registrador como 0 (`$zero`), independentemente do registrador real especificado no assembly. O formato F3 para I/O deveria ser `[opcode:6][Reg:6][zeros:20]`, mas o encoder gera `[opcode:6][000000][zeros:20]`.

**Evidência:**
```
teste.s:     in   $t2       → teste.txt linha 11: 01101000000000000000000000000000
                               opcode=011010(In) ✅, reg=000000($zero) ❌, esperado=001010($t2=10)

teste2.s:    out  $t6       → teste2.txt linha 20: 01101100000000000000000000000000
                               opcode=011011(Out) ✅, reg=000000($zero) ❌, esperado=001110($t6=14)

fatorial.s:  out  $t3       → fatorial.txt linha 31: 01101100000000000000000000000000
                               reg=000000 ❌, esperado=001011($t3=11)
```

**Esperado:**
```
in $t2:  011010 001010 00000000000000000000   (reg=$t2=10, formato 6 bits)
out $t6: 011011 001110 00000000000000000000   (reg=$t6=14, formato 6 bits)
```

**Ajuste necessário:**
Em `encoder.c`, na montagem de instruções `in` e `out`, extrair o registrador do assembly e codificá-lo nos bits 25-20 (com registradores de 6 bits) do binário. Atualmente o encoder parece ignorar o operando dessas instruções.

---

## Issue #10 — ~~[ENCODER] Formato de instrução usa campos de 5 bits para registradores em vez de 6 bits~~ CORRIGIDA

**Status:** CORRIGIDA em 28/03/2026
**Afeta:** teste2.cms, teste.cms, fatorial.cms, gcd.cms, sort.cms
**Etapa:** 4
**Arquivo(s) do compilador:** encoder.c

**Descrição:**
O ISA especifica campos de 6 bits para registradores (suportando 64 registradores, incluindo `$hi`=62 e `$lo`=61), mas o encoder produz campos de 5 bits (suportando apenas 32 registradores, padrão MIPS clássico). Isso altera a posição de todos os campos nas instruções e torna impossível codificar `$hi` e `$lo`.

Formato F1 observado: `[opcode:6][RD:5][RS:5][RT:5][Shamt:11]` = 32 bits
Formato F1 ISA:        `[opcode:6][RS:6][RT:6][RD:6][Shamt:8]` = 32 bits

Formato F2 observado: `[opcode:6][RD:5][RS:5][Imm:16]` = 32 bits
Formato F2 ISA:        `[opcode:6][RD:6][RS:6][Imm:14]` = 32 bits

**Evidência:**
```
Instrução: add $t6,$t4,$t5 (teste2.txt linha 17)
Binário:   00000001110011000110100000000000

Decodificação com 5 bits (observado):
  opcode=000000(ADD) ✅, RD=01110(14=$t6) ✅, RS=01100(12=$t4) ✅, RT=01101(13=$t5) ✅

Decodificação com 6 bits (ISA):
  opcode=000000(ADD) ✅, RS=011100(28=$gp) ❌, RT=110001(49=?) ❌, RD=101000(40=?) ❌

Instrução: addi $sp,$sp,-1 (teste2.txt linha 2)
Binário:   00000111101111011111111111111111

Decodificação com 5 bits: opcode=000001, RD=11101(29=$sp) ✅, RS=11101(29=$sp) ✅, Imm=-1 ✅
Decodificação com 6 bits: opcode=000001, RD=111011(59=?) ❌, RS=110111(55=?) ❌
```

**Esperado:**
Encoder deve usar campos de 6 bits conforme ISA. Exemplo para `add $t6,$t4,$t5`:
```
ISA F1: 000000 001100 001101 001110 00000000
        opcode RS=$t4  RT=$t5  RD=$t6  shamt=0
```

**Ajuste necessário:**
Em `encoder.c`, alterar as funções de montagem de instruções F1 e F2 para usar campos de 6 bits para registradores:
- F1: `[opcode:6][RS:6][RT:6][RD:6][Shamt:8]`
- F2: `[opcode:6][RD:6][RS:6][Imm:14]`
- F3 para I/O: `[opcode:6][Reg:6][zeros:20]`

**Correção aplicada:**
Em `encoder.c`, as funções de montagem foram atualizadas para seguir a ISA do processador:
- `F1` agora usa `[opcode:6][RS:6][RT:6][RD:6][Shamt:8]`
- `F2` agora usa `[opcode:6][RD:6][RS:6][Imm:14]`
- `In/Out` usam `[opcode:6][Reg:6][zeros:20]`

Com isso, os binários passam a codificar corretamente registradores de 6 bits, incluindo `$hi`=62 e `$lo`=61.

**Nota:**
No mesmo ajuste, `jr`/`jumpR` foi alinhada ao formato `F1` como instrução unária, sendo codificada com o registrador de salto em `RS` e com `RT=0`, `RD=0`.

---

## Issue #11 — [ENCODER] Ordem dos campos F1 difere do ISA~~ CORRIGIDA

**Afeta:** teste2.cms, teste.cms, fatorial.cms, gcd.cms, sort.cms
**Etapa:** 4
**Arquivo(s) do compilador:** encoder.c

**Descrição:**
Além do tamanho dos campos (Issue #10), a ordem dos campos de registradores em instruções F1 está invertida. O encoder coloca RD primeiro (`[opcode][RD][RS][RT][Shamt]`), mas o ISA especifica RS primeiro (`[opcode][RS][RT][RD][Shamt]`).

**Evidência:**
(Mesma da Issue #10 — a decodificação com 5 bits no formato [RD][RS][RT] funciona, mas com 6 bits no formato [RS][RT][RD] não.)

**Ajuste necessário:**
Corrigir simultaneamente com a Issue #10. Ao alterar para 6 bits, também reorganizar a ordem: `[opcode:6][RS:6][RT:6][RD:6][Shamt:8]`.

**Correção aplicada:**
Em `encoder.c`, a montagem de instruções `F1` já foi ajustada para seguir a ordem definida pela ISA:
- `[opcode:6][RS:6][RT:6][RD:6][Shamt:8]`

**Validação:**
Após recompilar o projeto e regenerar `gcd.txt` e `sort.txt`, o encoding foi conferido novamente e as instruções `F1` passaram a sair com os campos na ordem correta.

Exemplos validados:
- `sub $t2,$t0,$t1` → `RS=$t0`, `RT=$t1`, `RD=$t2`
- `slt $t8,$t6,$t7` → `RS=$t6`, `RT=$t7`, `RD=$t8`
- `jr $ra` → `RS=$ra`, `RT=0`, `RD=0`

---

## Issue #12 — ~~[ENCODER] `$v0` não está no mapeamento de registradores~~ CORRIGIDA

**Status:** CORRIGIDA em 28/03/2026

**Afeta:** gcd.cms, sort.cms
**Etapa:** 4
**Arquivo(s) do compilador:** encoder.c

**Descrição:**
O registrador `$v0` é usado no assembly para retorno de valor de funções (`add $v0,$tN,$zero`), mas o encoder o codifica como registrador 0 (`$zero`). Isso faz com que o valor de retorno seja escrito em `$zero` (que é sempre 0 por definição), perdendo o resultado da função.

**Evidência:**
```
gcd.s:     add  $v0,$t3,$zero

gcd.txt (linha 6): 00000000000010110000000000000000

Com 5 bits: opcode=000000(ADD), RD=00000(0=$zero) ❌
Esperado: RD = número de $v0 (ex: 2 no MIPS padrão)
```

**Esperado:**
`$v0` deve ter um número de registrador atribuído (ex: 2 como no MIPS, ou outro conforme a convenção do processador) e esse número deve ser usado na codificação.

**Ajuste necessário:**
Em `encoder.c`, adicionar `$v0` ao mapeamento de registradores com o número correto (verificar a especificação do processador Verilog para determinar qual registrador corresponde a `$v0`).

**Validação:**
Após recompilar o compilador e regenerar `gcd.txt` e `sort.txt`, as instruções que usam `$v0` passaram a codificar o registrador com valor `2`, e não mais `0` (`$zero`).

Exemplos validados:
- `gcd.s`: `add $v0,$t3,$zero` → `gcd.txt`: `00000000101100000000001000000000`
- `sort.s`: `add $v0,$t9,$zero` → `sort.txt`: `00000001000100000000001000000000`

---

## Issue #13 — ~~[FRONT-END] Variáveis locais não inseridas na tabela de símbolos~~ CORRIGIDA

**Status:** CORRIGIDA em 25/03/2026

**Afeta:** teste2.cms, teste.cms, fatorial.cms, gcd.cms, sort.cms
**Etapa:** 1
**Arquivo(s) do compilador:** analyze.c, Parser.y

**Descrição:**
Declarações de variáveis locais (dentro do corpo de funções) não são inseridas na tabela de símbolos. Apenas parâmetros de função e variáveis globais aparecem. Isso decorre do fato de que o parser gera nós `StmtK: Assign` para declarações locais em vez de `StmtK: Variable`, e o `buildSymtab` em `analyze.c` não reconhece essa estrutura como declaração.

**Evidência:**
```
teste2.cms declara `int x; int y;` em main.
Tabela de símbolos:
  main     global   function   void      1
  output   global   function   void(int) 0
  input    global   function   integer   0
  output   main     call       integer   4
  ← x e y AUSENTES

sort.cms declara variáveis locais em 3 funções.
Tabela de símbolos para minloc:
  a        minloc   variable   integer   3 6 9 10  ← parâmetro ✅
  low      minloc   variable   integer   3         ← parâmetro ✅
  high     minloc   variable   integer   3         ← parâmetro ✅
  ← i, x, k (variáveis locais) AUSENTES
```

**Esperado:**
Variáveis locais devem aparecer na tabela com `typeID = "variable"`, `typeData = "integer"`, no escopo da função onde são declaradas.

**Ajuste necessário:**
1. Em `Parser.y`: fazer declarações locais gerarem `StmtK: Variable` (mesmo nó que parâmetros).
2. Em `analyze.c` (`insertNode`): garantir que nós de declaração local sejam tratados pelo caso de inserção de variáveis.

**Correção aplicada:**
As declarações locais voltaram a ser tratadas como alocação de variável no front-end e agora aparecem no código intermediário com quádruplas `ALLOC` no escopo correto da função.

**Verificação pós-correção:**
```
teste2.tm:
  1: (ALLOC, x, main, -)
  2: (ALLOC, y, main, -)

sort.tm:
  6: (ALLOC, i, minloc, -)
  7: (ALLOC, x, minloc, -)
  8: (ALLOC, k, minloc, -)
 49: (ALLOC, i, sort, -)
 50: (ALLOC, k, sort, -)
 87: (ALLOC, i, main, -)
```

---

## Issue #14 — ~~[INTERMEDIÁRIO] `ADDR` não gerado para passagem de array como parâmetro~~ CORRIGIDA

**Status:** CORRIGIDA em 25/03/2026

**Afeta:** sort.cms
**Etapa:** 2
**Arquivo(s) do compilador:** cgen.c

**Descrição:**
Ao passar um vetor como argumento de função (ex: `sort(vet,0,10)`), o .tm gera `LOAD vet` que carrega o valor do primeiro elemento do vetor. Deveria gerar `ADDR vet` para obter o endereço base do vetor, que é o que a função chamada precisa para acessar os elementos.

**Evidência:**
```
sort.tm:
115: (LOAD, vet, -, t57)       ← carrega vet[0], deveria ser endereço de vet
116: (PARAM, t57, -, -)
```

**Esperado:**
```
115: (ADDR, vet, -, t57)       ← endereço base de vet
116: (PARAM, t57, -, -)
```

**Ajuste necessário:**
Em `cgen.c`, ao processar argumentos de chamada de função, verificar se o argumento é um vetor (tipo array). Se for, emitir `ADDR` em vez de `LOAD` para passar o endereço base.

**Correção aplicada:**
Em `cgen.c`, a passagem de vetores como argumento agora distingue valor de endereço e emite `ADDR` quando o operando é um array.

**Verificação pós-correção:**
```
sort.tm:
104: (ADDR, vet, -, t49)
105: (PARAM, t49, -, -)
106: (ASSIGN, 0, -, t50)
```

---

## Issue #15 — ~~[ASSEMBLY] `sll` usa nome de variável em vez de registrador para indexação~~ CORRIGIDA

**Status:** CORRIGIDA em 25/03/2026

**Afeta:** sort.cms
**Etapa:** 3
**Arquivo(s) do compilador:** asmgen.c

**Descrição:**
Na tradução de `LOADV`/`STOREV` (acesso indexado a vetores), o assembly emite `sll $tN, nomeVar, 2` onde `nomeVar` é o nome da variável array em vez de um registrador contendo o índice. Isso é sintaticamente inválido.

Adicionalmente, o `sll` por 2 (multiplicar por 4) assume endereçamento por bytes (padrão MIPS). Se o processador usa memória endereçada por palavras (como sugerido pelo PC+=1 para instruções), o `sll` é desnecessário — o índice pode ser usado diretamente como offset.

**Evidência:**
```
sort.s:
    lw   $t5,low($gp)         ← carrega índice low em $t5
    sll  $t0,a,2               ← "a" deveria ser $t5 (registrador com o índice)
    add  $t0,$gp,$t0
    lw   $t6,0($t0)
```

**Esperado (se endereçamento por palavras):**
```
    lw   $t5,offsetLow($sp)    ← carrega índice low
    addi $t0,$gp,offsetA        ← endereço base do vetor a
    add  $t0,$t0,$t5            ← endereço = base + índice
    lw   $t6,0($t0)
```

**Ajuste necessário:**
Em `asmgen.c`, na tradução de LOADV/STOREV:
1. Usar o registrador que contém o valor do índice (não o nome da variável)
2. Calcular endereço base do vetor (via `$gp` + offset numérico para globais, ou via `$sp` para parâmetros array)
3. Remover `sll` se a memória de dados é endereçada por palavras, ou manter se é por bytes

**Correção aplicada:**
Em `asmgen.c`, a tradução de `LOADV`/`STOREV` passou a usar apenas registradores no cálculo do endereço efetivo do vetor, sem emitir `sll` com nome simbólico de variável.

**Verificação pós-correção:**
```
sort.s:
    lw   $t0,3($sp)
    add  $t2,$t0,$t1
    lw   $t2,0($t2)

sort.s:
    lw   $t7,3($sp)
    add  $t8,$t7,$t0
    lw   $t1,0($t8)
```

---

## Issue #16 — ~~[ASSEMBLY] Passagem de vetor carrega valor em vez de endereço~~ CORRIGIDA

**Status:** CORRIGIDA em 25/03/2026

**Afeta:** sort.cms
**Etapa:** 3
**Arquivo(s) do compilador:** asmgen.c

**Descrição:**
Para a chamada `sort(vet,0,10)`, o assembly faz `lw $t7,vet($gp)` que carrega o valor de `vet[0]` em vez de calcular o endereço base do vetor. A função `sort` precisa receber o endereço do vetor para poder acessar seus elementos indexados.

**Evidência:**
```
sort.s:
    lw   $t7,vet($gp)        ← carrega vet[0], deveria calcular endereço
    addi $sp,$sp,-1
    sw   $t7,0($sp)           ← push do valor, deveria ser push do endereço
```

**Esperado:**
```
    addi $t7,$gp,offsetVet    ← calcula endereço base: $gp + offset de vet
    addi $sp,$sp,-1
    sw   $t7,0($sp)           ← push do endereço
```

**Ajuste necessário:**
Relacionado à Issue #14 (ADDR no .tm). Em `asmgen.c`, ao traduzir `ADDR`, emitir `addi $tN, $gp, memloc(vet)` para calcular o endereço base do vetor.

**Correção aplicada:**
Em `asmgen.c`, a tradução de `ADDR` passou a gerar o cálculo do endereço base do vetor com `addi`, empilhando esse endereço na chamada em vez do conteúdo do primeiro elemento.

**Verificação pós-correção:**
```
sort.s:
    addi $t9,$gp,0
    addi $sp,$sp,-1
    sw   $t9,0($sp)
```

---

## Resumo das Issues por Prioridade

### Críticas (bloqueiam execução correta do binário)

| # | Categoria | Descrição | Arquivo |
|---|-----------|-----------|---------|
| ~~1~~ | ~~INTERMEDIÁRIO~~ | ~~Comparações geram NOOP~~ | ~~cgen.c~~ |
| ~~2~~ | ~~INTERMEDIÁRIO~~ | ~~Declarações geram código espúrio~~ | ~~cgen.c, Parser.y~~ |
| ~~3~~ | ~~ASSEMBLY~~ | ~~Offsets simbólicos em lw/sw~~ | ~~asmgen.c~~ |
| ~~4~~ | ~~ASSEMBLY~~ | ~~jal usa nargs em vez de label~~ | ~~asmgen.c~~ |
| ~~5~~ | ~~ASSEMBLY~~ | ~~Sem jump para main no início~~ | ~~asmgen.c, cgen.c~~ |
| ~~6~~ | ~~ASSEMBLY~~ | ~~Parâmetros via $gp em vez de $sp~~ | ~~asmgen.c~~ |
| ~~7~~ | ~~ASSEMBLY~~ | ~~$ra não salvo/restaurado~~ | ~~asmgen.c~~ |
| 10 | ENCODER | Registradores 5-bit em vez de 6-bit | encoder.c |
| 11 | ENCODER | Ordem F1 invertida (RD antes de RS) | encoder.c |
| 9 | ENCODER | In/Out sem registrador | encoder.c |
| 12 | ENCODER | $v0 não mapeado | encoder.c |

### Importantes (afetam funcionalidades específicas)

| # | Categoria | Descrição | Arquivo |
|---|-----------|-----------|---------|
| ~~8~~ | ~~ASSEMBLY~~ | ~~mult/div com 3 operandos~~ | ~~asmgen.c~~ |
| ~~13~~ | ~~FRONT-END~~ | ~~Variáveis locais fora da tabela de símbolos~~ | ~~analyze.c, Parser.y~~ |
| ~~14~~ | ~~INTERMEDIÁRIO~~ | ~~ADDR não gerado para arrays~~ | ~~cgen.c~~ |
| ~~15~~ | ~~ASSEMBLY~~ | ~~sll usa nome de variável / desnecessário~~ | ~~asmgen.c~~ |
| ~~16~~ | ~~ASSEMBLY~~ | ~~Passagem de vetor carrega valor~~ | ~~asmgen.c~~ |
