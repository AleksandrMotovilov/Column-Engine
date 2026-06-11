#!/usr/bin/env bash
set -euo pipefail

ENABLE_RLE=ON
ENABLE_DELTA=OFF
ENABLE_DICT=ON
ENABLE_LZ4=ON
ENABLE_BITPACK=ON

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"

if [[ $# -lt 4 ]]; then
    echo "Usage: script/run_query.sh <query_num> <columnar> <output_csv> <log_file>" >&2
    exit 2
fi

QUERY_NUM="$1"
COLUMNAR="$2"
OUTPUT_CSV="$3"
LOG_FILE="$4"
QUERY_NUM_PADDED="$(printf "%02d" "${QUERY_NUM}")"

BIN="${BUILD_DIR}/exe/run_query"

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

cmake --build "${BUILD_DIR}" --target run_query -j "$(nproc)" > /dev/null 2>&1

if [[ ! -f "${COLUMNAR}" ]]; then
    echo "ERROR: columnar file not found: ${COLUMNAR}" >&2
    exit 2
fi

mkdir -p "$(dirname "${OUTPUT_CSV}")"
mkdir -p "$(dirname "${LOG_FILE}")"

TEMP_OUTPUT_DIR="$(mktemp -d)"
cleanup() { rm -rf "${TEMP_OUTPUT_DIR}"; }
trap cleanup EXIT

"${BIN}" \
    --input "${COLUMNAR}" \
    --output_dir "${TEMP_OUTPUT_DIR}" \
    --queries="${QUERY_NUM}" \
    2>&1 | tee "${LOG_FILE}"

EXPECTED_CSV="${TEMP_OUTPUT_DIR}/q${QUERY_NUM_PADDED}.csv"
if [[ -f "${EXPECTED_CSV}" ]]; then
    mv "${EXPECTED_CSV}" "${OUTPUT_CSV}"
    exit 0
fi

echo "ERROR: query output not found for Q${QUERY_NUM} in ${TEMP_OUTPUT_DIR}" >&2
exit 3
