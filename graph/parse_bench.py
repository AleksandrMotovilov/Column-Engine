#!/usr/bin/env python3

"""
Parse bench_queries.sh output from a txt file and print a dict {query_num: time_ms}.

Usage:
    python3 graphs/parse_bench.py <bench_output.txt>

Input format (one line per query):
    Q00: 0.012s
    Q01: 0.008s
    Q21: N/A

Output (printed to stdout):
    {0: 12.0, 1: 8.0, ...}   — only implemented queries, time in ms
"""

import re
import sys


def parse_bench(path: str) -> dict[int, float]:
    result: dict[int, float] = {}
    pattern = re.compile(r"Q(\d+):\s+([\d.]+)s")
    with open(path, encoding="utf-8") as f:
        for line in f:
            m = pattern.match(line.strip())
            if m:
                query_num = int(m.group(1))
                time_s = float(m.group(2))
                result[query_num] = round(time_s * 1000, 3)
    return result


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: python {sys.argv[0]} <bench_output.txt>", file=sys.stderr)
        sys.exit(1)

    data = parse_bench(sys.argv[1])
    print(data)
