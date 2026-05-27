#!/usr/bin/env bash

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

COLUMNAR="${1:-${ROOT_DIR}/clickbench/hits_sample.clmn}"
RESULTS_DIR="${ROOT_DIR}/clickbench/results/small"
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

passed=0
failed=0
skipped=0

for ((q = 0; q < TOTAL_QUERIES; q++)); do
    padded="$(printf "%02d" "${q}")"
    expected="${RESULTS_DIR}/query_${padded}.csv"
    actual="${TEMP_DIR}/q${padded}.csv"
    log="${TEMP_DIR}/q${padded}.log"

    if [[ ! -f "${expected}" ]]; then
        echo "Q${padded}: SKIP (no reference file)"
        ((skipped++))
        continue
    fi

    if "${BIN}" --input "${COLUMNAR}" --output_dir "${TEMP_DIR}" --queries="${q}" \
            > "${log}" 2>&1; then
        if [[ -f "${actual}" ]] && diff -q "${actual}" "${expected}" > /dev/null 2>&1; then
            echo "Q${padded}: OK"
            ((passed++))
        else
            echo "Q${padded}: WRONG"
            if [[ -f "${actual}" ]]; then
                echo "  got:      $(cat "${actual}")"
            else
                echo "  got:      (no output file)"
            fi
            echo "  expected: $(cat "${expected}")"
            ((failed++))
        fi
    else
        echo "Q${padded}: NOT IMPLEMENTED"
        ((skipped++))
    fi
done

echo ""
echo "Results: ${passed} passed, ${failed} failed, ${skipped} skipped"
[[ "${failed}" -eq 0 ]]
