# Compilador **C‑ → MIPS‑Lite**

Repositório do projeto de compiladores & arquitetura: tradução de **C‑** (subconjunto acadêmico de C) para **binário MIPS 32 bits** compatível com o processador monociclo em Verilog.  

Entrega final: **sem warnings, gerando `.txt` dentro do diretório "exemplos" pronto para gravar na ROM.**

---

## Índice
1. [Visão Geral](#visao-geral)
2. [Requisitos de Ambiente](#requisitos)
3. [Instalação no Windows (MSYS2 + VS Code)](#windows)
4. [Instalação no Linux / macOS](#linux-macos)
5. [Build & Smoke‑Test](#build)
6. [Estrutura do Projeto](#estrutura)
7. [Criando e Rodando um Programa C‑](#rodar)
8. [Função `output()` – opções](#output)
9. [FAQ & Troubleshooting](#faq)
10. [Licença](#licenca)

---

## 1 | Visão Geral <a id="visao-geral"></a>

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
* **scanner / parser** gerados por Flex 2.6 & Bison 3.8.  
* **analisador semântico**: escopos, tipos, checagem de chamada de função.  
* **geração de código intermediário** (quádruplas SSA‑light).  
* **back‑end**: alocação simples de registrador + emissão de ASM para o ISA do seu processador.  
* **assembler/encoder**: transforma ASM → hex 32 bits (texto) para inicializar a ROM.

---

## 2 | Requisitos de Ambiente <a id="requisitos"></a>

| Ferramenta | Versão mínima | Windows (MSYS2) | Linux | macOS |
|------------|--------------|-----------------|-------|-------|
| **GCC**    | 13 ou 15     | `mingw-w64-ucrt-x86_64-gcc` | pacote `build‑essential` | `brew install gcc` |
| **Flex**   | 2.6.x        | `pacman -S flex` | `apt install flex` | `brew install flex` |
| **Bison**  | 3.8.x        | `pacman -S bison` | `apt install bison` | `brew install bison` |
| **Make**   | 4.x          | `mingw-w64-ucrt-x86_64-make` | incluso | incluso |
| **VS Code**| 1.90+        | extensões *C/C++* & *Makefile Tools* |

> **Nota:** no Windows é imprescindível usar o **shell UCRT64** do MSYS2. O prompt deve terminar em `UCRT64 ~`, não `MINGW64 ~`.

---

## 3 | Instalação no Windows (MSYS2 + VS Code) <a id="windows"></a>

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

No **VS Code**: vá em *Settings* → “Terminal › Integrated › Default Profile Windows” → escolha **msys2‑ucrt64**.

---

## 4 | Instalação no Linux / macOS <a id="linux-macos"></a>

### Ubuntu/Debian
```bash
sudo apt update && sudo apt install build-essential flex bison make git
```

### macOS (Homebrew)
```bash
brew install gcc flex bison make
```

---

## 5 | Build & Smoke‑Test <a id="build"></a>

```bash
# Clonar (ou já estar na pasta)
cd compilador

make            # usa alias → mingw32-make no Windows

# Exemplo completo
./compilador --txt exemplos/fatorial.cms

# Saídas geradas (mesmo diretório)
#   fatorial.tm   – quádruplas
#   fatorial.s    – assembly
#   fatorial.txt  – binário texto (pronto para ROM)
```

Se chamar sem `--txt`, o binário sai em `.mem`; renomeie se preferir:
```bash
mv programa.mem programa.txt
```

### Carregar na ROM (ModelSim / FPGA)
```tcl
vsim work.MIPS
#do load_mem.do exemplos/fatorial.txt
run 5 us   ;# display deve mostrar 120 (0x0078)
```

---

## 6 | Estrutura do Projeto <a id="estrutura"></a>

```
├── analyze.{c,h}     # semântica
├── asmgen.{c,h}      # IR → ASM
├── encoder.{c,h}     # ASM → binário
├── cgen.{c,h}        # orquestra back‑end
├── symtab.{c,h}      # tabela de símbolos
├── Parser.y / Scanner.l
├── main.c            # CLI (--txt)  ✅ 2025‑07‑20
├── Makefile
├── exemplos/
│   └── fatorial.cms  # demo completo
└── tests/            # sua suíte de regressão (adicione aqui)
```

---

## 7 | Criando e Rodando um Programa C‑ <a id="rodar"></a>

Modelo de “Hello World numérico” (fatorial):
```c
void output(int x) { }     /* stub built‑in I/O */

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
Salve como `meuprog.cms` e compile:
```bash
./compilador --txt meuprog.cms
```

---

## 8 | Função `output()` – opções <a id="output"></a>

1. **Stub local (recomendado para começar)** – defina `void output(int x) { }` no topo de cada arquivo.  
2. **Built‑in automático** – adicione `initBuiltIns()` na `symtab.c` para inserir `output` (e `input`, se desejar) na tabela global ao iniciar.

---

## 9 | FAQ & Troubleshooting <a id="faq"></a>

| Erro | Causa & Solução |
|------|-----------------|
| `flex: command not found` | Está no shell errado (PowerShell) ou não instalou Flex. Abra **UCRT64** e `pacman -S flex`. |
| `make: command not found` | Instale `mingw-w64-ucrt-x86_64-make` e/ou crie alias (`make='mingw32-make'`). |
| `File XYZ not found` | Caminho/arquivo não existe; verifique se está na pasta correta ou use extensão `.cms`. |
| `Invalid Call. It was not declared` | Função como `output` não declarada. Use stub ou built‑in. |
| “Syntax error” em protótipo `void foo(int);` | A gramática C‑ não aceita apenas protótipo; forneça corpo `{ }`. |

---

## 10 | Licença <a id="licenca"></a>

Código sob **MIT License** desenvolvido por Guilherme. Curso de Engenharia da Computação - UNIFESP (2025).
