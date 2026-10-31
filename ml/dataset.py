"""Load the recordings in ml/data/ as raw arrays, one per file.

Each gesture folder (up/, circle_cw/, ...) is a class. Folders starting with "_" (_junk/,
_twisted/<gesture>/) hold recordings for evaluation only and never become classes.
Scaling, padding and gravity correction are left to train.py, because gesture.c has to repeat
them exactly.

    python ml/dataset.py      checks every file and counts recordings per gesture and person
"""
from collections import Counter
from pathlib import Path
from typing import NamedTuple

import numpy as np

DATA = Path(__file__).parent / "data"
HEADER = "ax,ay,az,gx,gy,gz,btn"


class Recording(NamedTuple):
    gesture: str         # folder under data/: "up", "_junk" or "_twisted/up"
    person: str
    lead_in: np.ndarray  # (k, 6) int16 raw counts, k <= 5, from just before the press
    samples: np.ndarray  # (n, 6) int16 raw counts, from press to release
    path: Path


def read(path, data_dir=DATA):
    with open(path) as f:
        if f.readline().strip() != HEADER:
            raise ValueError(f"{path}: header is not {HEADER}")
    rows = np.loadtxt(path, delimiter=",", skiprows=1, dtype=np.int16, ndmin=2)
    btn = rows[:, -1]
    # Lead-in rows (btn 0) first, then at least one gesture row (btn 1)
    if rows.shape[1] != 7 or not len(btn) or not set(btn) <= {0, 1} or not btn[-1] or (np.diff(btn) < 0).any():
        raise ValueError(f"{path}: expected 7 columns and btn rows 0...0 then 1...1")
    k = int((btn == 0).sum())
    gesture = path.parent.relative_to(data_dir).as_posix()
    if gesture == ".":
        raise ValueError(f"{path}: recordings belong in a gesture folder")
    return Recording(gesture, path.stem.split("_")[0], rows[:k, :6], rows[k:, :6], path)


def load(data_dir=DATA):
    """All recordings, plus the class names: the gesture folders not starting with "_", sorted."""
    recordings = [read(p, data_dir) for p in sorted(data_dir.rglob("*.csv"))]
    classes = sorted({r.gesture for r in recordings if not r.gesture.startswith("_")})
    return classes, recordings


if __name__ == "__main__":
    classes, recordings = load()
    print("classes:", classes)
    for (gesture, person), n in sorted(Counter((r.gesture, r.person) for r in recordings).items()):
        print(f"  {gesture:<16} {person:<10} {n}")
