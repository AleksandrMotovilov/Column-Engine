#!/usr/bin/env python3

import os

import matplotlib.pyplot as plt
import numpy as np

DICT: dict[str, float] = {
    "std::unordered_map"               : (207 + 211) / 2,
    "boost::unordered_map"             : (216 + 216) / 2,
    "boost::unordered_flat_map"        : (195 + 208) / 2,
    "boost::unordered_node_map"        : (208 + 207) / 2,
    "absl::flat_hash_map"              : (211 + 202) / 2,
    "absl::node_hash_map"              : (207 + 208) / 2,
}

DATASET = "hits_sample"
BATCH_ROWS_NUMBER = 100000

OUT_DIR = os.path.join(os.path.dirname(__file__), "visualization")
os.makedirs(OUT_DIR, exist_ok=True)
OUT_PATH = os.path.join(OUT_DIR, f"map_comparison_ms_{DATASET}_{BATCH_ROWS_NUMBER}.png")

LABELS = list(DICT.keys())
VALUES = list(DICT.values())

x = np.arange(len(LABELS))
colors = plt.cm.copper(np.linspace(0, 0.9, len(LABELS)))

fig, ax = plt.subplots(figsize=(max(8, len(LABELS) * 2), 10))

for i, (label, value, color) in enumerate(zip(LABELS, VALUES, colors)):
    ax.bar(x[i], value, color=color, edgecolor="white", linewidth=0.5,
           label=f"{label}: {value:.1f} мс")

ax.set_xticks(x)
ax.set_xticklabels(LABELS, rotation=25, ha="right", fontsize=10)
ax.set_ylabel("Среднее время выполнения запроса (мс)")
ax.set_title(
    f"Column-Engine — среднее время выполнения запросов Q0–Q42 "
    f"при разных реализациях hash map на датасете {DATASET}.csv "
    f"с размером батча {BATCH_ROWS_NUMBER}, мс"
)
ax.yaxis.grid(True, linestyle="--", alpha=0.5)
ax.set_axisbelow(True)

baseline = VALUES[0]
max_val = max(VALUES)
for xi, value in zip(x, VALUES):
    pct = round(value / baseline * 100) if baseline > 0 else 0
    ax.text(xi, value + max_val * 0.01, f"{value:.1f}", ha="center", va="bottom", fontsize=10)
    if value > max_val * 0.05:
        ax.text(xi, value / 2, f"{pct}%", ha="center", va="center",
                fontsize=10, color="white", fontweight="bold")

fig.tight_layout()
fig.savefig(OUT_PATH, dpi=150)
print(f"Saved: {OUT_PATH}")
