#!/bin/bash

# Performance benchmark script

set -e

echo "=== ReproVM Performance Benchmarks ==="

BENCHMARK_RESULTS="../benchmark-results.txt"
echo "Benchmark Results - $(date)" > $BENCHMARK_RESULTS
echo "=====================================" >> $BENCHMARK_RESULTS

# Benchmark 1: Simple serial execution
echo ""
echo "Benchmark 1: Simple Serial Execution"
cat > bench_manifest.txt <<EOF
task compile {
  cmd = gcc -c ../hello.c -o hello.o
  inputs = ../hello.c
  outputs = hello.o
  deps =
}

task link {
  cmd = gcc hello.o -o hello_bench
  inputs = hello.o
  outputs = hello_bench
  deps = compile
}

task run {
  cmd = ./hello_bench > output.txt
  inputs = hello_bench
  outputs = output.txt
  deps = link
}
EOF

START=$(date +%s.%N)
../reprovm bench_manifest.txt > /dev/null 2>&1
END=$(date +%s.%N)
DURATION=$(echo "$END - $START" | bc)
echo "Serial execution time: ${DURATION}s"
echo "Benchmark 1 - Serial: ${DURATION}s" >> $BENCHMARK_RESULTS

# Clean cache
rm -rf .reprovm

# Benchmark 2: Parallel execution
echo ""
echo "Benchmark 2: Parallel Execution (4 workers)"
START=$(date +%s.%N)
../reprovm_parallel -j 4 bench_manifest.txt > /dev/null 2>&1
END=$(date +%s.%N)
DURATION=$(echo "$END - $START" | bc)
echo "Parallel execution time: ${DURATION}s"
echo "Benchmark 2 - Parallel (4 workers): ${DURATION}s" >> $BENCHMARK_RESULTS

# Benchmark 3: Cache performance
echo ""
echo "Benchmark 3: Cache Hit Performance"
START=$(date +%s.%N)
../reprovm bench_manifest.txt > /dev/null 2>&1
END=$(date +%s.%N)
DURATION=$(echo "$END - $START" | bc)
echo "Cached execution time: ${DURATION}s"
echo "Benchmark 3 - Cached: ${DURATION}s" >> $BENCHMARK_RESULTS

# Cleanup
rm -f bench_manifest.txt hello.o hello_bench output.txt

echo ""
echo "Benchmark results saved to $BENCHMARK_RESULTS"
cat $BENCHMARK_RESULTS
