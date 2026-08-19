#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
EXAMPLES_DIR="$SCRIPT_DIR/exemplos"

compile_compiler=0
no_test=0
requested_programs=()

usage() {
    cat <<EOF
Uso: $(basename "$0") [opções] [programa ...]

Opções:
  --compile       compila o compilador antes dos testes
  --no-test       remove os arquivos gerados e não recompila os exemplos
  --help          mostra esta ajuda

Sem nomes de programas, todos os arquivos .cms em exemplos/ são compilados.
Exemplo:
  $(basename "$0") --compile fatorial gcd
EOF
}

while (($# > 0)); do
    case "$1" in
        --compile)
            compile_compiler=1
            ;;
        --no-test)
            no_test=1
            ;;
        --help|-h)
            usage
            exit 0
            ;;
        -*)
            printf 'Erro: opção desconhecida: %s\n\n' "$1" >&2
            usage >&2
            exit 2
            ;;
        *)
            requested_programs+=("${1%.cms}")
            ;;
    esac
    shift
done

if [[ ! -d "$EXAMPLES_DIR" ]]; then
    printf 'Erro: diretório de exemplos não encontrado: %s\n' "$EXAMPLES_DIR" >&2
    exit 1
fi

shopt -s nullglob
for generated_file in \
    "$EXAMPLES_DIR"/*.s \
    "$EXAMPLES_DIR"/*.tm \
    "$EXAMPLES_DIR"/*.txt \
    "$EXAMPLES_DIR"/*.mem; do
    rm -f -- "$generated_file"
done
printf 'Arquivos .s, .tm, .txt e .mem antigos removidos de exemplos/\n'

if ((compile_compiler)); then
    printf 'Compilando o compilador...\n'
    make -C "$SCRIPT_DIR"
fi

if ((no_test)); then
    exit 0
fi

if [[ ! -x "$SCRIPT_DIR/compilador" ]]; then
    printf 'Erro: compilador não encontrado. Execute com --compile.\n' >&2
    exit 1
fi

programs=()
if ((${#requested_programs[@]} == 0)); then
    for source_file in "$EXAMPLES_DIR"/*.cms; do
        programs+=("${source_file%.cms}")
    done
else
    for program in "${requested_programs[@]}"; do
        if [[ "$program" == */* || ! -f "$EXAMPLES_DIR/$program.cms" ]]; then
            printf 'Erro: programa não encontrado em exemplos/: %s.cms\n' "$program" >&2
            exit 1
        fi
        programs+=("$EXAMPLES_DIR/$program")
    done
fi

for program in "${programs[@]}"; do
    printf 'Compilando %s.cms...\n' "$(basename "$program")"
    "$SCRIPT_DIR/compilador" --txt "$program.cms" >/dev/null
done

printf 'Concluído: %d programa(s) compilado(s).\n' "${#programs[@]}"
