#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

echo "Compiling and running test_cas..."
gcc -std=c99 -O2 -Wall -Wextra -g cas.c util.c tests/test_cas.c -o tests/test_cas
./tests/test_cas
echo "PASS: cas"
