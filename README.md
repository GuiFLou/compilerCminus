# Compilador **C‑ → MIPS‑Lite**

Repositório do projeto de compiladores & arquitetura: tradução de **C‑** (subconjunto acadêmico de C) para **binário MIPS 32 bits** compatível com o processador monociclo em Verilog.  

O compilador gera, para cada fonte `.cms`, as quádruplas (`.tm`), o assembly (`.s`) e o binário em texto (`.txt`), pronto para a IMEM do processador ou para um setor do HD simulado. A referência do mapa de memória e do SO é [`../so/docs/CONTEXT.md`](../so/docs/CONTEXT.md); a estrutura interna do compilador está em [`../processador/context/CONTEXT.md`](../processador/context/CONTEXT.md).

---

## Índice
1. [Visão Geral](#visao-geral)
2. [Requisitos de Ambiente](#requisitos)
3. [Instalação no Windows (MSYS2 + VS Code)](#windows)
4. [Instalação no Linux / macOS](#linux-macos)
5. [Build & Smoke‑Test](#build)
6. [Estrutura do Projeto](#estrutura)
7. [Criando e Rodando um Programa C‑](#rodar)
8. [Built‑ins e intrínsecos](#builtins)
9. [Alvos e formato do binário](#alvos)
10. [Limitações conhecidas](#limitacoes)
11. [FAQ & Troubleshooting](#faq)
12. [Licença](#licenca)

---

## 1 | Visão Geral <a id="visao-geral"></a>

```
C‑ fonte  ─┐       ┌─────>  quádruplas  (.tm)
           │       │
           ├─> Front‑End ─┤
           │   (Flex/Bison│
           │    + semântica)└─────>  asm MIPS‑Lite (.s)
           │
           └──────────────────────>  binário   (.mem/.txt)
```
Recursos principais:
* **scanner / parser** gerados por Flex 2.6 & Bison 3.8.  
* **analisador semântico**: escopos, tipos, checagem de chamada de função e as restrições do alvo `os-user`.  
* **geração de código intermediário** (quádruplas SSA‑light).  
* **back‑end**: alocação simples de registrador, frames com `$fp` e emissão de ASM para o ISA do processador.  
* **assembler/encoder**: transforma ASM em palavras de 32 bits, uma por linha, escritas com os caracteres `0` e `1`.

---

## 2 | Requisitos de Ambiente <a id="requisitos"></a>

| Ferramenta | Versão mínima | Windows (MSYS2) | Linux | macOS |
|------------|--------------|-----------------|-------|-------|
| **GCC**    | 13 ou 15     | `mingw-w64-ucrt-x86_64-gcc` | pacote `build‑essential` | `brew install gcc` |
| **Flex**   | 2.6.x        | `pacman -S flex` | `apt install flex` | `brew install flex` |
| **Bison**  | 3.8.x        | `pacman -S bison` | `apt install bison` | `brew install bison` |
| **Make**   | 4.x          | `mingw-w64-ucrt-x86_64-make` | incluso | incluso |
| **VS Code**| 1.90+        | extensões *C/C++* & *Makefile Tools* |

> **Nota:** no Windows é imprescindível usar o **shell UCRT64** do MSYS2. O prompt deve terminar em `UCRT64 ~`, não `MINGW64 ~`.

O `Makefile` compila todos os `.c` do diretório, inclusive `Parser.tab.c` e `lex.yy.c`, que já estão gerados. Ele não roda Flex nem Bison: só é preciso regenerar esses dois arquivos depois de editar `Parser.y` ou `Scanner.l`. O `reset_tests.sh --target=os-user` usa `python3`.

---

## 3 | Instalação no Windows (MSYS2 + VS Code) <a id="windows"></a>

```bash
# 1. Instalar MSYS2 de https://www.msys2.org  (padrão C:\msys64)
# 2. Atualizar tudo
pacman -Syu        # feche e reabra o shell UCRT64 se pedir

# 3. Instalar toolchain
pacman -S --needed \
  mingw-w64-ucrt-x86_64-gcc \
  mingw-w64-ucrt-x86_64-make

# 4. Instalar Flex & Bison (repositório msys)
pacman -S flex bison

# 5. (Qualidade de vida) criar alias
echo "alias make='mingw32-make'" >> ~/.bashrc && source ~/.bashrc
```

No **VS Code**: vá em *Settings* → “Terminal › Integrated › Default Profile Windows” → escolha **msys2‑ucrt64**.

---

## 4 | Instalação no Linux / macOS <a id="linux-macos"></a>

### Ubuntu/Debian
```bash
sudo apt update && sudo apt install build-essential flex bison make git
```

### macOS (Homebrew)
```bash
brew install gcc flex bison make
```

---

## 5 | Build & Smoke‑Test <a id="build"></a>

```bash
# Clonar (ou já estar na pasta)
cd compilador

make            # usa alias → mingw32-make no Windows

# Exemplo completo
./compilador --txt exemplos/fatorial.cms

# Saídas geradas (mesmo diretório)
#   fatorial.tm   – quádruplas
#   fatorial.s    – assembly
#   fatorial.txt  – binário texto
```

Se chamar sem `--txt`, o binário sai em `.mem`; renomeie se preferir:
```bash
mv programa.mem programa.txt
```

A linha de comando é `./compilador [--txt] [--target=os-user] <arquivo[.cms]>`. Se o caminho não tiver nenhum ponto, o compilador acrescenta `.cms`.

### Executar no processador

O processador começa pela IMEM carregada de `processador/so.txt`, que é o kernel do SO. Para rodar um binário do alvo padrão sozinho, sem o SO, passe o `.txt` no parâmetro `ROM_FILE` do top-level (`Processador_GUI_MIPS`). O `fatorial` lê `n` pelo `IN` (SW[9:0], confirmado com KEY[1]) e mostra `n!` nos HEX0–HEX6; com SW = 5 o `OUT` mostra 120 e o PC para no `HLT`. Os testbenches e o comando do Icarus estão em [`../processador/README.md`](../processador/README.md).

### Resetar e recompilar os exemplos

`exemplos/` contém os dez programas usados pelo SO (`area_triangulo`, `busca_linear`, `fatorial`, `fibonacci`, `gcd`, `media`, `numero_primo`, `potencia`, `soma_digitos`, `sort`). Sem flag, o script `reset_tests.sh` remove os arquivos gerados (`.s`, `.tm`, `.txt` e `.mem`) dessa pasta e recompila os programas no alvo padrão. Essa pasta é a ROM solta da ferramenta; o SO executa os binários guardados no HD do processador.

```bash
./reset_tests.sh                         # todos os exemplos
./reset_tests.sh --compile               # recompila o compilador e os exemplos
./reset_tests.sh --compile fatorial gcd  # somente programas selecionados
./reset_tests.sh --no-test               # somente remove arquivos antigos
./reset_tests.sh --target=os-user        # grava os dez setores 20–29 do HD
./reset_tests.sh --target=os-user sort   # grava só o setor 29
```

Com `--target=os-user`, cada programa é compilado numa pasta temporária e o binário substitui o setor correspondente de `../processador/hd_image.txt` (área 20, busca 21, fatorial 22, fibonacci 23, gcd 24, média 25, primo 26, potência 27, dígitos 28, sort 29); o resto do setor vira zero e os outros setores não mudam. `exemplos/` não é alterada. Um `.cms` sem setor fica para a gestão de arquivos. `--no-test` não pode ser combinado com `--target=os-user`. Os nomes dos programas devem ser informados sem a pasta e sem a extensão `.cms`.

---

## 6 | Estrutura do Projeto <a id="estrutura"></a>

```
├── analyze.{c,h}     # semântica e restrições do os-user
├── asmgen.{c,h}      # IR → ASM (frames, intrínsecos HD/LCD/runat)
├── encoder.{c,h}     # ASM → binário (NOP de tamanho, HLT, base 3072)
├── cgen.{c,h}        # quádruplas; chama asmgen e encoder
├── symtab.{c,h}      # tabela de símbolos e built-ins (insertBuiltIns)
├── util.{c,h}        # nós da AST e impressão
├── Parser.y / Scanner.l   (+ Parser.tab.c/h e lex.yy.c gerados)
├── main.c            # CLI (--txt, --target=os-user)
├── Makefile
├── reset_tests.sh    # regenera exemplos/ ou grava os setores 20–29 do HD
└── exemplos/
    └── *.cms         # os dez programas (+ .tm, .s, .txt gerados)
```

Não há suíte de testes automatizada neste diretório. Os testes de integração ficam no processador (`tb_*.v`).

---

## 7 | Criando e Rodando um Programa C‑ <a id="rodar"></a>

Modelo de “Hello World numérico” (fatorial, igual a `exemplos/fatorial.cms`):
```c
void main(void) {
    int n;
    int result;
    n = input();
    result = 1;
    while (n > 0) {
        result = result * n;
        n = n - 1;
    }
    output(result);
}
```
`input` e `output` são built-ins: não declare nenhum dos dois no programa. Salve como `meuprog.cms` e compile:
```bash
./compilador --txt meuprog.cms                    # alvo padrão
./compilador --txt --target=os-user meuprog.cms   # binário para a folha do SO
```

---

## 8 | Built‑ins e intrínsecos <a id="builtins"></a>

A função `insertBuiltIns`, em `symtab.c`, coloca estes nomes na tabela global antes da análise. Declarar uma função com um deles (por exemplo `void output(int x) { }`) dá `Error 4: Invalid Declaration. Already declared.`

| Nome | Assinatura | Código gerado |
|------|------------|---------------|
| `input` | `int input()` | `IN` no temporário de destino. |
| `output` | `void output(int)` | `OUT` do valor. |
| `loadHD` | `int loadHD(setor, addr)` | HD → RAM (`hd_cmd` 1). |
| `loadROM` | `int loadROM(setor, addr)` | HD → IMEM (`hd_cmd` 2). |
| `storeHD` | `int storeHD(setor, addr)` | RAM → HD (`hd_cmd` 3). |
| `lcdshow` | `void lcdshow(modo, id, pid)` | Monta `{pid[3:0], id[9:0], modo[7:0]}` e grava em 4091 (`display_ctx`). |
| `lcdack` | `int lcdack()` | Lê 4091 e devolve o bit 27 (confirmação do KEY[1]). |
| `lcdsel` | `int lcdsel()` | Lê 4091 e devolve os bits 26:22 (índice do menu). |
| `runat` | `void runat(addr)` | Desvia para `addr` e volta no `HLT` armado. |

Os três intrínsecos do HD fazem `SW` do setor em 4093, do endereço em 4094 e do comando em 4092, depois leem `hd_status` (4095) em laço enquanto ele for 1 (busy). O retorno é 0 quando terminou (done) e 1 em erro ou idle: o idle só aparece se um início se perdesse, e assim o kernel não fica preso no laço. O valor sai do bit 1 de `status + 3`, sem desvio a mais.

`runat` não é `jal`. Grava `$v0`, `$t0`–`$t9`, `$gp`, `$sp`, `$fp` e `$ra` em 1043–1057, faz `SW` do endereço em 4088 (o hardware guarda PC+1 e desvia) e, na volta, restaura os 15 registradores com `LW` a partir de `lw $v0,1043`. `$hi` e `$lo` ficam de fora. O hardware só aceita destino entre 3072 e 3839.

---

## 9 | Alvos e formato do binário <a id="alvos"></a>

A linha de comando aceita o alvo padrão e `--target=os-user`. Outro valor de `--target=` é recusado; não existe alvo `os-kernel`.

| | Alvo padrão | `--target=os-user` |
|--|-------------|---------------------|
| Prólogo | `addi $sp,$zero,1015`, `j main` | `addi $gp,$zero,1536`, `addi $sp,$zero,1791`, `j main` |
| `$gp` | não inicializado (0 do reset) | 1536, início da janela 1536–1791 |
| `J`/`JAL` | endereço a partir de 0 | endereço + 3072 (folha da vez) |
| `BEQ`/`BNE` | relativos | relativos, sem ajuste |
| Tamanho | até o limite de 26 bits | recusa mais de 256 palavras (um setor) |
| Dados | sem limite | recusa `dataUsed > 256` (`analyze.c`) |
| HD, LCD, `runat` | aceitos | erro de semântica |
| Uso | `exemplos/` e o kernel `so.cms` | setores 20–29 de `hd_image.txt` |

Nos dois alvos o encoder:

- põe na palavra 0 um `NOP` cujo campo de 26 bits é o tamanho total `N` do binário;
- garante um `HLT` na palavra `N-1` (só acrescenta se o `.s` não terminar em `hlt`). `N` inclui o NOP e o HLT;
- desloca os labels em 1 por causa do NOP, de modo que `J`/`JAL` apontam para o endereço final;
- monta imediatos F2 de 14 bits (−8192 a 8191), registradores em campos de 6 bits e PC que avança uma palavra por instrução.

O kernel em `so/src/so.txt` sai do alvo padrão e também começa com o NOP de tamanho (`NOP 1335`) e termina em `HLT`.

Os parâmetros e locais vivem no frame da função: `0($fp)` guarda o `$fp` anterior, `1($fp)` o `$ra`, os parâmetros ficam em offsets positivos (o último em `2($fp)`) e os locais em offsets negativos. Só as globais usam `$gp`.

---

## 10 | Limitações conhecidas <a id="limitacoes"></a>

- **Temporários entre chamadas.** Os temporários do código intermediário são mapeados circularmente em `$t0`–`$t9` (`tN` → `$t(N % 10)`), e nada salva os `$t` antes de um `jal`. Numa expressão como `n * fat(n-1)`, o valor de `n` fica num `$t` que a chamada recursiva sobrescreve, e o resultado sai errado (`fat(5)` dá 1 na simulação). Guarde o resultado da chamada num local antes de usá-lo (`r = fat(n-1); return n * r;` dá 120). O `so.cms` e os dez programas de `exemplos/` não são afetados.
- **Pilha no `os-user`.** Globais e pilha dividem a janela 1536–1791. O limite `dataUsed > 256` conta as posições de símbolos e não considera a pilha, então um programa com muitos dados e chamadas profundas pode ter a pilha sobre as globais sem aviso.
- **Sem protótipos.** A gramática não aceita só a assinatura; a função precisa ser definida antes de ser chamada.

---

## 11 | FAQ & Troubleshooting <a id="faq"></a>

| Erro | Causa & Solução |
|------|-----------------|
| `flex: command not found` | Está no shell errado (PowerShell) ou não instalou Flex. Abra **UCRT64** e `pacman -S flex`. |
| `make: command not found` | Instale `mingw-w64-ucrt-x86_64-make` e/ou crie alias (`make='mingw32-make'`). |
| `File XYZ not found` | Caminho/arquivo não existe; verifique se está na pasta correta ou use extensão `.cms`. |
| `Invalid Call. It was not declared` | A função chamada não foi definida antes da chamada. `input`, `output` e os intrínsecos já são built-ins. |
| `Invalid Declaration. Already declared.` | Nome repetido no escopo, ou declaração de um built-in como `output`. Remova a declaração. |
| `os-user nao pode chamar HD, LCD nem runat` | Esses intrínsecos são do kernel; compile sem `--target=os-user`. |
| `[encoder] os-user excede 256 palavras` | O programa não cabe num setor. |
| `os-user: dados estaticos ocupam N palavras; o limite e 256` | Os dados não cabem na janela 1536–1791. |
| `alvo desconhecido: --target=...` | Só existe `--target=os-user`. |
| “Syntax error” em protótipo `void foo(int);` | A gramática C‑ não aceita apenas protótipo; forneça corpo `{ }`. |

---

## 12 | Licença <a id="licenca"></a>

Código sob **MIT License** desenvolvido por Guilherme. Curso de Engenharia da Computação - UNIFESP (2025).
