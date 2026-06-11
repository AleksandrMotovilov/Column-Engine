#!/usr/bin/env python3

import os

import matplotlib.pyplot as plt
import numpy as np

DICT = {
    "no compression" : 832402688,
    #"RLE" : 728958919,
    #"RLE+BitPacking" : 632117991,
    #"Delta" : 832402688,
    #"Delta+BitPacking" : 795729226,
    #"Delta+RLE" : 782759531,
    #"Delta+RLE+BitPacking" : 673536923,
    #"Dict" : 473023439,
    #"Dict+BitPacking" : 261129331,
    "RLE+Dict" : 369579670,
    "RLE+Dict+BitPacking" : 181428590,
    #"Delta+Dict" : 473023439,
    #"Delta+Dict+BitPacking" : 345039825,
    "Delta+RLE+Dict" : 423380282,
    "Delta+RLE+Dict+BitPacking" : 222847522,
    "LZ4" : 119782060,
    "RLE+Dict+LZ4" : 103721003,
    "Delta+RLE+Dict+LZ4" : 107317283,
    "RLE+Dict+BitPacking+LZ4" : 93918650,
    "Delta+RLE+Dict+BitPacking+LZ4" : 102326880,
    #"adaptive compression" : 0,
}

LABELS = list(DICT.keys())
SIZES = [v / (1024 * 1024) for v in DICT.values()]

UNIT = "MB"

DATASET = "hits_sample"
BATCH_ROWS_NUMBER = 100000

OUT_DIR = os.path.join(os.path.dirname(__file__), "visualization")
os.makedirs(OUT_DIR, exist_ok=True)
OUT_PATH = os.path.join(OUT_DIR, f"file_sizes_{DATASET}_{BATCH_ROWS_NUMBER}.png")

x = np.arange(len(LABELS))
colors = plt.cm.tab10(np.linspace(0, 0.9, len(LABELS)))

fig, ax = plt.subplots(figsize=(max(8, len(LABELS) * 2), 6))

for i, (label, size, color) in enumerate(zip(LABELS, SIZES, colors)):
    ax.bar(x[i], size, color=color, edgecolor="white", linewidth=0.5,
           label=f"{label}: {size:.1f} {UNIT}")

ax.set_xticks(x)
ax.set_xticklabels(LABELS, rotation=25, ha="right", fontsize=10)
ax.set_ylabel(f"Размер файла ({UNIT})")
ax.set_title(f"Размер clmn-файла по конфигурации сжатия на датасете {DATASET}.csv с размером батча {BATCH_ROWS_NUMBER}, {UNIT}")
ax.yaxis.grid(True, linestyle="--", alpha=0.5)
ax.set_axisbelow(True)

max_val = max(SIZES)
for xi, size in zip(x, SIZES):
    ax.text(xi, size + max_val * 0.01, f"{size:.1f}", ha="center", va="bottom", fontsize=10)

fig.tight_layout()
fig.savefig(OUT_PATH, dpi=150)
print(f"Saved: {OUT_PATH}")
