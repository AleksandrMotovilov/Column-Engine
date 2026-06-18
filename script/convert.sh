#!/usr/bin/env bash
set -euo pipefail

ENABLE_DELTA_INT=OFF
ENABLE_RLE_INT=ON
ENABLE_RLE_FLOAT=ON
ENABLE_RLE_STR=ON
ENABLE_DICT_INT=ON
ENABLE_DICT_FLOAT=ON
ENABLE_DICT_STR=ON
ENABLE_BITPACK_INT=ON
ENABLE_BITPACK_FLOAT=ON
ENABLE_BITPACK_STR=ON
ENABLE_LZ4=ON
ENABLE_ADAPTIVE=ON

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
    -DENABLE_RLE_INT="${ENABLE_RLE_INT}" \
    -DENABLE_RLE_FLOAT="${ENABLE_RLE_FLOAT}" \
    -DENABLE_RLE_STR="${ENABLE_RLE_STR}" \
    -DENABLE_DELTA_INT="${ENABLE_DELTA_INT}" \
    -DENABLE_DICT_INT="${ENABLE_DICT_INT}" \
    -DENABLE_DICT_FLOAT="${ENABLE_DICT_FLOAT}" \
    -DENABLE_DICT_STR="${ENABLE_DICT_STR}" \
    -DENABLE_LZ4="${ENABLE_LZ4}" \
    -DENABLE_BITPACK_INT="${ENABLE_BITPACK_INT}" \
    -DENABLE_BITPACK_FLOAT="${ENABLE_BITPACK_FLOAT}" \
    -DENABLE_BITPACK_STR="${ENABLE_BITPACK_STR}" \
    -DENABLE_ADAPTIVE="${ENABLE_ADAPTIVE}" \
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

start_ns="$(date +%s%N)"
"${BIN}" --input "${INPUT_CSV}" --schema "${INPUT_SCHEMA}" --output "${OUTPUT_CLMN}"
elapsed_ns=$(( $(date +%s%N) - start_ns ))
echo "Conversion time: $(echo "scale=3; ${elapsed_ns} / 1000000000" | bc) s"
