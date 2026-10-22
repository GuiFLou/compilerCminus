#!/usr/bin/env bash
# Sem --target, regenera a ROM solta em exemplos/.
# Com --target=os-user, grava a folha em processador/hd_image.txt.
# exemplos/ não é a imagem que o SO executa.

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
EXAMPLES_DIR="$SCRIPT_DIR/exemplos"
HD_IMAGE="$SCRIPT_DIR/../processador/hd_image.txt"

compile_compiler=0
no_test=0
target_os_user=0
requested_programs=()

usage() {
    cat <<EOF
Uso: $(basename "$0") [opções] [programa ...]

Opções:
  --compile         compila o compilador antes dos testes
  --no-test         remove os arquivos gerados e não recompila os exemplos
  --target=os-user  compila a folha e grava nos setores 20–29 do hd_image
  --help            mostra esta ajuda

Sem --target, o script remove os .s, .tm, .txt e .mem de exemplos/ e
recompila a ROM que nasce no endereço 0.

Com --target=os-user, exemplos/ permanece. Cada programa conhecido é
gravado no próprio setor de ../processador/hd_image.txt. O restante do
disco não muda. Um .cms sem setor fica para a gestão de arquivos.

Sem nomes de programas, todos os arquivos .cms em exemplos/ entram.
Exemplo:
  $(basename "$0") --compile fatorial gcd
  $(basename "$0") --target=os-user sort
EOF
}

sector_of() {
    case "$1" in
        area_triangulo) printf '20\n' ;;
        busca_linear) printf '21\n' ;;
        fatorial) printf '22\n' ;;
        fibonacci) printf '23\n' ;;
        gcd) printf '24\n' ;;
        media) printf '25\n' ;;
        numero_primo) printf '26\n' ;;
        potencia) printf '27\n' ;;
        soma_digitos) printf '28\n' ;;
        sort) printf '29\n' ;;
        *) printf '\n' ;;
    esac
}

while (($# > 0)); do
    case "$1" in
        --compile)
            compile_compiler=1
            ;;
        --no-test)
            no_test=1
            ;;
        --target=os-user)
            target_os_user=1
            ;;
        --target=*)
            printf 'Erro: alvo desconhecido: %s\n' "$1" >&2
            usage >&2
            exit 2
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

if ((compile_compiler)); then
    printf 'Compilando o compilador...\n'
    make -C "$SCRIPT_DIR"
fi

if ((no_test)); then
    if ((target_os_user)); then
        printf 'Erro: --no-test não grava o hd_image.\n' >&2
        exit 2
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
    exit 0
fi

if [[ ! -x "$SCRIPT_DIR/compilador" ]]; then
    printf 'Erro: compilador não encontrado. Execute com --compile.\n' >&2
    exit 1
fi

shopt -s nullglob
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

if ((target_os_user)); then
    if [[ ! -f "$HD_IMAGE" ]]; then
        printf 'Erro: imagem do disco não encontrada: %s\n' "$HD_IMAGE" >&2
        exit 1
    fi

    mapped_names=()
    mapped_sectors=()
    unmapped_names=()
    for program in "${programs[@]}"; do
        name="$(basename "$program")"
        sector="$(sector_of "$name")"
        if [[ -n "$sector" ]]; then
            mapped_names+=("$name")
            mapped_sectors+=("$sector")
        else
            unmapped_names+=("$name")
        fi
    done

    if ((${#unmapped_names[@]} > 0)); then
        printf 'Sem setor nesta imagem:' >&2
        for name in "${unmapped_names[@]}"; do
            printf ' %s.cms' "$name" >&2
        done
        printf '\nA gestão de arquivos define o destino de um programa novo.\n' >&2
        if ((${#requested_programs[@]} > 0)); then
            exit 1
        fi
    fi

    if ((${#mapped_names[@]} == 0)); then
        printf 'Erro: nenhum programa mapeado para gravar no disco.\n' >&2
        exit 1
    fi

    work="$(mktemp -d)"
    trap 'rm -rf "$work"' EXIT
    manifest="$work/manifest"

    index=0
    while ((index < ${#mapped_names[@]})); do
        name="${mapped_names[$index]}"
        sector="${mapped_sectors[$index]}"
        slot="$work/$name"
        mkdir -p "$slot"
        cp "$EXAMPLES_DIR/$name.cms" "$slot/$name.cms"
        printf 'Compilando %s.cms para o setor %s...\n' "$name" "$sector"
        "$SCRIPT_DIR/compilador" --txt --target=os-user "$slot/$name.cms" >/dev/null
        if [[ ! -f "$slot/$name.txt" ]]; then
            printf 'Erro: %s.cms não gerou binário.\n' "$name" >&2
            exit 1
        fi
        printf '%s %s\n' "$sector" "$slot/$name.txt" >> "$manifest"
        index=$((index + 1))
    done

    python3 - "$HD_IMAGE" "$manifest" <<'PY'
import os
import pathlib
import sys

hd_path = pathlib.Path(sys.argv[1])
manifest = pathlib.Path(sys.argv[2])
raw = hd_path.read_bytes()
newline = b"\r\n" if b"\r\n" in raw else b"\n"
lines = raw.splitlines()
if len(lines) != 16384:
    sys.stderr.write(
        "hd_image tem %d linhas; o disco tem 16384\n" % len(lines)
    )
    sys.exit(1)

zero = b"0" * 32
for entry in manifest.read_text().splitlines():
    sector_text, txt_path = entry.split(None, 1)
    sector = int(sector_text)
    if sector < 0 or sector > 63:
        sys.stderr.write("setor fora do disco: %d\n" % sector)
        sys.exit(1)
    words = []
    for word in pathlib.Path(txt_path).read_text().splitlines():
        if word == "":
            continue
        if len(word) != 32 or word.strip("01") != "":
            sys.stderr.write("palavra inválida em %s\n" % txt_path)
            sys.exit(1)
        words.append(word.encode())
    if not words or len(words) > 256:
        sys.stderr.write(
            "%s tem %d palavras; um setor tem 256\n" % (txt_path, len(words))
        )
        sys.exit(1)
    start = sector * 256
    lines[start:start + 256] = words + [zero] * (256 - len(words))

payload = newline.join(lines)
if raw.endswith(b"\n"):
    payload += newline
temporary = hd_path.with_name(hd_path.name + ".tmp")
temporary.write_bytes(payload)
os.replace(temporary, hd_path)
PY

    index=0
    while ((index < ${#mapped_names[@]})); do
        printf '%s.cms gravado no setor %s\n' \
            "${mapped_names[$index]}" "${mapped_sectors[$index]}"
        index=$((index + 1))
    done
    printf 'Concluído: %d setor(es) atualizado(s) em hd_image.txt\n' \
        "${#mapped_names[@]}"
    exit 0
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

for program in "${programs[@]}"; do
    printf 'Compilando %s.cms...\n' "$(basename "$program")"
    "$SCRIPT_DIR/compilador" --txt "$program.cms" >/dev/null
done

printf 'Concluído: %d programa(s) compilado(s).\n' "${#programs[@]}"
