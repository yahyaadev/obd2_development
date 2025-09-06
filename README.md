# OBD‑Lite (Bench‑Only) — Raspberry Pi + STM32 (Team Setup & Dev Guide)

This repo builds a **bench‑only, read‑only OBD‑lite** MVP: an STM32 device emits telemetry as JSON; a Raspberry Pi reads it and (later) forwards it to a phone app over **BLE**. Eventually, the JSON values will come from **simulated CAN** on a two‑node bench bus. The goal is to keep everything **simple, reproducible, and Pi‑first**.

---

## 0) TL;DR (Quick Start)

1. **Open the workspace** in VS Code: `File → Open Workspace from File…` → `obb2_development.code-workspace` (repo root).
2. Install **PlatformIO IDE** and **C/C++** extensions if prompted.
3. Plug the **NUCLEO‑F446RE** by USB (ST‑LINK onboard).
4. **Build** -> **Upload** -> **Monitor** via `Terminal → Run Task…`:
   - **PIO: Build (firmware)**
   - **PIO: Upload (firmware)**
   - **PIO: Monitor (firmware)** (115200 baud)
5. If serial permission errors: install udev rules (see *USB access* below), add user to `dialout`, replug board.

> Current status: UART → JSON stream is the first milestone. If it’s not implemented yet, see *Enable the JSON stream*.

---

## 1) Repo layout

```
obd2_development/
├─ README.md                 # This file
├─ LOG.md                    # Short step logs (what/why/evidence/next)
├─ obb2_development.code-workspace   # VS Code multi-root workspace (root + firmware/)
├─ .vscode/
│  └─ tasks.json             # Root tasks that run PIO in firmware/
├─ firmware/                 # PlatformIO project (STM32F446RE)
│  ├─ platformio.ini
│  └─ src/
│     └─ main.c              # (JSON loop lives here)
├─ bridge/                   # Pi scripts (logger, BLE bridge later)
└─ app/mobile/               # Flutter app (later)
```

---

## 2) Requirements

**Hardware**
- Raspberry Pi (3/4/5) with Internet access
- STM32F446RE development board (e.g., **NUCLEO‑F446RE**) with onboard ST‑LINK
- USB‑A ↔ Micro‑B or appropriate cable

**Software**
- VS Code on the Pi
- VS Code extensions: **PlatformIO IDE**, **C/C++**
- PlatformIO Core (installed by the extension automatically)
- Git

---

## 3) Open the workspace and install extensions

- Open `obb2_development.code-workspace` from the repo root.
- When prompted, install recommended extensions (PlatformIO IDE, C/C++). The workspace is set up so you edit everything in one window while PlatformIO still manages the firmware project under `firmware/`.

---

## 4) USB access on Linux (udev + dialout)

Run on the Pi, then **replug** the board:

```bash
# Install PlatformIO udev rules
sudo curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules   -o /etc/udev/rules.d/99-platformio-udev.rules

# Reload and trigger
sudo udevadm control --reload-rules
sudo udevadm trigger

# Make sure your user can access serial devices
sudo usermod -aG dialout $USER
# Log out/in (or reboot) after this; then replug the board
```

Check devices:
```bash
pio device list
ls /dev/ttyACM*
```

---

## 5) Build, Upload, Monitor (root tasks)

From VS Code (this repo’s window):

- **Build:** `Terminal → Run Build Task…` → **PIO: Build (firmware)**
- **Upload:** `Terminal → Run Task…` → **PIO: Upload (firmware)**
- **Monitor:** `Terminal → Run Task…` → **PIO: Monitor (firmware)` (115200 baud)

CLI alternative:
```bash
cd obd2_development/firmware
pio run                 # Build
pio run -t upload       # Flash via ST-LINK
pio device monitor --baud 115200
```

Artifacts:
- ELF: `firmware/.pio/build/nucleo_f446re/firmware.elf`

---

## 6) Enable the JSON stream (first milestone)

**Goal:** MCU prints **one JSON line every ~100 ms** on USART2 (PA2/PA3 → ST‑LINK VCP @ 115200). Example payload:

```json
{"seq":123,"tx_ms":4567,"rpm":1650,"speed_kph":42.1,"coolant_c":78,"batt_v":12.1}
```

**Where:** `firmware/src/main.c`

**Validate:** Run **PIO: Monitor (firmware)** and confirm a clean, newline‑terminated JSON object per line at ~10 Hz.

> Once the UART JSON is stable, the Pi will log it (CSV) and later forward it over BLE with fixed UUIDs. When CAN hardware is added, the JSON fields stay the same—only the source of the numbers changes.

---

## 7) PlatformIO notes (ARM64 + reproducibility)

- If you are on a **64‑bit Pi OS (aarch64)** and see toolchain install issues or segfaults, pin a known-good GCC in `firmware/platformio.ini`:

```ini
[env:nucleo_f446re]
platform = ststm32
board = nucleo_f446re
framework = stm32cube
upload_protocol = stlink
monitor_speed = 115200
; Pin a stable toolchain on aarch64 hosts:
platform_packages = platformio/toolchain-gccarmnoneeabi@=1.100301.220327
```

- You can also pin the STM32 platform version, e.g. `platform = ststm32@<X.Y.Z>`, to keep builds consistent across machines.

---

## 8) Roadmap (bench‑only MVP)

1) **UART JSON → Pi CSV** logger (latency, packet loss)
2) **Pi BLE bridge** (BlueZ GATT server; fixed service/characteristics; 5–10 Hz notify)
3) **Trip logic** (start/stop thresholds) + CSV export parity
4) **Alerts** (unplug/tow) — event JSON
5) **CAN bench** (Pi + CANable ↔ SN65HVD230 ↔ STM32) — fill the same JSON fields

Acceptance targets (later): median latency < 200 ms; packet loss < 1%; trip start/stop > 95%; alerts < 1 s.

---

## 9) Troubleshooting (fast)

- **No device/permission** → install udev rules, add `dialout`, replug; check `pio device list`.
- **HAL include squiggles** → open the workspace file, then `PlatformIO: Rebuild IntelliSense Index` in VS Code.
- **Serial shows nothing** → confirm the port (`ls /dev/ttyACM*`), correct baud (115200), and that `main.c` prints newline‑terminated JSON.
- **Build toolchain errors on Pi** → pin `platform_packages` (see *PlatformIO notes*).

---

## 10) Useful commands

```bash
pio device list
pio device monitor --baud 115200
pio run                 # build
pio run -t upload       # flash
```

---

### Credits / Board

- Board target: **NUCLEO‑F446RE** (STM32F446RET6, 180 MHz, 512 KB Flash, 128 KB RAM).
- Tooling: VS Code + PlatformIO IDE (PIO Core CLI under the hood).
