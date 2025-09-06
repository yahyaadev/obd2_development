#!/usr/bin/env python3
import argparse
import csv
import datetime as dt
import json
import sys
from pathlib import Path

try:
    import serial
except ImportError:
    print("Install pyserial: pip install pyserial", file=sys.stderr)
    sys.exit(1)


def guess_port():
    import glob

    cands = sorted(glob.glob("/dev/ttyACM*") + glob.glob("/dev/ttyUSB*"))
    return cands[0] if cands else None


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--port", default=None, help="Serial port (default: auto-detect)")
    p.add_argument("--baud", type=int, default=115200)
    p.add_argument("--outdir", default="logs", help="Folder for CSV logs")
    args = p.parse_args()

    port = args.port or guess_port()
    if not port:
        print(
            "No serial ports found (ACM/USB). Plug board and try again.",
            file=sys.stderr,
        )
        sys.exit(2)

    Path(args.outdir).mkdir(parents=True, exist_ok=True)
    stamp = dt.datetime.now().strftime("%Y%m%d_%H%M%S")
    csv_path = Path(args.outdir) / f"telemetry_{stamp}.csv"

    with serial.Serial(port, args.baud, timeout=1) as ser, open(
        csv_path, "w", newline=""
    ) as f:
        writer = None
        print(f"Logging {port} @ {args.baud} to {csv_path}")
        while True:
            line = ser.readline()
            if not line:
                continue
            try:
                obj = json.loads(line.decode("utf-8").strip())
            except Exception:
                # pass-through bad lines to stderr for debugging
                sys.stderr.write(f"BAD:{line!r}\n")
                continue
            if writer is None:
                fields = list(obj.keys())
                writer = csv.DictWriter(f, fieldnames=fields)
                writer.writeheader()
            writer.writerow(obj)
            f.flush()


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        pass
