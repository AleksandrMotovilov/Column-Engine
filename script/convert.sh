#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="${ROOT_DIR}/build/exe/csv_to_clmn"

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
