#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

echo "Compiling and running test_util..."
gcc -std=c99 -O2 -Wall -Wextra -g util.c tests/test_util.c -o tests/test_util
./tests/test_util
echo "PASS: util"
