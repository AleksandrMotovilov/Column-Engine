#!/usr/bin/env bash
set -euo pipefail

# Compression flags — edit here, then re-run this script to rebuild.
ENABLE_RLE=ON
ENABLE_DELTA=ON
ENABLE_DICT=ON
ENABLE_LZ4=ON
ENABLE_BITPACK=ON

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"

export CC="${CC:-clang-20}"
export CXX="${CXX:-clang++-20}"

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=OFF \
    -DENABLE_RLE="${ENABLE_RLE}" \
    -DENABLE_DELTA="${ENABLE_DELTA}" \
    -DENABLE_DICT="${ENABLE_DICT}" \
    -DENABLE_LZ4="${ENABLE_LZ4}" \
    -DENABLE_BITPACK="${ENABLE_BITPACK}"

cmake --build "${BUILD_DIR}" -j "$(nproc)"
