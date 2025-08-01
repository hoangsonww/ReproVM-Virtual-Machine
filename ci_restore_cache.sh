#!/usr/bin/env bash
# Requires AWS CLI configured with credentials.

set -euo pipefail

S3_BUCKET=${1:-"s3://my-reprovm-cache"}
LOCAL_ROOT=${2:-"."}

echo "Restoring cache from ${S3_BUCKET} to ${LOCAL_ROOT}/.reprovm"
aws s3 sync "${S3_BUCKET}/cache" "${LOCAL_ROOT}/.reprovm/cache" || true
aws s3 sync "${S3_BUCKET}/cas/objects" "${LOCAL_ROOT}/.reprovm/cas/objects" || true
echo "Restore complete."
