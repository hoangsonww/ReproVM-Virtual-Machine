#!/usr/bin/env bash
set -euo pipefail

# Require clang-format; fallback to other if not present
CLANG_FORMAT=${CLANG_FORMAT:-clang-format}
UNCRUSTIFY=${UNCRUSTIFY:-uncrustify}
ASTYLE=${ASTYLE:-astyle}

echo "Formatting C sources with clang-format..."
# apply in-place
find . -type f \( -name '*.c' -o -name '*.h' \) -not -path "./.reprovm/*" | while read -r f; do
    $CLANG_FORMAT -i --style=file "$f"
done

# optional: also run uncrustify for comparison / as alternative
if command -v $UNCRUSTIFY >/dev/null 2>&1; then
    echo "Optionally checking with uncrustify (dry-run)..."
    find . -type f \( -name '*.c' -o -name '*.h' \) -not -path "./.reprovm/*" | while read -r f; do
        $UNCRUSTIFY -c .uncrustify.cfg --check "$f" || true
    done
fi

# optional: astyle (if you prefer its style) - uncomment to enable
# if command -v $ASTYLE >/dev/null 2>&1; then
#     echo "Formatting with astyle (google style)..."
#     find . -type f \( -name '*.c' -o -name '*.h' \) -not -path "./.reprovm/*" | while read -r f; do
#         $ASTYLE --options=.astylerc "$f"
#     done
# fi

echo "Formatting complete."
