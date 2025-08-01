#!/usr/bin/env bash
# run_pipeline.sh - wrapper to invoke ReproVM (parallel if available), with logging.

set -euo pipefail

MANIFEST=${1:-manifest.txt}
JOBS=${REPROVM_JOBS:-0}

log() { echo "[$(date +'%Y-%m-%dT%H:%M:%S%z')] $*"; }

# Build if needed
if [ ! -x ./reprovm_parallel ]; then
  log "Parallel binary not found; attempting to build."
  make || true
fi

# Choose binary
if [ -x ./reprovm_parallel ]; then
  BIN=./reprovm_parallel
else
  BIN=./reprovm
fi

# Determine jobs flag
JFLAG=()
if [ "$JOBS" != "0" ] && [ "$BIN" = "./reprovm_parallel" ]; then
  JFLAG=("-j" "$JOBS")
fi

log "Starting pipeline with manifest=${MANIFEST} using ${BIN}"
"$BIN" "${JFLAG[@]}" "$MANIFEST" "$@"
RC=$?
if [ $RC -ne 0 ]; then
  log "Pipeline failed (exit $RC)"
  exit $RC
else
  log "Pipeline completed successfully."
fi
