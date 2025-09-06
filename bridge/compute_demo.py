#!/usr/bin/env python3
import csv, time, statistics, os

CSV_PATH = "logs/telemetry.csv"

def moving_avg(seq, n=5):
    if len(seq) < n: return None
    return sum(seq[-n:]) / n

def compute_window(rows):
    rx   = [int(r["rx_ms"]) for r in rows if r.get("rx_ms")]
    seqs = [int(r["seq"])   for r in rows if r.get("seq")]
    rpm  = [float(r["rpm"]) for r in rows if r.get("rpm") not in (None,"")]
    spd  = [float(r["speed_kph"]) for r in rows if r.get("speed_kph") not in (None,"")]

    deltas = [b-a for a,b in zip(rx, rx[1:]) if b >= a]
    rate_hz  = 1000.0 / (sum(deltas)/len(deltas)) if deltas else float("nan")
    jitter   = statistics.pstdev(deltas) if len(deltas) > 1 else float("nan")

    drops = 0
    for a,b in zip(seqs, seqs[1:]):
        if b > a + 1: drops += (b - a - 1)

    return {"n":len(rows), "rate_hz":rate_hz, "jitter_ms":jitter,
            "drops":drops, "rpm_ma":moving_avg(rpm), "speed_ma":moving_avg(spd)}

def main():
    print("[compute] watching logs/telemetry.csv … Ctrl+C to stop")
    last_n = 0
    while True:
        if not os.path.exists(CSV_PATH):
            time.sleep(0.5); continue
        try:
            with open(CSV_PATH, newline="", encoding="utf-8") as f:
                rows = list(csv.DictReader(f))
        except Exception:
            time.sleep(0.5); continue

        if len(rows) >= 20 and len(rows) != last_n:
            window = rows[-200:]   # ~20 s @ 10 Hz
            stats = compute_window(window)
            print(f"[n={stats['n']}] rate={stats['rate_hz']:.2f} Hz  "
                  f"jitter={stats['jitter_ms']:.1f} ms  drops={stats['drops']}  "
                  f"rpm_ma={stats['rpm_ma']:.1f}  speed_ma={stats['speed_ma']:.1f}")
            last_n = len(rows)
        time.sleep(1.0)

if __name__ == "__main__":
    main()
