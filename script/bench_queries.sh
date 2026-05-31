#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

COLUMNAR="${1:-${ROOT_DIR}/clickbench/hits_sample.clmn}"
BIN="${ROOT_DIR}/build/exe/run_query"
TOTAL_QUERIES=43

if [[ ! -x "${BIN}" ]]; then
    echo "ERROR: run_query not found at ${BIN}. Run script/build.sh first." >&2
    exit 1
fi

if [[ ! -f "${COLUMNAR}" ]]; then
    echo "ERROR: columnar file not found: ${COLUMNAR}" >&2
    echo "Run: script/convert.sh clickbench/hits_sample.csv clickbench/hits_sample.clmn" >&2
    exit 1
fi

TEMP_DIR="$(mktemp -d)"
cleanup() { rm -rf "${TEMP_DIR}"; }
trap cleanup EXIT

printf "%-6s  %10s\n" "Query" "Time (s)"
printf "%-6s  %10s\n" "------" "----------"

sum_ns=0
count=0

for ((q = 0; q < TOTAL_QUERIES; q++)); do
    padded="$(printf "%02d" "${q}")"
    log="${TEMP_DIR}/q${padded}.log"

    start_ns="$(date +%s%N)"
    if "${BIN}" --input "${COLUMNAR}" --output_dir "${TEMP_DIR}" --queries="${q}" \
            > "${log}" 2>&1; then
        end_ns="$(date +%s%N)"
        elapsed_ns=$(( end_ns - start_ns ))
        elapsed="$(echo "scale=3; ${elapsed_ns} / 1000000000" | bc)"
        printf "Q%-5s  %10s\n" "${padded}" "${elapsed}"
        sum_ns=$(( sum_ns + elapsed_ns ))
        count=$(( count + 1 ))
    else
        printf "Q%-5s  %10s\n" "${padded}" "N/A"
    fi
done

printf "%-6s  %10s\n" "------" "----------"
if [[ ${count} -gt 0 ]]; then
    avg="$(echo "scale=3; ${sum_ns} / ${count} / 1000000000" | bc)"
    printf "%-6s  %10s\n" "Avg" "${avg}"
else
    printf "%-6s  %10s\n" "Avg" "N/A"
fi
