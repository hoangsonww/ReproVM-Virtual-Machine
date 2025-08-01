#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

echo -n "abc" > tests/abc.txt

if [ ! -x ./crc32_asm ]; then
  echo "SKIP: crc32_asm binary missing"
  exit 0
fi

./crc32_asm tests/abc.txt > tests/crc_out.txt 2>/dev/null
expected="352441c2"
got=$(tr -d '\n' < tests/crc_out.txt)
if [ "$got" != "$expected" ]; then
  echo "FAIL: crc32 expected $expected got $got"
  exit 1
fi
echo "PASS: crc32"
