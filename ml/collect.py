"""Record labelled gestures from the wand over the Nucleo's ST-LINK USB serial.

Needs the data-collection mockup firmware (local branch ml-collect-fw), which prints one line
per sample at 100 Hz:

    ax,ay,az,gx,gy,gz,btn     raw MPU-6050 counts; btn is 1 while the button is held

That line format is private to the mockup and this script. Gestures are asked in shuffled order.
Each kept recording is saved at once as ml/data/<gesture>/<person>_<YYYYMMDD-HHMMSS-mmm>.csv:
up to 5 lead-in rows (btn 0) from just before the press, then the gesture rows (btn 1).
Ctrl+C stops without losing anything already saved.

Setup, once:   python -m venv .venv
               .venv\\Scripts\\activate
               pip install -r ml/requirements.txt
Examples:      python ml/collect.py --person toa --gestures up down left right --reps 40
               python ml/collect.py --person toa --gestures _junk --reps 20
"""
import argparse
import collections
import csv
import datetime
import random
import re
import sys
from pathlib import Path

RATE_HZ = 100
LEAD_IN = 5
MIN_SAMPLES, MAX_SAMPLES = 30, 200  # 0.3-2.0 s; 200 is also the recording buffer in gesture.c
STUCK_SAMPLES = 10 * RATE_HZ
HEADER = ["ax", "ay", "az", "gx", "gy", "gz", "btn"]
DATA = Path(__file__).parent / "data"


def parse(line):
    """One serial line as 7 ints, or None for anything else (the WHO_AM_I banner, a garbled line)."""
    try:
        row = [int(v) for v in line.strip().split(",")]
    except ValueError:
        return None
    return row if len(row) == 7 and row[6] in (0, 1) else None


def recordings(lines):
    """Split the sample stream into recordings: each is its lead-in rows, then its gesture rows."""
    lead_in = collections.deque(maxlen=LEAD_IN)
    rows = None
    for line in lines:
        row = parse(line)
        if row is None:
            continue
        if row[6]:
            if rows is None:
                rows = list(lead_in)
            rows.append(row)
            if len(rows) > LEAD_IN + STUCK_SAMPLES:
                sys.exit("btn has read 1 for 10 s. If the button isn't held, flip BTN_PRESSED "
                         "in the mockup firmware's main.c.")
        elif rows is not None:
            yield rows
            rows = None
            lead_in.clear()  # the caller may have paused; the lead-in must come from just before the next press
        else:
            lead_in.append(row)


def save(rows, gesture, person, data_dir=DATA):
    folder = data_dir / gesture
    folder.mkdir(parents=True, exist_ok=True)
    now = datetime.datetime.now()
    path = folder / f"{person}_{now:%Y%m%d-%H%M%S}-{now.microsecond // 1000:03d}.csv"
    with open(path, "x", newline="") as f:  # "x": never overwrite a saved recording
        writer = csv.writer(f)
        writer.writerow(HEADER)
        writer.writerows(rows)
    return path


def serial_lines(ser):
    while True:
        raw = ser.readline()
        if not raw:
            sys.exit("No data from the board for 2 s. Is the mockup firmware (branch ml-collect-fw) "
                     "flashed, and is no other program using the port?")
        yield raw.decode("ascii", "replace")


def find_port():
    from serial.tools import list_ports
    ports = [p.device for p in list_ports.comports() if p.vid == 0x0483]  # 0x0483 = STMicroelectronics
    if len(ports) != 1:
        sys.exit(f"Found {len(ports)} ST-LINK serial ports {ports}. Pass one with --port.")
    return ports[0]


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--person", required=True, help="who performs the gestures, e.g. toa")
    ap.add_argument("--gestures", nargs="+", required=True,
                    help="gesture names, which are also the folder names, e.g. up down left right")
    ap.add_argument("--reps", type=int, default=40, help="recordings per gesture (default 40)")
    ap.add_argument("--port", help="serial port, e.g. COM5 (default: the ST-LINK's port)")
    args = ap.parse_args()
    # Names become file and folder names
    if not re.fullmatch(r"[a-z0-9]+", args.person):
        ap.error("--person: use lowercase letters and digits only")
    for g in args.gestures:
        if not re.fullmatch(r"_?[a-z0-9_]+(/[a-z0-9_]+)?", g):
            ap.error(f"--gestures: {g!r} is not a valid name, e.g. up, circle_cw, _junk, _twisted/up")

    import serial  # imported here so the tests run without pyserial

    queue = [g for g in args.gestures for _ in range(args.reps)]
    random.shuffle(queue)
    total = len(queue)
    with serial.Serial(args.port or find_port(), 115200, timeout=2) as ser:
        ser.reset_input_buffer()
        takes = recordings(serial_lines(ser))
        while queue:
            gesture = queue[-1]  # stays at the end until kept, so a rejected one is asked again
            print(f"\n[{total - len(queue) + 1}/{total}]  next: {gesture.upper()}")
            print("  press and hold the button, do the gesture, release")
            rows = next(takes)
            n = sum(row[6] for row in rows)
            if not MIN_SAMPLES <= n <= MAX_SAMPLES:
                print(f"  {n / RATE_HZ:.2f} s is outside {MIN_SAMPLES / RATE_HZ}-{MAX_SAMPLES / RATE_HZ} s, try again")
                continue
            if input(f"  {n / RATE_HZ:.2f} s   Enter = keep, d = discard: ").strip().lower() != "d":
                print(f"  saved {save(rows, gesture, args.person)}")
                queue.pop()
            ser.reset_input_buffer()  # drop what arrived while waiting for the answer
    print(f"\nDone: {total} recordings saved.")


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nStopped. Every kept recording is already saved.")
