#!/usr/bin/env python3

import os

import matplotlib.pyplot as plt
import numpy as np

DICT = {
    "csv" : 75.6,
    "duckdb" : 22.085, 
    "clmn" : 10.963,
}

LABELS = list(DICT.keys())
SIZES = [v for v in DICT.values()]

UNIT = "ГБ"

DATASET = "hits"

OUT_DIR = os.path.join(os.path.dirname(__file__), "visualization")
os.makedirs(OUT_DIR, exist_ok=True)
OUT_PATH = os.path.join(OUT_DIR, f"hits_sizes_{DATASET}.png")

x = np.arange(len(LABELS))
colors = plt.cm.copper(np.linspace(0, 0.9, len(LABELS)))

fig, ax = plt.subplots(figsize=(max(8, len(LABELS) * 2), 10))

for i, (label, size, color) in enumerate(zip(LABELS, SIZES, colors)):
    ax.bar(x[i], size, color=color, edgecolor="white", linewidth=0.5,
           label=f"{label}: {size:.1f} {UNIT}")

ax.set_xticks(x)
ax.set_xticklabels(LABELS, rotation=25, ha="right", fontsize=10)
ax.set_ylabel(f"Размер файла ({UNIT})")
ax.set_title(f"Размер датасета {DATASET} в разных форматах, {UNIT}")
ax.yaxis.grid(True, linestyle="--", alpha=0.5)
ax.set_axisbelow(True)

original_size = SIZES[0]
max_val = max(SIZES)
for xi, size in zip(x, SIZES):
    pct = round(size / original_size * 100)
    ax.text(xi, size + max_val * 0.01, f"{size:.1f}", ha="center", va="bottom", fontsize=10)
    ax.text(xi, size / 2, f"{pct}%", ha="center", va="center", fontsize=10, color="white", fontweight="bold")

fig.tight_layout()
fig.savefig(OUT_PATH, dpi=150)
print(f"Saved: {OUT_PATH}")
