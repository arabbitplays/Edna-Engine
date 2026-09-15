#!/usr/bin/env bash
set -euo pipefail

SOURCE_ROOT="$1"
BUILD_ROOT="$2"
CLANG_TIDY="$3"

run-clang-tidy -p "$BUILD_ROOT" -clang-tidy-binary "$CLANG_TIDY" -header-filter='.*' \
  -fix "$SOURCE_ROOT/src"
