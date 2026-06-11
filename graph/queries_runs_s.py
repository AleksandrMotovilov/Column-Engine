#!/usr/bin/env python3

import os

import matplotlib.pyplot as plt
import numpy as np

DATASET = "hits_sample"
BATCH_ROWS_NUMBER = 100000

# ─────────────────────────────────────────────────────────────────────────────
CE_COLD: dict[int, float] = {
    0: 12.0, 1: 8.0, 2: 19.0, 3: 18.0, 4: 76.0, 5: 95.0, 6: 14.0, 7: 29.0,
    8: 516.0, 9: 561.0, 10: 143.0, 11: 168.0, 12: 427.0, 13: 509.0, 14: 515.0,
    15: 834.0, 16: 1607.0, 17: 1516.0, 18: 5803.0, 19: 6.0, 20: 261.0,
    21: 390.0, 22: 352.0, 23: 331.0, 24: 240.0, 25: 191.0, 26: 298.0,
    27: 1295.0, 28: 6866.0, 29: 348.0, 30: 653.0, 31: 2106.0, 32: 8353.0,
    33: 2211.0, 34: 2990.0, 35: 1857.0, 36: 62.0, 37: 57.0, 38: 74.0,
    39: 4763.0, 40: 76.0, 41: 104.0, 42: 20.0,
}

CE_HOT1: dict[int, float] = {
    0: 12.0, 1: 8.0, 2: 19.0, 3: 18.0, 4: 76.0, 5: 95.0, 6: 14.0, 7: 29.0,
    8: 516.0, 9: 561.0, 10: 143.0, 11: 168.0, 12: 427.0, 13: 509.0, 14: 515.0,
    15: 834.0, 16: 1607.0, 17: 1516.0, 18: 5803.0, 19: 6.0, 20: 261.0,
    21: 390.0, 22: 352.0, 23: 331.0, 24: 240.0, 25: 191.0, 26: 298.0,
    27: 1295.0, 28: 6866.0, 29: 348.0, 30: 653.0, 31: 2106.0, 32: 8353.0,
    33: 2211.0, 34: 2990.0, 35: 1857.0, 36: 62.0, 37: 57.0, 38: 74.0,
    39: 4763.0, 40: 76.0, 41: 104.0, 42: 20.0,
}

CE_HOT2: dict[int, float] = {
    0: 12.0, 1: 8.0, 2: 19.0, 3: 18.0, 4: 76.0, 5: 95.0, 6: 14.0, 7: 29.0,
    8: 516.0, 9: 561.0, 10: 143.0, 11: 168.0, 12: 427.0, 13: 509.0, 14: 515.0,
    15: 834.0, 16: 1607.0, 17: 1516.0, 18: 5803.0, 19: 6.0, 20: 261.0,
    21: 390.0, 22: 352.0, 23: 331.0, 24: 240.0, 25: 191.0, 26: 298.0,
    27: 1295.0, 28: 6866.0, 29: 348.0, 30: 653.0, 31: 2106.0, 32: 8353.0,
    33: 2211.0, 34: 2990.0, 35: 1857.0, 36: 62.0, 37: 57.0, 38: 74.0,
    39: 4763.0, 40: 76.0, 41: 104.0, 42: 20.0,
}
# ─────────────────────────────────────────────────────────────────────────────


def avg_dicts(d1: dict[int, float], d2: dict[int, float]) -> dict[int, float]:
    keys = sorted(set(d1) | set(d2))
    result = {}
    for k in keys:
        vals = [d for d in [d1.get(k), d2.get(k)] if d is not None]
        result[k] = sum(vals) / len(vals)
    return result


def make_chart(data: dict[int, float], title: str, out_path: str) -> None:
    labels = [f"Q{k:02d}" for k in sorted(data)]
    values = [data[k] / 1000 for k in sorted(data)]
    avg = sum(values) / len(values) if values else 0.0
    x = np.arange(len(labels))

    fig, ax = plt.subplots(figsize=(18, 10))
    ax.bar(x, values, color="steelblue", edgecolor="white", linewidth=0.5,
           label=f"Column-Engine (avg {avg:.3f} s)")

    ax.set_xticks(x)
    ax.set_xticklabels(labels, rotation=45, ha="right", fontsize=9)
    ax.set_ylabel("Время (с)")
    ax.set_title(title)
    ax.legend()
    ax.yaxis.grid(True, linestyle="--", alpha=0.5)
    ax.set_axisbelow(True)

    fig.tight_layout()
    fig.savefig(out_path, dpi=150)
    print(f"Saved: {out_path}")


OUT_DIR = os.path.join(os.path.dirname(__file__), "visualization")
os.makedirs(OUT_DIR, exist_ok=True)

make_chart(
    CE_COLD,
    f"Column-Engine — время выполнения запросов в режиме холодного кэша на датасете {DATASET}.csv с размером батча {BATCH_ROWS_NUMBER}, с",
    os.path.join(OUT_DIR, f"queries_cold_s_{DATASET}_{BATCH_ROWS_NUMBER}.png"),
)

CE_HOT_AVG = avg_dicts(CE_HOT1, CE_HOT2)

make_chart(
    CE_HOT_AVG,
    f"Column-Engine — время выполнения запросов в режиме горячего кэша на датасете {DATASET}.csv с размером батча {BATCH_ROWS_NUMBER}, с",
    os.path.join(OUT_DIR, f"queries_hot_s_{DATASET}_{BATCH_ROWS_NUMBER}.png"),
)
