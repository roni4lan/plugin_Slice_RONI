#!/usr/bin/env bash
set -euo pipefail
export MSYSTEM=MINGW64
export MINGW_PREFIX=/mingw64
export PATH=/mingw64/bin:/usr/bin
cd "$1"
make -j"${2:-4}" Release APPNAME="${3:-SliceBorders}"
make copy_dlls APPNAME="${3:-SliceBorders}"
