#!/usr/bin/env bash
set -euo pipefail

SOURCE_ROOT="$1"
CLANG_FORMAT="$2"

while IFS= read -r file; do
    echo "  formatting $file"
    "$CLANG_FORMAT" -i "$file"
done < <(find "$SOURCE_ROOT/src" "$SOURCE_ROOT/include" \
    \( -name "*.cpp" -o -name "*.hpp" \) -print)

echo "Done."
