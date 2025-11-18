#!/usr/bin/env bash
# Build VybeSC and copy the resulting plugin into the SuperCollider plugin folders.
# Optionally accepts `BUILD_TYPE`, `RUNTIME_DIR`, and `RUNTIME_OBJECTS` via environment or CLI.

set -euo pipefail

readonly ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly BUILD_DIR="${BUILD_DIR:-build}"
readonly BUILD_TYPE="${BUILD_TYPE:-Debug}"
readonly CMAKE_FLAGS=(
  -S "${ROOT_DIR}"
  -B "${BUILD_DIR}"
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
  -DCMAKE_C_COMPILER="${CMAKE_C_COMPILER:-/usr/bin/clang}"
  -DCMAKE_CXX_COMPILER="${CMAKE_CXX_COMPILER:-/usr/bin/clang++}"
)

if [[ -n "${RUNTIME_DIR:-}" ]]; then
  CMAKE_FLAGS+=("-DRUNTIME_DIR=${RUNTIME_DIR}")
fi

if [[ -n "${RUNTIME_OBJECTS:-}" ]]; then
  CMAKE_FLAGS+=("-DRUNTIME_OBJECTS=${RUNTIME_OBJECTS}")
fi

echo "Configuring VybeSC (${BUILD_TYPE}) in ${BUILD_DIR}..."
cmake "${CMAKE_FLAGS[@]}"

echo "Building VybeSC (${BUILD_TYPE})..."
cmake --build "${BUILD_DIR}" --config "${BUILD_TYPE}"

readonly PLUGIN_NAME="VybeSC_scsynth.scx"
readonly SOURCE_PLUGIN="${BUILD_DIR}/${PLUGIN_NAME}"

if [[ ! -f "${SOURCE_PLUGIN}" ]]; then
  echo "ERROR: ${PLUGIN_NAME} not found in ${BUILD_DIR}." >&2
  exit 1
fi

readonly TARGET_PATHS=(
  "$HOME/dev/games/vybe_native/macos/universal/supercollider/Resources/plugins"
  "$HOME/dev/vybe/vybe_native/macos/universal/supercollider/Resources/plugins"
  "$HOME/dev/vybe/sonic-pi/prebuilt/macos/universal/supercollider/Resources/plugins"
)

echo "Copying ${PLUGIN_NAME} to plugin folders..."
for target in "${TARGET_PATHS[@]}"; do
  mkdir -p "${target}"
  cp "${SOURCE_PLUGIN}" "${target}/${PLUGIN_NAME}"
done

echo "Build and install complete."