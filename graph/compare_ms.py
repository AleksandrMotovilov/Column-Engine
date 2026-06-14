#!/usr/bin/env python3

import os

import matplotlib.pyplot as plt
import numpy as np

DATASET = "hits"
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


def avg_dicts(d1: dict[int, float | None],
              d2: dict[int, float | None]) -> dict[int, float | None]:
    keys = sorted(set(d1) | set(d2))
    result: dict[int, float | None] = {}
    for k in keys:
        vals = [d for d in [d1.get(k), d2.get(k)] if d is not None]
        result[k] = sum(vals) / len(vals) if vals else None
    return result


def make_chart(ce: dict[int, float | None], duck: dict[int, float | None],
               title: str, out_path: str) -> None:
    all_keys = sorted(
        k for k in (set(ce) | set(duck))
        if ce.get(k) is not None and duck.get(k) is not None
    )
    labels = [f"Q{k:02d}" for k in all_keys]
    vals_ce   = [ce[k]   for k in all_keys]  # type: ignore[index]
    vals_duck = [duck[k] for k in all_keys]  # type: ignore[index]

    avg_ce   = sum(vals_ce)   / len(vals_ce)   if vals_ce   else 0.0
    avg_duck = sum(vals_duck) / len(vals_duck) if vals_duck else 0.0

    x = np.arange(len(labels))
    width = 0.4

    fig, ax = plt.subplots(figsize=(20, 10))
    ax.bar(x - width / 2, vals_ce,   width,
           label=f"Column-Engine (avg {avg_ce:.1f} ms)",
           color="#2c3e50",  edgecolor="white", linewidth=0.4)
    ax.bar(x + width / 2, vals_duck, width,
           label=f"DuckDB (avg {avg_duck:.1f} ms)",
           color="#b87333", edgecolor="white", linewidth=0.4)

    ax.set_xticks(x)
    ax.set_xticklabels(labels, rotation=45, ha="right", fontsize=9)
    ax.set_ylabel("Время (мс)")
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
    f"Column-Engine vs DuckDB — время выполнения запросов в режиме холодного кэша на датасете {DATASET}.csv с размером батча {BATCH_ROWS_NUMBER}, мс",
    os.path.join(OUT_DIR, f"compare_cold_ms_{DATASET}_{BATCH_ROWS_NUMBER}.png"),
)

CE_HOT_AVG   = avg_dicts(CE_HOT1,   CE_HOT2)
DUCK_HOT_AVG = avg_dicts(DUCK_HOT1, DUCK_HOT2)

make_chart(
    CE_HOT_AVG, DUCK_HOT_AVG,
    f"Column-Engine vs DuckDB — время выполнения запросов в режиме горячего кэша на датасете {DATASET}.csv с размером батча {BATCH_ROWS_NUMBER}, мс",
    os.path.join(OUT_DIR, f"compare_hot_ms_{DATASET}_{BATCH_ROWS_NUMBER}.png"),
)
