#!/usr/bin/env bash
# Compila UN ejecutable desde cualquier carpeta de fuentes (Au o Plus).
# Uso:
#   bash test/build/build.sh <dir_fuentes> <main.c> <salida.exe> [flags gcc...]
#
# Variables de entorno opcionales:
#   SKIP_EXTRA=keygen.c            excluye ademas ese archivo de la libreria
#   EXTRA_SRC=ruta/al/archivo.c    agrega esa fuente (ej. keygen_zero.c)
set -euo pipefail

SRC_REL="$1"
MAIN_REL="$2"
OUT="$3"
shift 3

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
if [[ "$SRC_REL" = /* ]]; then
  SRC="$SRC_REL"
else
  SRC="$ROOT/$SRC_REL"
fi
if [[ "$MAIN_REL" = /* ]]; then
  MAIN="$MAIN_REL"
elif [[ "$MAIN_REL" = test/* ]]; then
  MAIN="$ROOT/$MAIN_REL"
else
  MAIN="$SRC/$MAIN_REL"
fi

[ -d "$SRC" ] || { echo "No existe la carpeta de fuentes: $SRC" >&2; exit 1; }
[ -f "$MAIN" ] || { echo "No existe el main: $MAIN" >&2; exit 1; }

mkdir -p "$ROOT/artifacts"

# Todos los main() de prueba y randombytes.c (se usa el RNG portable).
SKIP='^(randombytes\.c|test\.c|test_todos\.c|test_firma_falsa\.c|test_falsificador\.c|test_auditor\.c|test_l\.c|test_l499\.c|test_multi\.c|test_casos\.c|test_trampas\.c|th_a\.c|th_b\.c|th_c\.c|probe_gadget\.c|probe2\.c|tp1\.c|keygen_zero\.c)$'
IFS=',' read -ra EXTRA_SKIP <<< "${SKIP_EXTRA:-}"

LIB=()
while IFS= read -r f; do
  [ -z "$f" ] && continue
  if echo "$f" | grep -Eq "$SKIP"; then
    continue
  fi
  salta=0
  for e in "${EXTRA_SKIP[@]}"; do
    [ "$f" = "$e" ] && salta=1
  done
  [ "$salta" -eq 1 ] || LIB+=("$SRC/$f")
done < <(cd "$SRC" && ls -1 *.c)

EXTRA=()
if [ -n "${EXTRA_SRC:-}" ]; then
  if [[ "$EXTRA_SRC" = /* ]]; then
    EXTRA=("$EXTRA_SRC")
  else
    EXTRA=("$ROOT/$EXTRA_SRC")
  fi
fi

echo "==> $OUT  (fuentes: $SRC, main: $MAIN_REL)"
gcc -O2 -maes -mavx2 -mfma -static \
  -I "$SRC" -I "$ROOT/test" -I "$ROOT/create/native/include" \
  "$@" \
  "$MAIN" "${LIB[@]}" "${EXTRA[@]}" \
  "$ROOT/create/native/shake256.c" \
  "$ROOT/test/randombytes_portable.c" \
  -o "$ROOT/artifacts/$OUT" -lm
