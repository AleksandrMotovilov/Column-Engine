#!/usr/bin/env bash
set -euo pipefail

# Compression flags — edit here to change encoding used in the output .clmn file.
# After changing, the script rebuilds csv_to_clmn automatically before converting.
ENABLE_RLE=ON
ENABLE_DELTA=ON
ENABLE_DICT=ON
ENABLE_LZ4=ON
ENABLE_BITPACK=ON

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
BIN="${BUILD_DIR}/exe/csv_to_clmn"

usage() {
    echo "Usage: script/convert.sh <input_csv> <output_columnar> [input_schema]" >&2
    echo "  input_schema defaults to clickbench/hits.schema" >&2
    exit 1
}

if [[ $# -lt 2 ]]; then
    usage
fi

INPUT_CSV="$1"
OUTPUT_CLMN="$2"
INPUT_SCHEMA="${3:-${ROOT_DIR}/clickbench/hits.schema}"

export CC="${CC:-clang-20}"
export CXX="${CXX:-clang++-20}"

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=OFF \
    -DENABLE_RLE="${ENABLE_RLE}" \
    -DENABLE_DELTA="${ENABLE_DELTA}" \
    -DENABLE_DICT="${ENABLE_DICT}" \
    -DENABLE_LZ4="${ENABLE_LZ4}" \
    -DENABLE_BITPACK="${ENABLE_BITPACK}" \
    > /dev/null 2>&1

cmake --build "${BUILD_DIR}" --target csv_to_clmn -j "$(nproc)" > /dev/null 2>&1

if [[ ! -f "${INPUT_CSV}" ]]; then
    echo "ERROR: input CSV not found: ${INPUT_CSV}" >&2
    exit 2
fi

if [[ ! -f "${INPUT_SCHEMA}" ]]; then
    echo "ERROR: schema file not found: ${INPUT_SCHEMA}" >&2
    exit 2
fi

mkdir -p "$(dirname "${OUTPUT_CLMN}")"

"${BIN}" --input "${INPUT_CSV}" --schema "${INPUT_SCHEMA}" --output "${OUTPUT_CLMN}"
