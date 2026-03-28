# Quádruplas do código intermediário

A geração ocorre em `cgen.c`. Cada instrução é uma quádrupla com quatro campos de texto, impressos como `(op, arg1, arg2, result)` (ver `printQuads`).

Na estrutura `Quadruple` (`cgen.h`): `op` até 9 caracteres + `\0`; `arg1`, `arg2` e `result` até 31 caracteres + `\0`.

## Formato geral

| Campo   | Papel |
|---------|--------|
| **op**  | Código da operação (mnemônico). |
| **arg1**| Primeiro operando ou metadado. |
| **arg2**| Segundo operando, `"-"` se inexistente. |
| **result**| Destino do resultado ou metadado; `"-"` se inexistente. |

Valores literais ausentes são representados pela string `"-"` em `emit`.

A coluna **Tipo** segue as mesmas classes do conjunto de instruções do processador descrito em `Relatório_Processador.tex` (tabela *Conjunto de Instruções do Projeto*): Aritmética, Lógica, Deslocamento, Acessa Memória, Salto, Salto Cond, Transf., Controle, I/O.

---

## Tabela por operação

| `op` | Tipo | `arg1` | `arg2` | `result` | Observação |
|------|------|--------|--------|----------|------------|
| `ALLOC` | Acessa Memória | Nome do símbolo | Escopo (`"global"`, nome da função) ou `"-"` | Tamanho do vetor (decimal) ou `"-"` | Alocação de variável; escalar: `result` é `"-"`; vetor: `result` é o tamanho. |
| `FUN` | Controle | Tipo: `int`, `bool` ou `void` | Nome da função | `"-"` | Início de função. |
| `ARG` | Transf. | Tipo: `int`, `bool` ou `void` | Nome do parâmetro | Nome da função (dono) | Parâmetro formal; uma quádrupla por parâmetro. |
| `END` | Controle | Nome da função | `"-"` | `"-"` | Fim de função. |
| `ASSIGN` | Transf. | Literal inteiro (decimal) | `"-"` | Temporário (`t0`, `t1`, …) | Cópia de constante para temporário. |
| `LOAD` | Acessa Memória | Nome da variável | `"-"` | Temporário | Leitura de escalar ou vetor sem índice. |
| `LOADV` | Acessa Memória | Nome do vetor | Índice (temp ou expressão já reduzida) | Temporário | Leitura indexada. |
| `ADDR` | Acessa Memória | Nome do vetor | `"-"` | Temporário | Endereço base do vetor (argumento por referência). |
| `STORE` | Acessa Memória | Valor (temp ou literal via RHS) | `"-"` | Nome da variável destino | Atribuição a escalar. |
| `STOREV` | Acessa Memória | Valor RHS | Índice | Nome do vetor | Atribuição indexada. |
| `ADD` | Aritmética | Operando esquerdo | Operando direito | Temporário | |
| `SUB` | Aritmética | Operando esquerdo | Operando direito | Temporário | |
| `MUL` | Aritmética | Operando esquerdo | Operando direito | Temporário | |
| `DIV` | Aritmética | Operando esquerdo | Operando direito | Temporário | |
| `EQUAL` | Lógica | Operando esquerdo | Operando direito | Temporário | Relacional `==` (análogo a comparações como `slt` no ISA). |
| `NEQ` | Lógica | Operando esquerdo | Operando direito | Temporário | Relacional `!=`. |
| `LT` | Lógica | Operando esquerdo | Operando direito | Temporário | Relacional `<`. |
| `LTE` | Lógica | Operando esquerdo | Operando direito | Temporário | Relacional `<=`. |
| `GT` | Lógica | Operando esquerdo | Operando direito | Temporário | Relacional `>`. |
| `GTE` | Lógica | Operando esquerdo | Operando direito | Temporário | Relacional `>=`. |
| `PARAM` | Transf. | Valor do argumento | `"-"` | `"-"` | Argumento de chamada; vários `PARAM` antes de `CALL` / `CALL_O`. |
| `CALL` | Salto | Nome da função | Número de argumentos (decimal) | Temporário **ou** `"-"` | Chamada (~ `Jal`); com valor: `result` é temp; como comando: `result` é `"-"`. |
| `CALL_I` | I/O | `"-"` | `"-"` | Temporário | Entrada (`input`), análogo a `In`. |
| `CALL_O` | I/O | Valor a imprimir | `"-"` | `"-"` | Saída (`output`), análogo a `Out`; precedido de `PARAM` com o mesmo valor. |
| `RET` | Salto | Valor retornado ou `"-"` (void) | `"-"` | `"-"` | Retorno de função (transferência de controlo). |
| `IFF` | Salto Cond | Condição (temp) | Rótulo se falso (`L0`, …) | `"-"` | Salto se condição falsa (~ `beq`/`bne` após redução). |
| `GOTO` | Salto | Rótulo | `"-"` | `"-"` | Desvio incondicional (~ `Jump`). |
| `LAB` | Controle | Nome do rótulo | `"-"` | `"-"` | Alvo de salto (rótulo); análogo ao *Label* em formato F3. |
| `HALT` | Controle | `"-"` | `"-"` | `"-"` | Fim do programa (~ `hlt`); emitido ao final de `codeGen`. |

Nenhuma quádrupla é gerada diretamente para deslocamentos (`Sr` / `Sl`); o *back-end* pode traduzi-los a partir de outras operações se necessário.

---

## Convenções de operandos

- **Temporários:** `t0`, `t1`, … (`newTemp`).
- **Rótulos:** `L0`, `L1`, … (`newLabel`).
- **Escopos em `ALLOC`:** string do escopo lexical ou `"-"` quando não aplicável da mesma forma que no nó da AST.

Esta lista cobre todas as operações passadas a `emit` em `cgen.c`.
