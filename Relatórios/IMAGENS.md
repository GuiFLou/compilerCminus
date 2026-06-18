# Listagem de imagens — Relatório do Compilador

Arquivos referenciados em `relatorio_compiladores.tex`. No Overleaf, crie a pasta **`Imagens/`** na raiz do projeto LaTeX.

> **Nota:** O capítulo do processador foi reduzido ao contrato de máquina visível ao compilador. Imagens de componentes Verilog (memórias, mux, ULA, etc.) **não são mais necessárias** neste relatório.

---

## Imagens obrigatórias (5 arquivos)

| # | Arquivo | Label LaTeX | Capítulo | Descrição |
|---|---------|-------------|----------|-----------|
| 1 | `Datapath.png` | `fig:MyDevice` | 2 — Arquitetura Alvo | Visão funcional do datapath (contrato compilador ↔ hardware) |
| 2 | `Compilador_Analise_blocos_sysml.png` | `fig:AnaliseBloc` | 3 — Análise | SysML: blocos da fase de análise |
| 3 | `Compilador_Analise_sysml.png` | `fig:AnaliseAtiv` | 3 — Análise | SysML: atividades da fase de análise |
| 4 | `Compilador_Sintese_blocos_sysml.png` | `fig:SinteseBloc` | 4 — Síntese | SysML: blocos da fase de síntese |
| 5 | `Compilador_Sintese_sysml.png` | `fig:SinteseAtiv` | 4 — Síntese | SysML: atividades da fase de síntese |

---

## Imagens removidas do relatório (opcionais)

Estas figuras existiam na versão anterior focada em hardware. Podem ser ignoradas no Overleaf deste relatório ou reutilizadas no relatório do processador:

- `DiagramaBlocos_Processador.png`, `UnidadeControle.png`
- `MemDados.png`, `MemInstr.png`, `Adder.png`, `ExtBits.png`, `Debounce.png`
- `ULA_24.png`, `bncReg24.png`, `PC.png`
- `MuxRDst.png`, `MuxJal.png`, `MuxJalWrite.png`, `MuxAluSrc.png`, `MuxJump.png`, `MuxBranch.png`, `MuxMTR.png`, `MuxJReg.png`

---

## Imagens opcionais (melhorias futuras)

| Arquivo sugerido | Uso |
|------------------|-----|
| `Pipeline_Compilador.png` | Fluxo `.cms` → tokens → AST → `.tm` → `.s` → `.txt` |
| `AST_exemplo.png` | Árvore sintática de trecho C- |
| `FPGA_resultado.png` | Validação: display ou simulação com binário gerado |

---

## Estrutura no Overleaf

```
projeto-overleaf/
├── relatorio_compiladores.tex
└── Imagens/
    ├── Datapath.png
    ├── Compilador_Analise_blocos_sysml.png
    ├── Compilador_Analise_sysml.png
    ├── Compilador_Sintese_blocos_sysml.png
    └── Compilador_Sintese_sysml.png
```

---

## Checklist

- [ ] Pasta `Imagens/` criada
- [ ] 5 arquivos obrigatórios enviados
- [ ] `Datapath.png` — visão funcional (não precisa ser screenshot Verilog)
- [ ] 4 diagramas SysML do compilador
- [ ] Compilar PDF e conferir Lista de Ilustrações (5 figuras)
