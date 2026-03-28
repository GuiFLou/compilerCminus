# Plano de Testes — Compilador C‑ → MIPS‑Lite

Este documento é um roteiro completo para um **agente de teste** executar, validar e documentar cada estágio do compilador para todos os programas de exemplo. O objetivo final é produzir um relatório `ISSUES.md` com os ajustes necessários no compilador focados em apenas >BUGs<.

---

## Sumário

1. [Preparação do ambiente](#1-preparação-do-ambiente)
2. [Estrutura de logs](#2-estrutura-de-logs)
3. [Testes por arquivo](#3-testes-por-arquivo)
   - 3.1 [teste2.cms](#31-teste2cms)
   - 3.2 [teste.cms](#32-testecms)
   - 3.3 [fatorial.cms](#33-fatorialcms)
   - 3.4 [gcd.cms](#34-gcdcms)
   - 3.5 [sort.cms](#35-sortcms)
4. [Critérios de validação por etapa](#4-critérios-de-validação-por-etapa)
5. [Geração do relatório ISSUES.md](#5-geração-do-relatório-issuesmd)

---

## 1. Preparação do ambiente

### 1.1 Pré-requisito: compilador já compilado

Este plano **não cobre o build**. Assuma que o binário `compilador` (ou `compilador.exe` no Windows) **já foi gerado manualmente** no MSYS2 sempre que houver alteração no código.

### 1.2 Comando de compilação para cada teste

Para cada arquivo `exemplos/<nome>.cms`, executar:

```bash
./compilador --txt exemplos/<nome>.cms
```

Isso gera três arquivos no mesmo diretório do `.cms`:

| Arquivo           | Conteúdo                                |
|-------------------|-----------------------------------------|
| `<nome>.tm`       | Código intermediário (quádruplas)        |
| `<nome>.s`        | Assembly MIPS‑Lite                      |
| `<nome>.txt`      | Binário 32 bits (texto, uma linha/word) |

Além disso, o **terminal (stdout)** exibirá a saída do **módulo analítico**: árvore sintática, tabela de símbolos e verificação de tipos.

### 1.3 Convenção de log

Para cada arquivo testado, o agente deve criar um log estruturado com quatro seções, uma por etapa dentro de Testes/logs.md. Use o formato abaixo:

```
=== LOG: <nome>.cms ===

--- ETAPA 1: Módulo Analítico (stdout) ---
STATUS: OK | PROBLEMA
OBSERVAÇÕES:
  - ...

--- ETAPA 2: Código Intermediário (<nome>.tm) ---
STATUS: OK | PROBLEMA
OBSERVAÇÕES:
  - ...

--- ETAPA 3: Assembly (<nome>.s) ---
STATUS: OK | PROBLEMA
OBSERVAÇÕES:
  - ...

--- ETAPA 4: Binário (<nome>.txt) ---
STATUS: OK | PROBLEMA
OBSERVAÇÕES:
  - ...
```

---

## 2. Estrutura de logs

O agente deve manter um log cumulativo durante toda a execução. Ao final, esse log será a base para gerar o `ISSUES.md`.

Cada observação deve conter:

- **Arquivo de origem** (qual `.cms`)
- **Etapa** (1, 2, 3 ou 4)
- **Descrição do problema** (o que está errado)
- **Evidência** (trecho do output, linha do `.tm`/`.s`/`.txt` que demonstra o problema)
- **Expectativa** (o que deveria estar ali)

---

## 3. Testes por arquivo

Os testes estão ordenados do mais simples ao mais complexo. Essa ordem é intencional: problemas em exemplos simples ajudam a isolar falhas fundamentais antes de avaliar funcionalidades avançadas.

---

### 3.1 teste2.cms

**Código fonte:**
```c
void main(void)
{   int x; int y;
    x = 5; y = 3;
    output(x + y);
}
```

**Comportamento esperado:** calcula `5 + 3 = 8` e chama `output(8)`.

**Complexidade:** mínima — apenas variáveis locais, atribuição de literais, soma e chamada de output.

#### Etapa 1 — Módulo Analítico (stdout)

Executar `./compilador --txt exemplos/teste2.cms` e capturar toda a saída do terminal.

Verificar:
- [ ] A árvore sintática contém a declaração da função `main` com tipo `void`
- [ ] Declaração de duas variáveis `x` e `y` do tipo `int`
- [ ] Statements de atribuição: `x = 5`, `y = 3`
- [ ] Expressão `x + y` como argumento de `output`
- [ ] Chamada de `output` presente
- [ ] Tabela de símbolos lista: `main` (function, void, scope global), `x` (variable, integer, scope main), `y` (variable, integer, scope main), `output` (function/built-in, void)
- [ ] Nenhum erro semântico reportado
- [ ] Nenhum erro sintático reportado

#### Etapa 2 — Código Intermediário (teste2.tm)

Ler o arquivo `exemplos/teste2.tm`.

Verificar:
- [ ] Presença de `FUN void main -`
- [ ] `ALLOC x main -` e `ALLOC y main -` (ou equivalente)
- [ ] `ASSIGN 5 - tN` seguido de `STORE tN - x`
- [ ] `ASSIGN 3 - tN` seguido de `STORE tN - y`
- [ ] `LOAD x - tN` e `LOAD y - tN` para carregar os valores
- [ ] `ADD tX tY tZ` para a soma
- [ ] `PARAM tZ` e `CALL_O` (ou `CALL output 1 -`) para a saída
- [ ] `END main - -`
- [ ] `HALT - - -` ao final
- [ ] Todas as quádruplas fazem sentido em relação ao código fonte
- [ ] Temporários (`t0`, `t1`, ...) são usados de forma consistente

#### Etapa 3 — Assembly (teste2.s)

Ler o arquivo `exemplos/teste2.s`.

Verificar:
- [ ] Label `main:` presente
- [ ] Instruções `addi` para carregar literais 5 e 3 em registradores (`addi $tN, $zero, 5`)
- [ ] Instruções `sw` para armazenar em `x` e `y` via `$gp` (ex: `sw $tN, offset($gp)`)
- [ ] Instruções `lw` para carregar `x` e `y` de volta
- [ ] Instrução `add` para somar
- [ ] Instrução `out` para output do resultado
- [ ] Instrução `hlt` ao final
- [ ] Os offsets usados em `lw`/`sw` são numéricos (não nomes de variáveis) — se forem nomes, isso é um problema conhecido (ver CONTEXT.md §4.2)
- [ ] Nenhuma instrução referencia registradores inexistentes ou fora da convenção

#### Etapa 4 — Binário (teste2.txt)

Ler o arquivo `exemplos/teste2.txt`.

Verificar:
- [ ] Cada linha tem exatamente 32 caracteres (0 e 1)
- [ ] Nenhuma linha contém caracteres que não sejam `0` ou `1`
- [ ] Decodificar manualmente ao menos 3-5 instruções e conferir:
  - O opcode (bits 31-26) corresponde à instrução esperada conforme a tabela ISA:
    - `ADD` = `000000`, `ADDI` = `000001`, `SUB` = `000010`, `LOAD/lw` = `001111`, `STORE/sw` = `010000`, `JUMP/j` = `010001`, `JAL` = `010011`, `BEQ` = `010100`, `MOVE` = `010110`, `HLT` = `011000`, `OUT` = `011011`
  - Os campos de registradores (6 bits cada) correspondem ao mapeamento esperado:
    - `$zero` = 0, `$t0`-`$t9` = 8-17, `$gp` = 28, `$sp` = 29, `$ra` = 31, `$lo` = 61, `$hi` = 62
  - Campos de imediato (14 bits, formato F2) estão corretos para os valores utilizados
  - A instrução `hlt` está presente (`011000` + 26 zeros)
  - A instrução `out` usa o formato correto (F3: 6 bits opcode + 6 bits registrador + 20 bits zeros)
- [ ] O número total de instruções parece razoável para o programa (estimativa: ~10-20 instruções)
- [ ] Não há linhas vazias ou malformadas

---

### 3.2 teste.cms

**Código fonte:**
```c
void main(void)
{   int x; int y;
    x = input(); y = input();
    output(x + y);
}
```

**Comportamento esperado:** lê dois valores via `input()`, soma, e chama `output()` com o resultado.

**Complexidade:** baixa — adiciona `input()` (built-in) ao cenário anterior.

#### Etapa 1 — Módulo Analítico (stdout)

Executar `./compilador --txt exemplos/teste.cms` e capturar a saída.

Verificar:
- [ ] Árvore sintática mostra `main` com duas variáveis e chamadas de `input()`
- [ ] Chamadas de `input()` aparecem como expressão de atribuição
- [ ] Chamada de `output(x + y)` presente
- [ ] Tabela de símbolos lista `input` (function, integer) e `output` (function, void) como built-ins
- [ ] `x` e `y` declarados como variáveis inteiras no escopo de `main`
- [ ] Nenhum erro sintático ou semântico

#### Etapa 2 — Código Intermediário (teste.tm)

Ler `exemplos/teste.tm`.

Verificar:
- [ ] `CALL_I` (ou equivalente para input) aparece duas vezes
- [ ] O resultado de cada `CALL_I` é armazenado (`STORE`) em `x` e `y` respectivamente
- [ ] `LOAD x` e `LOAD y` antes da soma
- [ ] `ADD` para a soma
- [ ] `CALL_O` (ou equivalente para output) com o resultado da soma
- [ ] Estrutura geral coerente: FUN → ALLOC → body → END → HALT

#### Etapa 3 — Assembly (teste.s)

Ler `exemplos/teste.s`.

Verificar:
- [ ] Instrução `in` presente (para `input()`) — deve aparecer duas vezes
- [ ] O registrador de destino do `in` é válido (ex: `$tN`)
- [ ] Após cada `in`, o valor é armazenado com `sw` em `x`/`y` via `$gp`
- [ ] Carregamentos, soma e `out` seguem o mesmo padrão de teste2
- [ ] `hlt` ao final

#### Etapa 4 — Binário (teste.txt)

Ler `exemplos/teste.txt`.

Verificar:
- [ ] Formato correto (32 bits por linha, apenas 0/1)
- [ ] Decodificar a instrução `in`:
  - Opcode deve ser `011010`
  - Formato F3: `011010` + 6 bits registrador destino + 20 bits zeros
- [ ] Decodificar a instrução `out`:
  - Opcode deve ser `011011`
  - Formato F3: `011011` + 6 bits registrador fonte + 20 bits zeros
- [ ] Demais instruções consistentes com o assembly

---

### 3.3 fatorial.cms

**Código fonte:**
```c
/* calcula 5! = 120 e imprime em r0 */
void main(void) {
    int n;
    int result;
    n = 5;
    result = 1;
    while (n > 0) {
        result = result * n;
        n = n - 1;
    }
    output(result);
}
```

**Comportamento esperado:** calcula `5! = 120` e chama `output(120)`.

**Complexidade:** média — introduz `while`, comparação `>`, multiplicação `*` e subtração `-`.

#### Etapa 1 — Módulo Analítico (stdout)

Executar `./compilador --txt exemplos/fatorial.cms` e capturar a saída.

Verificar:
- [ ] Árvore sintática inclui o `while` com condição `n > 0`
- [ ] Corpo do while contém atribuições com expressões `result * n` e `n - 1`
- [ ] `output(result)` após o while
- [ ] Tabela de símbolos com `n` e `result` no escopo de `main`
- [ ] `output` reconhecido como built-in ou declaração válida
- [ ] Nenhum erro

#### Etapa 2 — Código Intermediário (fatorial.tm)

Ler `exemplos/fatorial.tm`.

Verificar:
- [ ] Labels para o loop while (ex: `L0`, `L1`) — um no início da condição e outro após o corpo
- [ ] Condição: LOAD de `n`, comparação com 0 (possivelmente via `slt` ou subtração), `IFF` para label de saída
- [ ] Corpo do loop:
  - LOAD `result` e `n`, `MUL`, STORE em `result`
  - LOAD `n`, `ASSIGN 1`, `SUB`, STORE em `n`
- [ ] `GOTO` de volta ao início do loop
- [ ] Label de saída do loop
- [ ] LOAD `result`, `CALL_O`
- [ ] Verificar se o fluxo de controle (IFF, GOTO, LAB) está correto para um while

**Atenção especial à multiplicação:**
- [ ] A quádrupla `MUL` está presente
- [ ] O .tm gera `MUL a b dst` (a multiplicação deve funcionar via `mult` + `move $lo` no assembly)

#### Etapa 3 — Assembly (fatorial.s)

Ler `exemplos/fatorial.s`.

Verificar:
- [ ] Labels de loop presentes (ex: `L0:`, `L1:`)
- [ ] Condição do while: comparação de `n` com 0 e branch condicional (`beq` ou `bne`)
- [ ] **Multiplicação:** deve usar `mult $tX, $tY` seguido de `move $tZ, $lo` (NÃO `mult $tX, $tY, $tZ` com 3 operandos — isso não existe na ISA)
- [ ] Subtração: `sub` ou `subi` para `n - 1`
- [ ] `j` (jump) para voltar ao início do loop
- [ ] `out` com o resultado final
- [ ] `hlt`
- [ ] Offsets numéricos em `lw`/`sw` (não nomes de variáveis)

#### Etapa 4 — Binário (fatorial.txt)

Ler `exemplos/fatorial.txt`.

Verificar:
- [ ] Formato correto (32 bits, 0/1)
- [ ] **Importante (PC no .txt):** como o binário final é **uma instrução (word) por linha** no `.txt`, o processador/simulador deve avançar o PC como **PC ← PC + 1** para ir à próxima instrução (índice da próxima linha). No MIPS tradicional, o PC avança como **PC ← PC + 4** (bytes), mas aqui a memória de instrução está organizada por *words* indexadas por linha.
- [ ] Decodificar a instrução `mult`:
  - Opcode: `000100` (Mult)
  - Formato F1: opcode(6) + RS(6) + RT(6) + RD(6) + Shamt(8) — RD deve ser 0 (resultado em High/Low)
- [ ] Decodificar `move` após mult:
  - Opcode: `010110` (move)
  - Deve mover de `$lo` (reg 61) para o registrador destino
- [ ] Decodificar ao menos um branch (`beq`/`bne`):
  - Opcode: `010100` ou `010101`
  - Formato F2: campo imediato (14 bits) com offset relativo correto
- [ ] Decodificar ao menos um jump (`j`):
  - Opcode: `010001`
  - Formato F3: campo endereço (26 bits) apontando para a label correta
- [ ] Total de instruções razoável (~20-35 instruções)

---

### 3.4 gcd.cms

**Código fonte:**
```c
/* Um programa para calcular o mdc
   segundo o algoritmo de Euclides. */
int gcd (int u, int v)
{
    if (v == 0) return u ;
    else return gcd(v, u-u/v*v);
    /* u-u/v*v == u mod v */
}

void main(void)
{   int x; int y;
    x = input(); y = input();
    output(gcd(x,y));
}
```

**Comportamento esperado:** lê dois inteiros, calcula o MDC via Euclides recursivo, e imprime.

**Complexidade:** alta — introduz **recursão**, **chamada de função com parâmetros**, **divisão**, **retorno de valor**, **if-else**.

#### Etapa 1 — Módulo Analítico (stdout)

Verificar:
- [ ] Declaração de `gcd` como função `int` com parâmetros `u` (int) e `v` (int)
- [ ] Declaração de `main` como função `void`
- [ ] Árvore sintática inclui `if` com condição `v == 0`
- [ ] Ramo `then`: `return u`
- [ ] Ramo `else`: `return gcd(v, u-u/v*v)`
- [ ] Expressão `u-u/v*v` corretamente parseada (precedência: divisão e multiplicação antes da subtração)
- [ ] Tabela de símbolos: `gcd` (function, integer, global), `u` e `v` (variable, integer, scope gcd), `x` e `y` (variable, integer, scope main)
- [ ] Built-ins `input` e `output` presentes
- [ ] Nenhum erro

#### Etapa 2 — Código Intermediário (gcd.tm)

Ler `exemplos/gcd.tm`.

Verificar:
- [ ] `FUN int gcd -` com `ARG u` e `ARG v` (ou equivalente)
- [ ] Condição: LOAD `v`, comparação com 0, `IFF` para o else
- [ ] Ramo then: LOAD `u`, `RET u`
- [ ] Ramo else: expressão `u-u/v*v`:
  - LOAD `u`, LOAD `v`
  - `DIV u v → tN` (quociente)
  - `MUL tN v → tM`
  - `SUB u tM → tK` (resto)
  - `PARAM v`, `PARAM tK`
  - `CALL gcd 2 tR`
  - `RET tR`
- [ ] `FUN void main -` com `CALL_I` para input, `CALL gcd`, `CALL_O`
- [ ] `END gcd`, `END main`, `HALT`
- [ ] Fluxo de labels correto para if-else

**Atenção especial:**
- [ ] A expressão `u/v*v` respeita a precedência correta (da esquerda para a direita: `(u/v)*v`)
- [ ] A quádrupla DIV está presente e o resto é calculado como `u - (u/v)*v`

#### Etapa 3 — Assembly (gcd.s)

Ler `exemplos/gcd.s`.

Verificar:
- [ ] Label `gcd:` presente
- [ ] **Parâmetros na pilha:** acessos a `u` e `v` devem usar offsets relativos a `$sp` (NÃO `$gp`) — ver CONTEXT.md §10
- [ ] **Divisão:** `div $tX, $tY` (2 operandos) seguido de `move $tZ, $lo` (quociente)
- [ ] **Multiplicação:** `mult $tX, $tY` seguido de `move $tZ, $lo`
- [ ] **Chamada recursiva:**
  - Save de `$ra` antes do `jal` (`sw $ra, ...($sp)`)
  - Push dos argumentos na pilha (`addi $sp, $sp, -1` + `sw`)
  - `jal gcd`
  - Pop dos argumentos após retorno
  - Restore de `$ra`
- [ ] **Retorno:** `add $v0, ...` (ou `move`) + `jr $ra`
- [ ] Condição `v == 0`: `beq` com `$zero`
- [ ] `hlt` ao final

#### Etapa 4 — Binário (gcd.txt)

Ler `exemplos/gcd.txt`.

Verificar:
- [ ] Formato correto
- [ ] Decodificar `jal`:
  - Opcode: `010011`
  - Formato F3: 26 bits de endereço apontando para a posição de `gcd`
- [ ] Decodificar `jr` (JumpR):
  - Opcode: `010010`
  - Formato F1: RS = `$ra` (31)
- [ ] Decodificar `div`:
  - Opcode: `000110`
  - Formato F1: RS e RT preenchidos, RD = 0
- [ ] Decodificar `mult`:
  - Opcode: `000100`
  - Formato F1: RS e RT preenchidos, RD = 0
- [ ] Decodificar `move` após div/mult:
  - Opcode: `010110`
  - Movendo de `$lo` (61) para destino
- [ ] Verificar endereço de `jal gcd` — deve apontar para a instrução correta (posição do label `gcd` no binário)
- [ ] Instruções `in` e `out` com formato correto

---

### 3.5 sort.cms

**Código fonte:**
```c
int vet[ 10 ];

int minloc ( int a[], int low, int high )
{   int i; int x; int k;
    k = low;
    x = a[low];
    i = low + 1;
    while (i < high){
        if (a[i] < x){
            x = a[i];
            k = i;
        }
        i = i + 1;
    }
    return k;
}

void sort( int a[], int low, int high)
{   int i; int k;
    i = low;
    while (i < high-1){
        int t;
        k = minloc(a,i,high);
        t = a[k];
        a[k] = a[i];
        a[i] = t;
        i = i + 1;
    }
}

void main(void)
{
    int i;
    i = 0;
    while (i < 10){
        vet[i] = input();
        i = i + 1;
    }
    sort(vet,0,10);
    i = 0;
    while (i < 10){
        output(vet[i]);
        i = i + 1;
    }
}
```

**Comportamento esperado:** lê 10 inteiros, ordena por selection sort, imprime os 10 ordenados.

**Complexidade:** muito alta — introduz **vetor global**, **parâmetro array (passagem por endereço)**, **acesso indexado (LOADV/STOREV)**, **múltiplas funções**, **loops aninhados com if**.

#### Etapa 1 — Módulo Analítico (stdout)

Verificar:
- [ ] Declaração de vetor global `vet[10]` do tipo `int`
- [ ] Função `minloc` com parâmetros `int a[]`, `int low`, `int high` e retorno `int`
- [ ] Função `sort` com parâmetros `int a[]`, `int low`, `int high` e retorno `void`
- [ ] Função `main` com retorno `void`
- [ ] Variáveis locais de cada função corretamente no escopo
- [ ] Tabela de símbolos inclui `vet` (variable, integer, global, com tamanho 10)
- [ ] Parâmetros `a[]` marcados como array/vetor na tabela
- [ ] Acessos indexados `a[i]`, `a[low]`, `a[k]`, `vet[i]` reconhecidos
- [ ] Chamadas `minloc(a,i,high)` e `sort(vet,0,10)` sem erros
- [ ] Nenhum erro sintático ou semântico

#### Etapa 2 — Código Intermediário (sort.tm)

Ler `exemplos/sort.tm`.

Verificar:
- [ ] Três funções: `FUN int minloc`, `FUN void sort`, `FUN void main`
- [ ] `ALLOC vet global -` (vetor global com tamanho 10)
- [ ] **LOADV e STOREV:** quádruplas para acessos indexados:
  - `LOADV a index tN` para `a[i]`, `a[low]`, `a[k]`
  - `STOREV value index vet` para `vet[i] = input()`
- [ ] **Passagem de array:** quádrupla `ADDR vet - tN` (ou equivalente) antes de `PARAM tN` ao chamar `sort(vet,0,10)` e `output(vet[i])`
- [ ] `PARAM` para cada argumento das chamadas de `minloc` e `sort`
- [ ] `CALL minloc 3 tN` e `CALL sort 3 -`
- [ ] Estrutura de controle correta: loops while com IFF/GOTO/LAB, if com IFF/LAB
- [ ] `RET k` em minloc
- [ ] Fluxo geral coerente com o algoritmo de selection sort

#### Etapa 3 — Assembly (sort.s)

Ler `exemplos/sort.s`.

Verificar:
- [ ] Labels `minloc:`, `sort:`, `main:`
- [ ] **Parâmetros na pilha:** `a`, `low`, `high`, `i`, etc. acessados via `$sp` (ver CONTEXT.md §10)
- [ ] **Acesso a vetor global (`vet[i]`):**
  - Calcular endereço: `addi baseReg, $gp, memloc(vet)`, depois `add addr, baseReg, indexReg`
  - `lw`/`sw` com offset 0 a partir do endereço calculado
  - **NÃO** deve usar `sll` para multiplicar índice por 4 se endereçamento em palavras (ver CONTEXT.md §11)
- [ ] **Acesso a vetor parâmetro (`a[i]`):**
  - Carregar base de `a` a partir de `$sp` (base é o endereço passado pelo chamador)
  - Depois `add addr, baseReg, indexReg` e `lw`/`sw` com offset 0
- [ ] **ADDR:** instrução para calcular o endereço base do vetor (ex: `addi tN, $gp, memloc(vet)`)
- [ ] **Convenção de chamada completa:**
  - Save `$ra` antes de chamadas
  - Push de argumentos
  - `jal`
  - Pop de argumentos
  - Restore `$ra`
  - Resultado em `$v0`
- [ ] Comparações `<` implementadas via `slt` + `beq`/`bne`
- [ ] `hlt` ao final

#### Etapa 4 — Binário (sort.txt)

Ler `exemplos/sort.txt`.

Verificar:
- [ ] Formato correto (32 bits, 0/1)
- [ ] Tamanho razoável para o programa (~60-120 instruções)
- [ ] Decodificar ao menos 5 instruções representativas:
  - Um `lw` (Load, opcode `001111`): verificar formato F2, registradores e imediato
  - Um `sw` (Store, opcode `010000`): verificar formato F2
  - Um `jal` (opcode `010011`): verificar endereço de destino
  - Um `jr` (JumpR, opcode `010010`): verificar RS = `$ra`
  - Um `beq` ou `bne`: verificar offset relativo
  - Um `in` e um `out`: verificar formato F3
  - Um `slt`: verificar formato F1
- [ ] Não há instruções com opcode desconhecido (todos os 6 bits iniciais devem corresponder a uma instrução válida da ISA)
- [ ] Endereços de `jal` apontam para posições corretas dos labels

---

## 4. Critérios de validação por etapa

### Etapa 1 — Módulo Analítico

| Critério | Descrição |
|----------|-----------|
| **Árvore sintática completa** | Todas as declarações, statements e expressões do `.cms` estão representados |
| **Tipos corretos** | Variáveis `int`, funções `void`/`int`, parâmetros tipados corretamente |
| **Escopos corretos** | Cada variável/parâmetro está no escopo da sua função; globais no escopo global |
| **Sem erros falsos** | Nenhum erro semântico/sintático em código válido |
| **Built-ins reconhecidos** | `input` e `output` presentes na tabela de símbolos |
| **Precedência** | Expressões como `u-u/v*v` respeitam precedência (multiplicação/divisão antes de soma/subtração) |

### Etapa 2 — Código Intermediário (.tm)

| Critério | Descrição |
|----------|-----------|
| **Cobertura completa** | Todas as funções e statements do fonte têm quádruplas correspondentes |
| **Temporários consistentes** | `t0`, `t1`, ... usados sem conflito |
| **Labels corretos** | `IFF`/`GOTO` apontam para labels que existem; `LAB` define cada label usado |
| **Chamadas corretas** | `PARAM` na quantidade certa antes de `CALL`; `CALL` com o número correto de argumentos |
| **Retorno correto** | Funções `int` têm `RET valor`; funções `void` têm `RET - - -` ou nenhum RET explícito |
| **HALT presente** | `HALT - - -` é a última quádrupla |
| **ALLOC global** | Variáveis globais (como `vet[10]`) alocadas antes das funções |

### Etapa 3 — Assembly (.s)

| Critério | Descrição |
|----------|-----------|
| **Instruções válidas** | Todos os mnemônicos pertencem ao ISA definido em CONTEXTO_PROCESSADOR.md |
| **Registradores válidos** | Apenas registradores do banco de 64 ($zero, $t0-$t9, $gp, $sp, $ra, $v0, $hi, $lo) |
| **Offsets numéricos** | `lw`/`sw` usam offsets numéricos, não nomes de variáveis |
| **Parâmetros via $sp** | Parâmetros de função acessados via $sp, não $gp |
| **Variáveis globais/locais via $gp** | Variáveis não-parâmetro acessadas via offset($gp) |
| **Div/Mult corretos** | Usam 2 operandos + `move` de `$lo`; nunca 3 operandos |
| **Convenção de chamada** | Save/restore $ra, push/pop args, jal, jr $ra |
| **Controle de fluxo** | Labels definidos antes de uso; branches e jumps apontam para labels corretos |

### Etapa 4 — Binário (.txt)

| Critério | Descrição |
|----------|-----------|
| **Formato válido** | 32 caracteres por linha, apenas `0` e `1` |
| **Opcodes corretos** | Bits 31-26 de cada instrução correspondem ao ISA |
| **Formatos corretos** | F1 (register), F2 (immediate), F3 (jump/io) usados conforme o tipo da instrução |
| **Registradores corretos** | Campos de 6 bits mapeiam para os registradores esperados |
| **Imediatos corretos** | Valores imediatos (14 bits F2) estão corretos e com extensão de sinal quando negativo |
| **Endereços de salto** | `jal`/`j` apontam para endereços corretos; branches têm offset relativo correto |
| **Coerência asm↔bin** | Cada linha do .txt corresponde exatamente a uma instrução do .s, na mesma ordem |
| **HLT presente** | Última instrução é `hlt` (`011000` + 26 zeros) |

---

## 5. Geração do relatório ISSUES.md

Após completar **todos** os testes acima, o agente deve:

### 5.1 Consolidar os logs

Reunir todas as observações de todas as etapas de todos os arquivos testados.

### 5.2 Classificar os problemas

Agrupar por tipo:

| Categoria | Exemplos |
|-----------|----------|
| **FRONT-END** | Erros na árvore sintática, tabela de símbolos incorreta, falsos erros semânticos |
| **INTERMEDIÁRIO** | Quádruplas faltando, temporários incorretos, fluxo de controle errado |
| **ASSEMBLY** | Instruções inválidas, offsets simbólicos em vez de numéricos, convenção de chamada errada |
| **ENCODER** | Opcodes errados, formatos errados, registradores mal mapeados, endereços de salto incorretos |
| **GERAL** | Crash, arquivo não gerado, output vazio |

### 5.3 Gerar o ISSUES.md

Criar o arquivo `ISSUES.md` na raiz do projeto com o seguinte formato:

```markdown
# Issues do Compilador C‑ → MIPS‑Lite

Data do teste: <data>
Arquivos testados: teste2.cms, teste.cms, fatorial.cms, gcd.cms, sort.cms

---

## Issue #1 — [CATEGORIA] Título curto

**Afeta:** <lista de .cms afetados>
**Etapa:** <1|2|3|4>
**Arquivo(s) do compilador:** <ex: asmgen.c, encoder.c>

**Descrição:**
<Explicação detalhada do problema encontrado>

**Evidência:**
<Trecho do output/arquivo que demonstra o problema>

**Esperado:**
<O que deveria aparecer/acontecer>

**Ajuste necessário:**
<O que exatamente precisa ser alterado no código do compilador>

---

## Issue #2 — ...
```

### 5.4 Regras para o relatório

1. **Enumerar todas as issues** — não omitir problemas, mesmo que pareçam menores
2. **Não duplicar** — se o mesmo problema aparece em vários `.cms`, agrupar numa única issue listando todos os afetados
3. **Ser específico** — indicar qual arquivo do compilador (`asmgen.c`, `encoder.c`, `cgen.c`, etc.) precisa do ajuste
4. **Priorizar** — listar primeiro os problemas que bloqueiam a geração correta do binário, depois os que afetam apenas a qualidade do output
5. **Referenciar o ISA** — para problemas no binário, citar os opcodes e formatos do CONTEXTO_PROCESSADOR.md

---

## Referência rápida — Tabela de opcodes do ISA

| Instrução | Opcode   | Formato | Instrução | Opcode   | Formato |
|-----------|----------|---------|-----------|----------|---------|
| ADD       | `000000` | F1      | Or        | `001010` | F1      |
| AddI      | `000001` | F2      | OrI       | `001011` | F2      |
| Sub       | `000010` | F1      | Not       | `001100` | F2      |
| SubI      | `000011` | F2      | Sr        | `001101` | F1      |
| Mult      | `000100` | F1      | Sl        | `001110` | F1      |
| Multi     | `000101` | F2      | Load      | `001111` | F2      |
| Div       | `000110` | F1      | Store     | `010000` | F2      |
| Divi      | `000111` | F2      | Jump      | `010001` | F3      |
| And       | `001000` | F1      | JumpR     | `010010` | F1      |
| AndI      | `001001` | F2      | Jal       | `010011` | F3      |
| beq       | `010100` | F2      | move      | `010110` | F2      |
| bne       | `010101` | F2      | nop       | `010111` | F3      |
| hlt       | `011000` | F3      | slt       | `011001` | F1      |
| In        | `011010` | F3      | Out       | `011011` | F3      |

## Referência rápida — Mapeamento de registradores

| Registrador | Número | Registrador | Número |
|-------------|--------|-------------|--------|
| `$zero`     | 0      | `$t5`       | 13     |
| `$t0`       | 8      | `$t6`       | 14     |
| `$t1`       | 9      | `$t7`       | 15     |
| `$t2`       | 10     | `$t8`       | 16     |
| `$t3`       | 11     | `$t9`       | 17     |
| `$t4`       | 12     | `$gp`       | 28     |
| `$sp`       | 29     | `$lo`       | 61     |
| `$ra`       | 31     | `$hi`       | 62     |

## Referência rápida — Formatos de instrução

**F1** (Register): `[opcode:6][RS:6][RT:6][RD:6][Shamt:8]` = 32 bits

**F2** (Immediate): `[opcode:6][RD:6][RS:6][Imm:14]` = 32 bits

**F3** (Jump/IO): `[opcode:6][Endereço:26]` = 32 bits
- Para In/Out: `[opcode:6][Reg:6][zeros:20]`
