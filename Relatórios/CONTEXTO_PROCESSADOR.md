# Contexto do processador MIPS_Lite para o compilador

Este documento resume o que o relatório `Relatórios/Relatório_Processador.tex` define sobre o processador `MIPS_Lite`, mas com foco no que interessa ao compilador, ao gerador de assembly e ao encoder.

---

## Destaques para os próximos passos do desenvolvimento

As três informações abaixo são **de extrema importância** para o desenvolvimento do backend (gerador de código, montador e encoder). Consulte as seções indicadas para os detalhes completos.

### Conjunto de Instruções (ISA)

- **Localização completa:** Seção 4.
- Instruções de **32 bits**, em três formatos: **F1** (RS, RT, RD, Shamt), **F2** (RD, RS, Imediato 14 bits), **F3** (Endereço 26 bits ou Opcode + registrador + 20 bits zerados para In/Out).
- **Opcode binário (6 bits) de cada instrução:**

| Instrução | Opcode (binário) | Instrução | Opcode (binário) |
|-----------|------------------|-----------|------------------|
| `ADD`     | `000000`         | `SubI`    | `000011`         |
| `AddI`    | `000001`         | `Mult`    | `000100`         |
| `Sub`     | `000010`         | `Multi`   | `000101`         |
| `Div`     | `000110`         | `Or`      | `001010`         |
| `Divi`    | `000111`         | `OrI`     | `001011`         |
| `And`     | `001000`         | `Not`     | `001100`         |
| `AndI`    | `001001`         | `Sr`      | `001101`         |
| `Sl`      | `001110`         | `Jal`     | `010011`         |
| `Load`    | `001111`         | `beq`     | `010100`         |
| `Store`   | `010000`         | `bne`     | `010101`         |
| `Jump`    | `010001`         | `move`    | `010110`         |
| `JumpR`   | `010010`         | `nop`     | `010111`         |
| `hlt`     | `011000`         | `In`      | `011010`         |
| `slt`     | `011001`         | `Out`     | `011011`         |

- **Restrições:** imediatos em 14 bits (F2); saltos absolutos em 26 bits (F3); `mult`/`multi`/`div`/`divi` escrevem em **High** e **Low**; `jal` salva `PC+1` em registrador de retorno.

### Acesso à memória pelas instruções

- **Localização completa:** Seções 3 (formatos), 4 (Load/Store) e 5 (modos de endereçamento).
- **Load** e **Store** usam formato **F2**: endereço efetivo = **base (RS) + deslocamento (imediato 14 bits)**. Em **Load**: RD recebe o valor lido da memória; em **Store**: registrador-base e registrador-fonte são **distintos** (RD e RS conforme o formato).
- Memória de **dados**: 1024 palavras de 32 bits; endereços válidos e constantes/offsets devem respeitar o limite dos 14 bits de imediato (com extensão de sinal).
- Memória de **instruções**: 1024 palavras; jumps usam endereço absoluto de 26 bits; branches usam endereçamento **relativo ao PC** com cálculo `extImm << 2`.
- Modos de endereçamento usados: registrador, imediato, **base + deslocamento** (load/store), relativo ao PC (branches), absoluto/pseudo-direto (jumps).

### Organização dos registradores

- **Localização completa:** Seção 6 (e itens 2 e 7).
- Banco de **64 registradores**; cada identificador usa **6 bits**. Não assumir a convenção de 32 registradores do MIPS clássico.
- **Registradores reservados (não usar como temporários gerais):**
  - **High** e **Low**: resultados de `mult`, `multi`, `div` e `divi`.
  - **Registrador de link/retorno** (último do banco): usado por `jal` para gravar `PC + 1`; essencial para chamadas de função e retorno.
- O alocador de registradores do compilador deve tratar High, Low e o registrador de link como reservados pelo hardware.

---

## 1. Visão geral

- O processador é um `MIPS` simplificado, de estilo `RISC`.
- A implementação é `monociclo`.
- A arquitetura é baseada em `Harvard`: memória de instruções separada da memória de dados.
- As instruções têm tamanho fixo de `32 bits`.
- Existem `3 formatos` de instrução.
- O banco de registradores tem `64 registradores`, então cada identificador de registrador ocupa `6 bits`.
- Além dos registradores gerais, há registradores de uso específico para operações especiais e controle de fluxo, incluindo `High`, `Low` e o registrador de retorno usado por `jal`.

## 2. Implicações diretas para o compilador

- O backend deve gerar instruções de `32 bits` exatamente nos formatos `F1`, `F2` e `F3`.
- Como há `64 registradores`, o montador e o gerador de código não devem assumir a convenção clássica de `32` registradores do MIPS tradicional.
- Instruções imediatas usam `14 bits` de imediato no formato `F2`; isso limita constantes inline e offsets.
- Saltos absolutos usam `26 bits` no formato `F3`.
- `mult`, `multi` e `div` escrevem em registradores especiais (`High` e `Low`), então o compilador precisa tratar essas instruções como produtoras de resultado especial, não em registrador geral.
- `jal` salva `PC + 1` em um registrador especial de procedimento/retorno, então chamadas de função dependem dessa convenção.
- `in` e `out` fazem parte da ISA; isso permite mapear built-ins como `input()` e `output()`.
- `hlt` existe como instrução real, então o programa final pode encerrar explicitamente.
- O hardware usa mais de dois registradores com papel especial, então o backend deve tratá-los como reservados.

## 3. Formatos de instrução

### F1

Usado para operações entre registradores e shifts.

| Bits | Campo |
|-----|-----|
| 31-26 | Opcode |
| 25-20 | RS |
| 19-14 | RT |
| 13-8 | RD |
| 7-0 | Shamt |

Para o compilador, deve-se seguir a interpretação do Verilog: `RS`, `RT`, `RD`, e não a ordem textual mostrada na tabela original do relatório.

### F2

Usado para imediato, acesso à memória e branches.

| Bits | Campo |
|---|---|
| 31-26 | Opcode |
| 25-20 | RD |
| 19-14 | RS |
| 13-0 | Imediato |

### F3

Usado para jumps, instruções de controle e I/O.

**Para Jump, Jal, Nop, Hlt (campo “Endereço”):**

| Bits | Campo |
|---|---|
| 31-26 | Opcode |
| 25-0 | Endereço |

**Para In e Out:** o relatório descreve F3 só com opcode e “Endereço / -”, mas a semântica exige um registrador. A interpretação correta para o compilador é tratar essas instruções como **6 bits opcode + 6 bits registrador + 20 bits não usados (zerados)**. O “valor lido nos switches” não entra na instrução: é uma entrada de hardware lida na execução; a instrução **In** apenas indica em qual registrador gravar esse valor. Da mesma forma, **Out** indica de qual registrador ler o valor a enviar para a saída.

## 4. Conjunto de instruções

| Instrução | Formato | Opcode | Semântica resumida |
|---|---|---|---|
| `ADD` | `F1` | `000000` | soma entre registradores |
| `AddI` | `F2` | `000001` | soma com imediato |
| `Sub` | `F1` | `000010` | subtração entre registradores |
| `SubI` | `F2` | `000011` | subtração com imediato |
| `Mult` | `F1` | `000100` | produto em `High/Low` |
| `Multi` | `F2` | `000101` | produto com imediato em `High/Low` |
| `Div` | `F1` | `000110` | resto em `High`, quociente em `Low` |
| `Divi` | `F2` | `000111` | divisão de registrador por imediato |
| `And` | `F1` | `001000` | AND lógico |
| `AndI` | `F2` | `001001` | AND com imediato |
| `Or` | `F1` | `001010` | OR lógico |
| `OrI` | `F2` | `001011` | OR com imediato |
| `Not` | `F2` | `001100` | NOT lógico |
| `Sr` | `F1` | `001101` | shift right com `shamt` |
| `Sl` | `F1` | `001110` | shift left com `shamt` |
| `Load` | `F2` | `001111` | leitura de memória |
| `Store` | `F2` | `010000` | escrita em memória com registrador-base e registrador-fonte distintos |
| `Jump` | `F3` | `010001` | salto incondicional |
| `JumpR` | `F1` | `010010` | salto para endereço em registrador |
| `Jal` | `F3` | `010011` | salto com link |
| `beq` | `F2` | `010100` | branch se iguais |
| `bne` | `F2` | `010101` | branch se diferentes |
| `move` | `F2` | `010110` | cópia entre registradores |
| `nop` | `F3` | `010111` | nenhuma operação |
| `hlt` | `F3` | `011000` | para a execução |
| `slt` | `F1` | `011001` | comparação "menor que" |
| `In` | `F3` | `011010` | entrada de dados |
| `Out` | `F3` | `011011` | saída de dados |

## 5. Modos de endereçamento relevantes

O relatório associa a ISA a modos de endereçamento próximos aos do MIPS:

- `Registrador`: operações aritméticas e lógicas entre registradores.
- `Imediato`: constantes embutidas na instrução.
- `Base + deslocamento`: `load` e `store`.
- `Relativo ao PC`: branches.
- `Absoluto / pseudo-direto`: jumps.

Para o compilador, isso sugere o seguinte:

- `if` e `while` podem ser implementados com `beq` e `bne`.
- acessos a variáveis e vetores podem ser mapeados para `load/store` com base + offset;
- chamadas e desvios incondicionais podem usar `jump`, `jal` e `jumpR`.

## 6. Organização do hardware que afeta o código gerado

### Memórias

- Memória de instruções: `1024 palavras de 32 bits`.
- Memória de dados: `1024 palavras de 32 bits`.

Isso sugere que o programa final e os dados devem caber nesse espaço, ou então o compilador/montador precisará validar limites.

### Program Counter

- O `PC` aponta para instruções na ROM.
- O caminho normal do fluxo faz `PC <- PC + 1`.
- Branch e jump substituem esse valor por um endereço calculado.

### ULA

- A ULA recebe operandos de registradores ou imediato.
- Existe sinal `Zero`, usado no controle de `beq` e `bne`.

### JAL e registrador de retorno

- O relatório afirma que `jal` grava `PC + 1` em um registrador específico.
- O texto sobre multiplexadores indica que o "último endereço" do banco de registradores foi reservado para isso.

Para o compilador, vale tratar esse registrador como reservado para retorno de chamada.

### Registradores especiais

- O processador não tem apenas dois registradores de uso específico.
- Para o backend, devem ser considerados reservados pelo menos `High`, `Low` e o registrador de link usado por `jal`.
- O alocador de registradores não deve usar esses registradores como temporários gerais.

### Entrada e saída

- O projeto possui suporte explícito a `input` por switches da FPGA.
- O valor de entrada pode seguir para registrador ou para memória, dependendo da lógica do sistema.
- A saída é exibida em portas/displays.

Isso reforça o mapeamento natural:

- `input()` -> `in`
- `output(x)` -> `out`

## 7. Convenções práticas recomendadas para o compilador

Enquanto o hardware e o encoder não forem revalidados juntos, vale assumir estas regras de projeto:

- reservar registradores especiais para `High`, `Low` e registrador de link de `jal`;
- evitar gerar `mult`, `multi`, `div` e `divi` até que o caminho de leitura de `High/Low` esteja claramente definido;
- preferir `AddI`, `SubI`, `move`, `load`, `store`, `beq`, `bne`, `jump`, `jal`, `jumpR`, `in`, `out`, `nop` e `hlt` no primeiro conjunto estável do backend;
- limitar imediatos ao intervalo representável em `14 bits`, com extensão de sinal;
- codificar branch de forma compatível com o hardware, considerando o cálculo com `extImm << 2`;
- padronizar como labels viram campo de `26 bits` em `F3`.

## 8. Decisões confirmadas para o backend

Estas definições devem ser tratadas como referência do compilador:

- No formato `F1`, a ordem correta dos campos para geração de código é `RS`, `RT`, `RD`, conforme o Verilog.

- A instrução `Store` deve ser tratada com registrador-base e registrador-fonte distintos.

- A instrução `Divi` deve ser entendida como divisão de um valor em registrador por um imediato, armazenando o resultado em outro registrador.

- Para `In` e `Out`, usar `6 bits de opcode + 6 bits de registrador + 20 bits zerados`. O valor dos switches não faz parte da instrução; ele é lido do hardware em tempo de execução.

- Há mais de dois registradores com papel especial no processador.

- O encoder deve gerar branches de forma compatível com o cálculo de desvio feito no hardware com `extImm << 2`.

## 9. Resumo operacional para o backend

Se o objetivo for fazer o compilador funcionar logo com esse processador, o backend deve assumir:

1. ISA fixa de `32 bits`, com formatos `F1/F2/F3`.
2. Arquitetura `Harvard`, com ROM de instruções e RAM de dados separadas.
3. Banco com `64 registradores`.
4. Suporte nativo a aritmética básica, branches, jumps, I/O e halt.
5. `assembly`, `encoder` e backend devem seguir as convenções já fixadas neste documento para registradores, immediatos, `store`, `in/out`, `jal` e branches.

## 10. Uso de $gp e recursão (parâmetros na pilha)

### Por que usar $gp para parâmetros quebra recursão

No compilador, o `asmgen.c` tratava a quádrupla **ARG** copiando cada parâmetro da pilha para uma posição fixa em memória relativamente a **$gp** (ex.: `1($gp)`, `2($gp)`). Variáveis locais e parâmetros compartilham o mesmo `memloc` da tabela de símbolos para **LOAD**/**STORE** via `$gp`.

Isso quebra recursão porque:

1. **Um único endereço por parâmetro:** Na tabela de símbolos, cada parâmetro (ex.: `u`, `v` de `gcd`) tem um único `memloc` (ex.: 1 e 2). Todas as ativações da função usam os mesmos offsets em `$gp` para esses nomes.
2. **Sobrescrita na chamada recursiva:** Na primeira ativação de `gcd(u=12, v=8)`, o prólogo (ARG) grava 12 em `1($gp)` e 8 em `2($gp)`. Se no meio do corpo a função chama `gcd(8, 4)`, o prólogo da nova ativação grava 8 e 4 em `1($gp)` e `2($gp)`, sobrescrevendo os valores da ativação anterior.
3. **Retorno à ativação anterior:** Quando a chamada interna retorna, a ativação externa continua usando LOAD/STORE em `1($gp)` e `2($gp)`, que agora contêm os valores da chamada interna (8 e 4), e não mais os da ativação externa (12 e 8). O resultado é incorreto.

Ou seja: parâmetros em **$gp** são efetivamente **globais por nome**; não há “cópia por ativação”, então recursão (e reentrância) quebram.

### Solução adotada: parâmetros só na pilha

- **Parâmetros não são mais copiados para $gp.** A quádrupla ARG apenas registra o nome do parâmetro na ordem de declaração; não se emite `lw`/`sw` para `memloc($gp)`.
- **Acesso a parâmetros:** Sempre por **offset em relação a $sp**. Na entrada da função, o chamador já empilhou `$ra` e os argumentos (último argumento no topo). O callee acessa o parâmetro de índice `i` (0 = primeiro) em `(paramCount - 1 - i + stackDelta)($sp)`, onde `stackDelta` é o número de palavras empilhadas pelo callee desde a entrada (ex.: antes de um `jal`, após salvar `$ra` e os argumentos da chamada).
- **Variáveis locais e globais:** Continuam em `memloc($gp)` como antes; apenas **parâmetros** da função atual passam a ser acessados via `$sp`.
- **Convenção de chamada:** Mantida: chamador faz `saveReturnRegister` (push `$ra`), depois um `pushArg` por argumento (em ordem), depois `jal`; após o retorno, `popArgs` e `restoreReturnRegister`. O callee não altera o significado dos offsets dos seus parâmetros até ele mesmo empilhar mais coisas (aí usa `stackDelta`).

Assim, cada ativação tem seus próprios argumentos na pilha; não há compartilhamento via $gp e a recursão passa a funcionar.

### Alterações no código

- **asmgen.c:**
  - Manter lista dos nomes dos parâmetros da função atual (preenchida ao processar ARG).
  - Para ARG: apenas registrar o parâmetro (e incrementar o índice); **não** emitir `lw`/`sw` para `$gp`.
  - Para LOAD/STORE: se o símbolo for parâmetro da função atual, emitir `lw`/`sw` com offset em `$sp` (usando `paramCount`, índice do parâmetro e `stackDelta`); caso contrário, manter `memloc($gp)`.
  - Atualizar `stackDelta` ao emitir `saveReturnRegister`, `pushArg`, `popArgs` e `restoreReturnRegister`.
- **cgen.c** e a convenção de chamada (quádruplas PARAM/CALL/RET): sem alteração necessária; a mudança é só na convenção de *implementação* no assembly (parâmetros só na pilha no asmgen).

## 11. Vetores e parâmetros array (ex.: sort.cms)

### Convenção no intermediário

- **Parâmetro array** (ex.: `int a[]`): passado **por endereço**. O chamador emite `ADDR arr, -, temp` e depois `PARAM temp`. O valor empilhado é o **endereço base** do vetor (em palavras, para memória word-addressable).
- **LOADV name, index, dst**: acessa `name[index]`. O primeiro operando é o nome do vetor (global, local ou parâmetro); o segundo é o índice (já em registrador).
- **STOREV value, index, name**: grava `value` em `name[index]`; o nome do vetor está em `result`.

### O que o asmgen precisa fazer

1. **ADDR** (atualmente “não implementado”): deve produzir o endereço base do vetor em um registrador. Para variável global/vetor no escopo atual: `addi dst, $gp, memloc` (o encoder usa base + imediato; assumindo endereçamento em palavras, `memloc` é o offset da primeira palavra do vetor).
2. **LOADV** quando o vetor é **parâmetro**: a base não está em `$gp`; está na pilha (o valor passado pelo chamador). Gerar: carregar a base a partir de `offset($sp)` (mesmo offset usado para LOAD do parâmetro), depois `base + index` em um temp, depois `lw dst, 0(addr)`. Assim cada ativação usa sua própria cópia do endereço e vetores como parâmetro funcionam com recursão.
3. **LOADV** quando o vetor **não** é parâmetro (global/local em `$gp`): base = `$gp + memloc(name)`. Gerar: `addi baseReg, $gp, memloc` (ou equivalente), depois `baseReg + index`, depois `lw dst, 0(addr)`. Não usar o nome do vetor como registrador no `sll` (isso gera assembly inválido quando o nome é parâmetro e quebra o caso sort/minloc).
4. **STOREV**: mesma lógica para a base: se `result` (nome do vetor) for parâmetro, base na pilha; senão base = `$gp + memloc`. Em seguida calcular endereço, depois `sw value, 0(addr)`.

Assumindo **endereçamento em palavras**: o índice já representa a posição em palavras; não é necessário `sll` para multiplicar por 4 nesse modelo. Se o ISA usar deslocamento em bytes, a base e o índice precisariam ser convertidos para bytes (ex.: base*4 + index*4).

## 13. Instrução Div e operação de resto (ISA High/Low)

### Diferença entre o ISA e o que o compilador gerava

- **ISA (processador):** A instrução **Div** é formato F1 com **dois operandos** (RS, RT). O resultado **não** vai para um registrador geral: o **resto** é escrito em **High** e o **quociente** em **Low**. Não existe “div rd, r1, r2” com resultado em rd.
- **Compilador (antes do ajuste):** O asmgen emitia `div rd, r1, r2` (três operandos, resultado em rd), o que **não corresponde** ao ISA. O encoder montava F1 com três campos; o hardware ignora o campo RD para Div e escreve apenas em High/Low.
- **Código intermediário (.tm):** O resto é implementado como **u - (u/v)*v**: uma quádrupla **DIV** (quociente) seguida de **MUL** (quociente × divisor) e **SUB** (dividendo − esse produto). Não há quádrupla REM; o quociente vem da DIV e o resto é obtido por essa sequência.

### Proposta: gerar Div correta e obter quociente/resto

1. **DIV no assembly:** Emitir a instrução **Div** do ISA com **dois operandos**: `div rs, rt` (dividendo, divisor). O processador coloca resto em **High** e quociente em **Low**.
2. **Quociente em registrador geral:** Imediatamente após `div`, copiar **Low** para o registrador destino da quádrupla com a instrução **move** do ISA: `move rd, $lo`. Assim a quádrupla `(DIV, a, b, dst)` passa a significar “quociente em dst”.
3. **Resto:** O .tm já calcula resto como `u - (u/v)*v`. Com a DIV correta, o quociente em dst é usado na sequência MUL e SUB; o resultado final é o resto. Alternativamente, se no futuro existir uso direto de resto, pode-se emitir `move rd, $hi` após `div` (resto em High).
4. **MUL (Mult):** O ISA escreve o produto em **High/Low**. Para a quádrupla `(MUL, a, b, dst)` (resultado 32 bits em dst), emitir `mult r1, r2` e depois `move rd, $lo`.

### Convenção de registradores $hi e $lo

- O encoder e o processador devem tratar **$hi** e **$lo** como os registradores especiais onde Div/Mult escrevem (e dos quais se lê com **move**). No encoder, eles são mapeados para os índices do banco reservados a High e Low (ex.: 62 e 61 se o último for o de link JAL).

### Alterações no código

- **asmgen.c:** Para **DIV**: emitir `div r1, r2` e em seguida `move rd, $lo`. Para **MUL**: emitir `mult r1, r2` e em seguida `move rd, $lo`.
- **encoder.c:** Incluir **$hi** e **$lo** em `mapReg` com os números corretos do processador. Para as instruções **DIV** e **MULT** (F1), quando a linha de assembly tiver **apenas dois operandos**, montar F1 com RS=op1, RT=op2 e RD=0 (campo ignorado pelo hardware).

## 14. Uso deste documento

Este arquivo deve servir como referência de contexto para:

- implementar ou revisar `asmgen.c`;
- implementar ou revisar `encoder.c`;
- definir convenções de registradores;
- decidir quais instruções já podem ser consideradas estáveis no compilador;
- documentar divergências entre o relatório e a implementação real.
