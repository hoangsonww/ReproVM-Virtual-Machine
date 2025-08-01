#!/usr/bin/env bash
# Clean local ReproVM cache and CAS

set -euo pipefail

echo "Removing .reprovm cache and objects..."
rm -rf .reprovm/cache .reprovm/cas
echo "Done."
