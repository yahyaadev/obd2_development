#!/usr/bin/env python3
import argparse, csv, json, os, sys, time
from glob import glob

try:
    import serial  # pyserial
except ImportError:
    print("pyserial not installed. Activate venv and: pip install -r requirements.txt", file=sys.stderr)
    sys.exit(1)

def autodetect_port():
    cands = sorted(glob("/dev/ttyACM*") + glob("/dev/ttyUSB*"))
    if not cands:
        raise RuntimeError("No serial ports found (/dev/ttyACM* or /dev/ttyUSB*).")
    return cands[0]

def open_serial(port, baud):
    # IMPORTANT: set a timeout; otherwise readline() can block forever if no newline arrives
    return serial.Serial(port=port, baudrate=baud, timeout=1)  # seconds

def ensure_parent(path):
    d = os.path.dirname(path)
    if d: os.makedirs(d, exist_ok=True)

def infer_fieldnames(sample):
    common = ["seq","tx_ms","rpm","speed_kph","coolant_c","batt_v"]
    fields = [f for f in common if f in sample]
    for k in sorted(sample.keys()):
        if k not in fields: fields.append(k)
    return ["rx_iso","rx_ms"] + fields

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", default="auto")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--out", default="logs/telemetry.csv")
    ap.add_argument("--print", action="store_true")
    args = ap.parse_args()

    port = autodetect_port() if args.port == "auto" else args.port
    ensure_parent(args.out)

    start = time.monotonic()
    writer = None; csvfile = None; fieldnames = None
    rows_written = 0; bad_lines = 0

    print(f"[logger] {port} @ {args.baud}", file=sys.stderr)
    try:
        with open_serial(port, args.baud) as ser:
            while True:
                raw = ser.readline()
                if not raw: 
                    continue
                line = raw.decode("utf-8", errors="ignore").strip()
                if not line:
                    continue
                if args.print:
                    print(line)

                try:
                    obj = json.loads(line)
                    if not isinstance(obj, dict):
                        raise ValueError("JSON is not an object")
                except Exception:
                    bad_lines += 1
                    continue

                if writer is None:
                    fieldnames = infer_fieldnames(obj)
                    csv_exists = os.path.exists(args.out)
                    csvfile = open(args.out, "a", newline="", encoding="utf-8")
                    writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
                    if not csv_exists: writer.writeheader()

                row = {"rx_iso": time.strftime("%Y-%m-%dT%H:%M:%S", time.localtime()),
                       "rx_ms": int((time.monotonic() - start) * 1000)}
                for k in fieldnames:
                    if k in ("rx_iso","rx_ms"): continue
                    if k in obj: row[k] = obj[k]

                writer.writerow(row)
                rows_written += 1
                if rows_written % 50 == 0 and csvfile is not None:
                    csvfile.flush()
    except KeyboardInterrupt:
        pass
    finally:
        if csvfile: csvfile.close()
        print(f"[logger] rows={rows_written} bad={bad_lines}", file=sys.stderr)

if __name__ == "__main__":
    main()
