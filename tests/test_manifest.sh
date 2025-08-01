#!/usr/bin/env bash
set -euo pipefail

# integration test for serial manifest execution
ROOT=$(dirname "$0")/..
cd "$ROOT"

echo "Running manifest integration test..."

# prepare workspace
rm -rf tests/tmp_manifest
mkdir -p tests/tmp_manifest
cd tests/tmp_manifest

# hello.c
cat <<'EOF' > hello.c
#include <stdio.h>
int main(void){ puts("Hello, ReproVM!"); return 0;}
EOF

# manifest
cat <<'EOF' > manifest.txt
task build {
  cmd = gcc -o hello hello.c
  inputs = hello.c
  outputs = hello
  deps =
}
task test {
  cmd = ./hello > result.txt
  inputs = hello
  outputs = result.txt
  deps = build
}
task checksum {
  cmd = sha256sum result.txt > result.sha
  inputs = result.txt
  outputs = result.sha
  deps = test
}
EOF

# initial run
"$ROOT"/reprovm manifest.txt > run1.log 2>&1

# verify outputs
if [ ! -f hello ] || [ ! -f result.txt ] || [ ! -f result.sha ]; then
  echo "FAIL: manifest initial run missing outputs"
  exit 1
fi

# second run, expect cache hits
"$ROOT"/reprovm manifest.txt > run2.log 2>&1
if ! grep '\[\*\]' run2.log >/dev/null; then
  echo "FAIL: expected cache hits on second run"
  exit 1
fi

# modify input
cat <<'EOF' > hello.c
#include <stdio.h>
int main(void){ puts("Hello, Updated ReproVM!"); return 0;}
EOF

"$ROOT"/reprovm manifest.txt > run3.log 2>&1
if ! grep "Running task 'build'" run3.log >/dev/null; then
  echo "FAIL: build after change did not run"
  exit 1
fi

echo "PASS: manifest integration"
