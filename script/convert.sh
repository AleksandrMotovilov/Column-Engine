#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

INPUT_CSV="${ROOT_DIR}/clickbench/hits_sample.csv"
OUTPUT_CLMN="${ROOT_DIR}/clickbench/hits_sample.clmn"
INPUT_SCHEMA="${ROOT_DIR}/clickbench/hits.schema"
BIN="${ROOT_DIR}/build/exe/csv_to_clmn"

if [[ ! -x "${BIN}" ]]; then
    echo "ERROR: csv_to_clmn not found at ${BIN}" >&2
    echo "Run script/build.sh first" >&2
    exit 1
fi

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
