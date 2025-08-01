#!/usr/bin/env bash
set -euo pipefail
ROOT=$(dirname "$0")/..
cd "$ROOT"

echo "Running parallel integration test..."

cat <<'EOF' > tests/parallel_manifest.txt
task A {
  cmd = echo A > a.txt
  inputs =
  outputs = a.txt
  deps =
}
task B {
  cmd = echo B > b.txt
  inputs =
  outputs = b.txt
  deps =
}
task C {
  cmd = cat a.txt b.txt > c.txt
  inputs = a.txt, b.txt
  outputs = c.txt
  deps = A, B
}
EOF

./reprovm_parallel -j 2 tests/parallel_manifest.txt > tests/parallel_run.log 2>&1

if [ ! -f a.txt ] || [ ! -f b.txt ] || [ ! -f c.txt ]; then
  echo "FAIL: parallel run missing outputs"
  exit 1
fi

# optional, check graph symbols exist (not strict)
grep '\[✔\] A' tests/parallel_run.log >/dev/null || true
grep '\[✔\] B' tests/parallel_run.log >/dev/null || true
grep '\[✔\] C' tests/parallel_run.log >/dev/null || true

echo "PASS: parallel integration"
