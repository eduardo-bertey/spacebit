#!/usr/bin/env bash
# Compatibilidad: compila un ejecutable Au/Plus.
# Uso: bash test/build/build_one.sh <dir_fuentes> <main.c> <salida.exe> [flags gcc...]
set -euo pipefail
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec bash "$DIR/build.sh" "$@"
