#!/bin/bash

# Memory leak detection with Valgrind

set -e

echo "=== Running Valgrind Memory Leak Detection ==="

# Check if valgrind is available
if ! command -v valgrind &> /dev/null; then
    echo "Valgrind not found, skipping memory leak detection"
    exit 0
fi

# Create a simple test manifest
cat > test_manifest_valgrind.txt <<EOF
task test {
  cmd = echo "Memory leak test"
  inputs =
  outputs = test_output.txt
  deps =
}
EOF

# Run with valgrind
echo "Running ReproVM with Valgrind..."
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --log-file=valgrind-out.txt \
         ../reprovm test_manifest_valgrind.txt

# Check results
if grep -q "ERROR SUMMARY: 0 errors" valgrind-out.txt; then
    echo "✓ No memory leaks detected"
    exit 0
else
    echo "✗ Memory leaks detected!"
    cat valgrind-out.txt
    exit 1
fi
