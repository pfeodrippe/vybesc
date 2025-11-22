#!/usr/bin/env bash
# Build and run VybeSC tests

set -euo pipefail

readonly ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build}"

echo "Building tests..."
cmake --build "${BUILD_DIR}" --config Debug --target test_dlopen

echo ""
echo "Running test_dlopen..."
"${BUILD_DIR}/tests/test_dlopen"

echo ""
echo "Alternatively, run via ctest:"
echo "  cd ${BUILD_DIR}"
echo "  ctest --output-on-failure --verbose"
