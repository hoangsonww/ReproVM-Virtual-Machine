#!/usr/bin/env bash
set -euo pipefail

ROOT=$(dirname "$0")/..
cd "$ROOT"

echo "Building project..."
make

echo
echo "=== Running individual tests ==="
./tests/test_util.sh
./tests/test_cas.sh
./tests/test_manifest.sh
./tests/test_parallel.sh
./tests/test_crc32.sh

echo
echo "ALL TESTS PASSED"
