#!/usr/bin/env bash
# Compila un ejecutable del Plus, opcionalmente sustituyendo keygen.c por
# keygen_zero.c (serial cero) para la prueba del hueco analogo.
# Uso:
#   bash test/build/build_plus.sh <dir_fuentes> <main.c> <salida.exe>
#   bash test/build/build_plus.sh <dir_fuentes> <main.c> <salida.exe> zero
set -euo pipefail
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC="${1:?falta dir de fuentes}"
MAIN="${2:?falta main.c}"
OUT="${3:?falta nombre.exe}"
MODO="${4:-normal}"

if [ "$MODO" = "zero" ]; then
  export SKIP_EXTRA="keygen.c"
  export EXTRA_SRC="$SRC/keygen_zero.c"
  echo "==> $OUT con keygen_zero.c (serial cero)"
fi

exec bash "$DIR/build.sh" "$SRC" "$MAIN" "$OUT"
