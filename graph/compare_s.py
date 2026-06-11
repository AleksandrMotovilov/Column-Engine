#!/usr/bin/env python3

import os

import matplotlib.pyplot as plt
import numpy as np

DATASET = "hits"
BATCH_ROWS_NUMBER = 100000

# ─────────────────────────────────────────────────────────────────────────────
CE_COLD: dict[int, float] = {
    0: 12.0, 1: 8.0, 2: 19.0, 3: 18.0, 4: 76.0, 5: 95.0, 6: 14.0,
    7: 29.0, 8: 516.0, 9: 561.0, 10: 143.0, 11: 168.0, 12: 427.0, 13: 509.0,
    14: 515.0, 15: 834.0, 16: 1607.0, 17: 1516.0, 18: 5803.0, 19: 6.0,
    20: 261.0, 21: 390.0, 22: 352.0, 23: 331.0, 24: 240.0, 25: 191.0,
    26: 298.0, 27: 1295.0, 28: 6866.0, 29: 348.0, 30: 653.0, 31: 2106.0,
    32: 8353.0, 33: 2211.0, 34: 2990.0, 35: 1857.0, 36: 62.0, 37: 57.0,
    38: 74.0, 39: 4763.0, 40: 76.0, 41: 104.0, 42: 20.0,
}

CE_HOT1: dict[int, float] = {
    0: 12.0, 1: 8.0, 2: 19.0, 3: 18.0, 4: 76.0, 5: 95.0, 6: 14.0,
    7: 29.0, 8: 516.0, 9: 561.0, 10: 143.0, 11: 168.0, 12: 427.0, 13: 509.0,
    14: 515.0, 15: 834.0, 16: 1607.0, 17: 1516.0, 18: 5803.0, 19: 6.0,
    20: 261.0, 21: 390.0, 22: 352.0, 23: 331.0, 24: 240.0, 25: 191.0,
    26: 298.0, 27: 1295.0, 28: 6866.0, 29: 348.0, 30: 653.0, 31: 2106.0,
    32: 8353.0, 33: 2211.0, 34: 2990.0, 35: 1857.0, 36: 62.0, 37: 57.0,
    38: 74.0, 39: 4763.0, 40: 76.0, 41: 104.0, 42: 20.0,
}

CE_HOT2: dict[int, float] = {
    0: 12.0, 1: 8.0, 2: 19.0, 3: 18.0, 4: 76.0, 5: 95.0, 6: 14.0,
    7: 29.0, 8: 516.0, 9: 561.0, 10: 143.0, 11: 168.0, 12: 427.0, 13: 509.0,
    14: 515.0, 15: 834.0, 16: 1607.0, 17: 1516.0, 18: 5803.0, 19: 6.0,
    20: 261.0, 21: 390.0, 22: 352.0, 23: 331.0, 24: 240.0, 25: 191.0,
    26: 298.0, 27: 1295.0, 28: 6866.0, 29: 348.0, 30: 653.0, 31: 2106.0,
    32: 8353.0, 33: 2211.0, 34: 2990.0, 35: 1857.0, 36: 62.0, 37: 57.0,
    38: 74.0, 39: 4763.0, 40: 76.0, 41: 104.0, 42: 20.0,
}

DUCK_COLD: dict[int, float] = {
    0: 27.0, 1: 462.0, 2: 1544.0, 3: 1945.0, 4: 6121.0, 5: 7897.0, 6: 71.0,
    7: 480.0, 8: 7496.0, 9: 10865.0, 10: 4078.0, 11: 3887.0, 12: 6698.0,
    13: 10809.0, 14: 7600.0, 15: 6768.0, 16: 15336.0, 17: 15041.0, 18: 27908.0,
    19: 1111.0, 20: 22349.0, 21: 23594.0, 22: 44796.0, 23: 1988.0, 24: 415.0,
    25: 174.0, 26: 531.0, 27: 22872.0, 28: 109294.0, 29: 1298.0, 30: 9342.0,
    31: 13421.0, 32: 36981.0, 33: 29931.0, 34: 30019.0, 35: 6673.0, 36: 315.0,
    37: 185.0, 38: 237.0, 39: 661.0, 40: 172.0, 41: 139.0, 42: 123.0,
}

DUCK_HOT1: dict[int, float] = {
    0: 1.0, 1: 57.0, 2: 350.0, 3: 509.0, 4: 4347.0, 5: 4758.0, 6: 2.0,
    7: 81.0, 8: 4990.0, 9: 6740.0, 10: 1595.0, 11: 1727.0, 12: 4162.0,
    13: 6383.0, 14: 4368.0, 15: 4672.0, 16: 10126.0, 17: 9877.0, 18: 19336.0,
    19: 41.0, 20: 7709.0, 21: 6585.0, 22: 6831.0, 23: 630.0, 24: 135.0,
    25: 48.0, 26: 139.0, 27: 9766.0, 28: 104432.0, 29: 330.0, 30: 4049.0,
    31: 4753.0, 32: 30495.0, 33: 15463.0, 34: 16068.0, 35: 4993.0, 36: 116.0,
    37: 64.0, 38: 59.0, 39: 261.0, 40: 14.0, 41: 14.0, 42: 34.0,
}

DUCK_HOT2: dict[int, float] = {
    0: 0.0, 1: 55.0, 2: 350.0, 3: 513.0, 4: 4825.0, 5: 4657.0, 6: 2.0,
    7: 82.0, 8: 5390.0, 9: 6823.0, 10: 1600.0, 11: 1709.0, 12: 4154.0,
    13: 6339.0, 14: 4316.0, 15: 4572.0, 16: 10218.0, 17: 9652.0, 18: 19383.0,
    19: 41.0, 20: 7691.0, 21: 6525.0, 22: 6890.0, 23: 632.0, 24: 132.0,
    25: 46.0, 26: 137.0, 27: 9794.0, 28: 104061.0, 29: 319.0, 30: 3992.0,
    31: 4760.0, 32: 31603.0, 33: 15576.0, 34: 15971.0, 35: 5012.0, 36: 116.0,
    37: 65.0, 38: 58.0, 39: 253.0, 40: 14.0, 41: 16.0, 42: 32.0,
}
# ─────────────────────────────────────────────────────────────────────────────


def avg_dicts(d1: dict[int, float], d2: dict[int, float]) -> dict[int, float]:
    keys = sorted(set(d1) | set(d2))
    result = {}
    for k in keys:
        vals = [d for d in [d1.get(k), d2.get(k)] if d is not None]
        result[k] = sum(vals) / len(vals)
    return result


def make_chart(ce: dict[int, float], duck: dict[int, float],
               title: str, out_path: str) -> None:
    all_keys = sorted(set(ce) | set(duck))
    labels = [f"Q{k:02d}" for k in all_keys]
    vals_ce   = [ce.get(k, 0.0)   / 1000 for k in all_keys]
    vals_duck = [duck.get(k, 0.0) / 1000 for k in all_keys]

    avg_ce   = sum(vals_ce)   / len(vals_ce)   if vals_ce   else 0.0
    avg_duck = sum(vals_duck) / len(vals_duck) if vals_duck else 0.0

    x = np.arange(len(labels))
    width = 0.4

    fig, ax = plt.subplots(figsize=(20, 10))
    ax.bar(x - width / 2, vals_ce,   width,
           label=f"Column-Engine (avg {avg_ce:.3f} s)",
           color="steelblue",  edgecolor="white", linewidth=0.4)
    ax.bar(x + width / 2, vals_duck, width,
           label=f"DuckDB (avg {avg_duck:.3f} s)",
           color="darkorange", edgecolor="white", linewidth=0.4)

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
    CE_COLD, DUCK_COLD,
    f"Column-Engine vs DuckDB — время выполнения запросов в режиме холодного кэша на датасете {DATASET}.csv с размером батча {BATCH_ROWS_NUMBER}, с",
    os.path.join(OUT_DIR, f"compare_cold_s_{DATASET}_{BATCH_ROWS_NUMBER}.png"),
)

CE_HOT_AVG   = avg_dicts(CE_HOT1,   CE_HOT2)
DUCK_HOT_AVG = avg_dicts(DUCK_HOT1, DUCK_HOT2)

make_chart(
    CE_HOT_AVG, DUCK_HOT_AVG,
    f"Column-Engine vs DuckDB — время выполнения запросов в режиме горячего кэша на датасете {DATASET}.csv с размером батча {BATCH_ROWS_NUMBER}, с",
    os.path.join(OUT_DIR, f"compare_hot_s_{DATASET}_{BATCH_ROWS_NUMBER}.png"),
)
