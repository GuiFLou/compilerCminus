# Contexto do compilador C‑ → MIPS-Lite (para IA)

Este documento descreve como o projeto do compilador funciona (ou deve funcionar), para servir de contexto a ferramentas de IA que trabalhem no código.

---

## 1. Visão geral do pipeline

O compilador traduz **código fonte C‑** (arquivos `.cms`) em **binário 32 bits** compatível com um processador MIPS monociclo em Verilog. O fluxo é:

```
  .cms (fonte)
      │
      ▼
  ┌─────────────────────────────────────────────────────────────────┐
  │  MÓDULO DE ANÁLISE (front-end)                                   │
  │  • Análise léxica (Flex)  →  tokens                              │
  │  • Análise sintática (Bison)  →  árvore sintática (AST)          │
  │  • Análise semântica  →  tabela de símbolos + erros/alertas     │
  │  Saídas: árvore sintática e tabela de símbolos no listing        │
  └─────────────────────────────────────────────────────────────────┘
      │
      ▼
  ┌─────────────────────────────────────────────────────────────────┐
  │  MÓDULO DE GERAÇÃO (back-end)                                    │
  │  • Geração de código intermediário  →  quádruplas  (.tm)         │
  │  • Geração de assembly  →  MIPS-Lite  (.s)                      │
  │  • Montador/encoder  →  binário texto  (.mem / .txt)              │
  └─────────────────────────────────────────────────────────────────┘
```

- **Entrada:** `arquivo.cms` (subconjunto acadêmico de C).
- **Saídas:** `arquivo.tm` (quádruplas), `arquivo.s` (assembly), `arquivo.mem` ou `arquivo.txt` (binário para ROM).
- **Listing:** saída de diagnóstico (árvore, tabela de símbolos, tokens, etc.) vai para `stdout` por padrão.

---

## 2. Estrutura do projeto

| Arquivo        | Papel |
|----------------|--------|
| `main.c`       | CLI (aceita `--txt` e caminho do `.cms`), orquestra análise e geração. |
| `Scanner.l`    | Especificação Flex: análise léxica. |
| `Parser.y`     | Especificação Bison: análise sintática e construção da AST. |
| `Parser.tab.c`, `Parser.tab.h` | Gerados por Bison a partir de `Parser.y`. |
| `lex.yy.c`     | Gerado por Flex a partir de `Scanner.l`. |
| `globals.h`    | Tipos compartilhados (tokens, `TreeNode`, flags, etc.). |
| `parse.h`      | Declaração de `parse()` e `yyerror()`. |
| `scan.h`       | Declaração de `getToken()`, `tokenString`, `MAXTOKENLEN`. |
| `util.c`, `util.h` | Árvore (novos nós, cópia de string, `printTree`, `aggScope`). |
| `symtab.c`, `symtab.h` | Tabela de símbolos (hash), inserção/consulta, built-ins, impressão. |
| `analyze.c`, `analyze.h` | Análise semântica: construção da tabela e verificação de tipos. |
| `cgen.c`, `cgen.h` | Geração de quádruplas a partir da AST e orquestração do back-end. |
| `asmgen.c`, `asmgen.h` | Tradução de quádruplas para assembly MIPS-Lite (`.s`). |
| `encoder.c`, `encoder.h` | Montador: assembly (`.s`) → binário (`.mem`/`.txt`). |
| `Makefile`     | Build (espera que `Parser.tab.c` e `lex.yy.c` já existam ou sejam gerados com Flex/Bison). |

---

## 3. Módulo de análise

### 3.1 Análise léxica (Scanner.l)

- **Ferramenta:** Flex.
- **Saída:** sequência de tokens para o parser.
- **Tokens:** palavras reservadas (`if`, `else`, `int`, `return`, `void`, `while`), operadores e delimitadores (`=`, `==`, `<`, `>`, `<=`, `>=`, `!=`, `[`, `]`, `{`, `}`, `+`, `-`, `*`, `/`, `(`, `)`, `;`, `,`), `NUM` (inteiros), `ID` (identificadores), comentários `/* ... */`, newline (atualiza `lineno`).
- **Interface:** `getToken()` (em `Scanner.l`) retorna o próximo `TokenType` e preenche `tokenString`; usa `source` e `listing` globais; com `TraceScan` ativo, imprime cada token no listing.

### 3.2 Análise sintática (Parser.y)

- **Ferramenta:** Bison.
- **Valor semântico:** `YYSTYPE` = `TreeNode *` (ponteiro para nó da árvore).
- **Gramática:** programa = lista de declarações; declarações = variáveis (`int id;`, `int id[N];`) e funções (`int id(params) { ... }`, `void id(params) { ... }`); parâmetros; corpo com declarações locais e lista de statements; statements = expressões, blocos, `if`/`if-else`, `while`, `return`; expressões com atribuição, operadores relacionais e aritméticos; variáveis escalares e indexadas (`id[exp]`); chamadas de função.
- **Árvore:** cada nó é `TreeNode` (`globals.h`): `NodeKind` (StmtK, ExpK), para StmtK usa `StmtKind` (IfK, whileK, AssignK, variableK, functionK, callK, returnK, numberK), para ExpK usa `ExpKind` (OpK, ConstK, IdK, vectorK, vectorIdK, typeK); até 3 filhos e lista de irmãos (`sibling`); atributos como `attr.op`, `attr.val`, `attr.name`, `attr.len`, `attr.scope`; campo `type` para análise semântica (`ExpType`: voidK, integerK, booleanK).
- **Escopo:** na regra de função, `aggScope()` (em `util.c`) propaga o nome da função como `scope` para parâmetros e corpo.
- **Saída do parser:** árvore sintática cuja raiz é retornada por `parse()` (`savedTree`).
- **Erros:** `yyerror()` imprime “Syntax error at line …” e seta `Error = TRUE`.

### 3.3 Análise semântica (analyze.c, symtab.c)

- **Tabela de símbolos:** hash com escopo (`symtab.c`). Cada entrada: nome, linha(s), `memloc`, escopo, `typeID` (e.g. "variable", "function", "call"), `typeData` (e.g. "integer", "void").
- **Inicialização:** `symtabInit()` zera a tabela e insere built-ins: `input` (integer) e `output` (void com um int).
- **Construção da tabela:** `buildSymtab(syntaxTree)` faz uma passagem em pré-ordem (com `traverse(..., insertNode, nullProc)`). Para cada nó:
  - **variableK / functionK:** insere no escopo atual; se já existir (no escopo ou em global), erro “Invalid Declaration. Already declared.”
  - **callK:** verifica se o nome existe (escopo local ou global); senão, “Invalid Call. It was not declared.”
  - **vectorK / vectorIdK:** verifica se o vetor foi declarado.
- **Pós-condição:** exige que exista `main` em escopo global; caso contrário, “main was not declared”.
- **Verificação de tipos:** `typeCheck(syntaxTree)` faz passagem em pós-ordem (com `traverse(..., nullProc, checkNode)`). Regras relevantes:
  - Condição de `if` deve ser booleana (não inteira).
  - Atribuição de retorno de função `void` a variável é erro (“assignment of void return”).
- **Listing:** com `TraceAnalyze`, imprime “Building Symbol Table…”, “Checking Types…”, “Type Checking Finished” e a tabela de símbolos formatada.

---

## 4. Módulo de geração

### 4.1 Geração de código intermediário (cgen.c)

- **Representação:** quádruplas `(op, arg1, arg2, result)` em lista encadeada (`Quadruple`), campos string (ex.: `op[10]`, `arg1[32]`, etc.).
- **Operações emitidas:**  
  **Declaração/alocação:** `ALLOC name scope -` para variáveis.  
  **Função:** `FUN tipo name -`, depois `ARGS` (se houver parâmetros), corpo, `END name - -`.  
  **Expressões:** `ASSIGN lit - dst`, `LOAD name - dst`, `LOADV name idx dst`, `ADD/SUB/MUL/DIV a b dst`.  
  **Atribuição:** `STORE rhs - name`, `STOREV rhs idx name`.  
  **Controle:** `IFF cond label -`, `GOTO label - -`, `LAB label - -`.  
  **Chamadas:** `PARAM arg - -`, `CALL name nargs result` (ou `-` se void), `CALL_I` (input → temp), `CALL_O` (output com um argumento).  
  **Retorno:** `RET value - -` ou `RET - - -`.  
  **Fim:** `HALT - - -`.
- **Temporários e labels:** `newTemp()` → `t0`, `t1`, …; `newLabel()` → `L0`, `L1`, …
- **Fluxo:** `codeGen(syntaxTree, base)` zera lista e contadores, percorre a raiz (e irmãos) com `genStmt`, emite `HALT`, grava as quádruplas em `<base>.tm`, chama `asmGen(quadList, <base>.s)` e `encodeAsm(<base>.s, <base>.mem)`.
- **Observação:** apenas o primeiro nó da raiz é percorrido (`TreeNode *n = syntaxTree; genStmt(n);`); declarações no topo (variáveis globais, funções) são tratadas como irmãos desse nó, então o projeto pode precisar iterar sobre `syntaxTree` e seus `sibling` para gerar código de todas as funções (como no comentário “percorre TODAS as funções ligadas por sibling”).

### 4.2 Geração de assembly (asmgen.c)

- **Entrada:** lista de quádruplas.
- **Saída:** arquivo `.s` com assembly MIPS-Lite (texto).
- **Registradores:** temporários lógicos `t0`–`t9` mapeados para `$t0`–`$t9`; uso de `$gp`, `$sp`, `$ra`, `$v0`, `$zero`.
- **Mapeamento de quádruplas:**  
  - ALU: ADD, SUB → `add`/`sub`; MUL → `mult r1,r2` + `move rd,$lo` (ISA escreve em High/Low); DIV → `div r1,r2` + `move rd,$lo` (quociente em Low); ASSIGN (literal ou cópia) → `addi` ou `add` com `$zero`. Resto no .tm é `u - (u/v)*v` (DIV + MUL + SUB).  
  - Memória: LOAD/STORE → `lw`/`sw` com `name($gp)`; LOADV/STOREV → cálculo de endereço com `sll` (índice*4) e `$gp`.  
  - Controle: IFF → `beq cond, $zero, label`; GOTO → `j label`; LAB → `label:`.  
  - Funções: PARAM → push na pilha (`addi $sp,$sp,-1`; `sw`); CALL → `jal` + pop de argumentos; RET → `add $v0,...` (se valor) e `jr $ra`; CALL_I → `in`; CALL_O → `out`.  
  - HALT → `hlt`.
- **Observação sobre endereçamento:** LOAD/STORE usam o **nome** da variável no assembly (ex.: `lw $t0, n($gp)`). O encoder atual não resolve símbolos de dados; ele interpreta o “offset” como número. Para gerar binário correto, é necessário ou (1) emitir o **offset** (`memloc` da tabela de símbolos) em vez do nome no asmgen, ou (2) fazer o encoder resolver nomes de variáveis a partir de uma tabela de símbolos de dados.

### 4.3 Encoder / montador (encoder.c)

- **Entrada:** arquivo **assembly** (`.s`), não `.tm`.
- **Saída:** arquivo binário (`.mem` ou `.txt`) com uma palavra de 32 bits por linha, em **texto** (caracteres `0` e `1`).
- **Importante (PC no .txt):** como a ROM/memória de instrução é alimentada com **uma word por linha** do `.mem`/`.txt`, o endereço/PC efetivo para “próxima instrução” deve considerar **incremento unitário** (**PC ← PC + 1** em *words*), e não o padrão do MIPS byte-addressed (**PC ← PC + 4**). Isso afeta o entendimento de “endereço”/“posição” ao resolver labels e offsets de desvio.
- **Formato de instrução:** três formatos (F1, F2, F3) com opcode em 6 bits e campos conforme o ISA (rd, rs, rt, imediato, endereço). Mnemônicos mapeados incluem: ADD, ADDI, SUB, SUBI, MULT/mul, DIV, AND, OR, NOT, SR/SRL/srl, SL/SLL/sll, LOAD/lw, STORE/sw, JUMP/j, JUMPR/jr, JAL, BEQ, BNE, MOVE, NOP, HLT, SLT, IN, OUT.
- **Duas passagens:** primeira passagem coleta rótulos e endereços; segunda passagem monta cada instrução e resolve referências a labels (saltos relativos quando aplicável).
- **Registradores:** `$zero`=0, `$sp`=29, `$gp`=28, `$ra`=31, `$hi`=62, `$lo`=61, `$t0`–`$t9`=8–17 (mapeamento interno). DIV/MULT com dois operandos montam F1 com RD=0 (resultado em High/Low).

---

## 5. Fluxo em main.c

1. Parse de argumentos: opção `--txt` e caminho do arquivo; se não houver extensão, acrescenta `.cms`.
2. Abre o arquivo fonte, define `listing = stdout`, imprime “C- COMPILATION: …”.
3. **Se NO_PARSE:** apenas chama `getToken()` em loop até ENDFILE (só scanner).
4. **Caso contrário:**  
   - Chama `parse()` → árvore sintática.  
   - Se `TraceParse`: imprime “Syntax tree:” e `printTree(syntaxTree)`.  
   - **Se !NO_ANALYZE e !Error:**  
     - `buildSymtab(syntaxTree)`; se `TraceAnalyze`, imprime mensagens e tabela.  
     - `typeCheck(syntaxTree)`.  
   - **Se !NO_CODE e !Error:**  
     - Abre `<base>.tm` em `code`.  
     - `codeGen(syntaxTree, base)` (gera `.tm`, `.s` e `.mem`).  
     - Fecha `code`.  
     - Se `--txt`, renomeia `<base>.mem` para `<base>.txt`.
5. Fecha o arquivo fonte; retorna 0.

Flags de controle (em `main.c`): `NO_PARSE`, `NO_ANALYZE`, `NO_CODE` (todas FALSE por padrão); `EchoSource`, `TraceScan`, `TraceParse`, `TraceAnalyze`, `TraceCode` (todas TRUE por padrão). `Error` é setado por erros sintáticos ou semânticos e impede a geração de código.

---

## 6. Linguagem C‑ (arquivos .cms)

- **Tipos:** `int`, `void`. Variáveis escalares e vetores `int id[N]`.
- **Funções:** retorno `int` ou `void`; parâmetros podem ser `int id` ou `int id[]`.
- **Built-ins:** `input()` (retorna int) e `output(int)`; devem estar na tabela global (inseridas em `symtabInit`) ou declaradas como stub no fonte (ex.: `void output(int x) { }`).
- **Obrigatório:** função `main` sem parâmetros (void ou int).
- **Gramática:** não aceita apenas protótipo de função (ex.: `void foo(int);`); é necessário corpo `{ }`.

---

## 7. Pontos úteis para manutenção e IA

- **Parser.tab.c / lex.yy.c:** gerados por Bison e Flex; não editar à mão. Comandos típicos: `bison -d Parser.y`, `flex Scanner.l`.
- **Convenção de nomes:** variáveis na tabela de símbolos têm `memloc`; esse valor pode ser usado no back-end para emitir offset em LOAD/STORE em vez do nome, se o encoder não resolver símbolos.
- **Erros semânticos:** códigos mencionados no código (ex.: “Error 1”, “Error 4”, “Error 5”) indicam “não declarado”, “declaração inválida (já declarado)” e “chamada inválida (não declarado)”.
- **Quádruplas:** o formato `.tm` é texto, uma quádrupla por linha (op, arg1, arg2, result), numeradas a partir de 0; útil para depuração e para entender a IR antes do assembly.
- **Binário final:** uma linha por palavra de 32 bits (0/1), pronto para carregar na ROM (ex.: ModelSim/FPGA com `load_mem.do`).

Este documento reflete o estado do código no repositório e pode ser expandido com detalhes do ISA (opcodes, formatos F1/F2/F3) ou da convenção de chamada (pilha, $ra, $v0) conforme necessário.
