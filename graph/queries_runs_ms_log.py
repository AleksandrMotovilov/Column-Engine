#!/usr/bin/env python3

import os

import matplotlib.pyplot as plt
import numpy as np

DATASET = "hits_sample"
BATCH_ROWS_NUMBER = 100000

# ─────────────────────────────────────────────────────────────────────────────
CE_COLD: dict[int, float | None] = {
    0: 1650.0, 1: 4180.0, 2: 6770.0, 3: 5420.0, 4: 18230.0, 5: 27070.0,
    6: 5030.0, 7: 4180.0, 8: 43010.0, 9: 49240.0, 10: 18660.0, 11: 20650.0,
    12: 55340.0, 13: 62480.0, 14: 65060.0, 15: 130020.0, 16: 205150.0,
    17: 189970.0, 18: None, 19: 6370.0, 20: 64620.0, 21: 85030.0, 22: 159940.0,
    23: 319050.0, 24: 38380.0, 25: 19180.0, 26: 38540.0, 27: 142220.0,
    28: None, 29: 3730.0, 30: 78340.0, 31: 121890.0, 32: None, 33: None,
    34: None, 35: 188250.0, 36: 79790.0, 37: 79400.0, 38: 80550.0,
    39: 138900.0, 40: 26160.0, 41: 24690.0, 42: 20210.0,
}

CE_HOT1: dict[int, float | None] = {
    0: 20.0, 1: 420.0, 2: 300.0, 3: 260.0, 4: 11830.0, 5: 15090.0,
    6: 990.0, 7: 580.0, 8: 34530.0, 9: 36840.0, 10: 5740.0, 11: 7210.0,
    12: 44070.0, 13: 47670.0, 14: 52930.0, 15: 124630.0, 16: 191450.0,
    17: 175720.0, 18: None, 19: 810.0, 20: 16300.0, 21: 23950.0, 22: 43530.0,
    23: 310060.0, 24: 21590.0, 25: 8070.0, 26: 22000.0, 27: 141560.0,
    28: None, 29: 170.0, 30: 56580.0, 31: 99160.0, 32: None, 33: None,
    34: None, 35: 182990.0, 36: 19680.0, 37: 17840.0, 38: 17860.0,
    39: 34770.0, 40: 5660.0, 41: 4420.0, 42: 4710.0,
}

CE_HOT2: dict[int, float | None] = {
    0: 20.0, 1: 420.0, 2: 300.0, 3: 260.0, 4: 11420.0, 5: 14810.0,
    6: 980.0, 7: 580.0, 8: 34810.0, 9: 37380.0, 10: 5820.0, 11: 7170.0,
    12: 43470.0, 13: 47830.0, 14: 52410.0, 15: 124340.0, 16: 190380.0,
    17: 176310.0, 18: None, 19: 820.0, 20: 16180.0, 21: 23760.0, 22: 45720.0,
    23: 317920.0, 24: 21610.0, 25: 7960.0, 26: 22260.0, 27: 142330.0,
    28: None, 29: 170.0, 30: 56100.0, 31: 102470.0, 32: None, 33: None,
    34: None, 35: 181000.0, 36: 19450.0, 37: 17530.0, 38: 17940.0,
    39: 35340.0, 40: 5630.0, 41: 4420.0, 42: 4720.0,
}
# ─────────────────────────────────────────────────────────────────────────────


def avg_dicts(d1: dict[int, float | None],
              d2: dict[int, float | None]) -> dict[int, float | None]:
    keys = sorted(set(d1) | set(d2))
    result: dict[int, float | None] = {}
    for k in keys:
        vals = [d for d in [d1.get(k), d2.get(k)] if d is not None]
        result[k] = sum(vals) / len(vals) if vals else None
    return result


def make_chart(data: dict[int, float | None], title: str, out_path: str) -> None:
    keys = [k for k in sorted(data) if data[k] is not None]
    labels = [f"Q{k:02d}" for k in keys]
    values = [data[k] for k in keys]  # type: ignore[misc]
    avg = sum(values) / len(values) if values else 0.0
    x = np.arange(len(labels))

    fig, ax = plt.subplots(figsize=(18, 10))
    ax.bar(x, values, color="#2c3e50", edgecolor="white", linewidth=0.5,
           label=f"Column-Engine (avg {avg:.1f} ms)")

    ax.set_yscale("log")
    ax.set_xticks(x)
    ax.set_xticklabels(labels, rotation=45, ha="right", fontsize=9)
    ax.set_ylabel("Время (мс)")
    ax.set_title(title)
    ax.legend()
    ax.yaxis.grid(True, which="both", linestyle="--", alpha=0.5)
    ax.set_axisbelow(True)

    fig.tight_layout()
    fig.savefig(out_path, dpi=150)
    print(f"Saved: {out_path}")


OUT_DIR = os.path.join(os.path.dirname(__file__), "visualization")
os.makedirs(OUT_DIR, exist_ok=True)

make_chart(
    CE_COLD,
    f"Column-Engine — время выполнения запросов в режиме холодного кэша на датасете {DATASET}.csv с размером батча {BATCH_ROWS_NUMBER}, мс",
    os.path.join(OUT_DIR, f"queries_cold_ms_log_{DATASET}_{BATCH_ROWS_NUMBER}.png"),
)

CE_HOT_AVG = avg_dicts(CE_HOT1, CE_HOT2)

make_chart(
    CE_HOT_AVG,
    f"Column-Engine — время выполнения запросов в режиме горячего кэша на датасете {DATASET}.csv с размером батча {BATCH_ROWS_NUMBER}, мс",
    os.path.join(OUT_DIR, f"queries_hot_ms_log_{DATASET}_{BATCH_ROWS_NUMBER}.png"),
)
