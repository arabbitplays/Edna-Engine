#!/usr/bin/env bash
set -euo pipefail

SOURCE_ROOT="$1"
BUILD_ROOT="$2"
CLANG_TIDY="$3"

HEADER_FILTER="^${SOURCE_ROOT}/"

while IFS= read -r -d '' file; do
    "$CLANG_TIDY" -p "$BUILD_ROOT" -header-filter="$HEADER_FILTER" --fix-errors "$file"
done < <(find "$SOURCE_ROOT/src" -type f \( -name '*.cpp' -o -name '*.cc' -o -name '*.cxx' \) -print0)
