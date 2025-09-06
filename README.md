# obd2_development — Raspberry Pi + STM32 (OBD‑Lite Bench MVP)

This repo builds a **bench‑only, read‑only OBD‑lite MVP**: an STM32 device emits telemetry as **newline‑terminated JSON**; a Raspberry Pi reads it and (later) forwards it to a phone app over **BLE**. Eventually, the JSON values will come from simulated CAN on a two‑node bench bus. The goal is to keep everything **simple, reproducible, and Pi‑first**.

> **Status / Scope**
> - Firmware lives in `firmware/` (PlatformIO project for **NUCLEO‑F446RE**).
> - VS Code tasks in `.vscode/tasks.json` give one‑click **Build**, **Upload**, **Monitor**, plus a **Serial logger** task.
> - A tiny bridge script logs the STM32 JSON to CSV on the Pi (`bridge/serial_logger.py`).
> - CI builds firmware on every push touching `firmware/**`.
> - Future: BLE bridge / mobile app, bench CAN feeding the same JSON fields.

---

## 0) Clone & Open

```bash
git clone https://github.com/ka5j/obd2_development
cd obd2_development
code obd2_development.code-workspace
```

If you don’t use VS Code, you can still build from CLI (see below).

---

## 1) Repo layout

```
obd2_development/
├─ README.md
├─ LOG.md
├─ obd2_development.code-workspace         # VS Code workspace (root + firmware/)
├─ .vscode/
│  └─ tasks.json                           # Root tasks (run PIO in firmware/)
├─ firmware/                               # PlatformIO project (STM32F446RE)
│  ├─ platformio.ini
│  └─ src/
│     ├─ main.c
│     └─ retarget.c                        # printf() → USART2 (VCP)
├─ bridge/                                 # Pi tooling
│  ├─ serial_logger.py
│  └─ requirements.txt
├─ scripts/
│  └─ bootstrap_pi.sh                      # One‑time setup on Raspberry Pi
├─ .github/workflows/
│  └─ firmware-ci.yml                      # CI: build firmware on push/PR
└─ .editorconfig / .clang-format / .pre-commit-config.yaml
```

---

## 2) Requirements

**Hardware**

- Raspberry Pi (3/4/5) with Internet access
- STM32F446RE dev board (e.g., **NUCLEO‑F446RE**) with onboard ST‑LINK
- USB‑A ↔ Micro‑B (or appropriate) cable

**Software**

- VS Code on the Pi (recommended)
- VS Code extensions: **PlatformIO IDE**, **C/C++**
- PlatformIO Core (installed by the extension automatically)
- Python 3.x (for the bridge/venv)
- Git

---

## 3) Hardware notes (VCP & pins)

- **UART/VCP**: `USART2` on **PA2/PA3** is, by default, wired to the **ST‑LINK Virtual COM Port**. That’s how `printf()` reaches your `/dev/ttyACM*` on the Pi.
- **Default solder bridges for VCP**: **SB13 = ON**, **SB14 = ON**, **SB62 = OFF**, **SB63 = OFF** (target USART2 ↔ ST‑LINK). If you want PA2/PA3 on the Arduino headers instead, flip those (SB62/SB63 ON, SB13/SB14 OFF).
- **Baud**: 115200 8‑N‑1 (repo defaults).

_Reference: ST UM1724 (STM32 Nucleo‑64, MB1136)._

---

## 4) One‑time bootstrap on the Pi (udev + venv)

From repo root, run:

```bash
./scripts/bootstrap_pi.sh
# Re‑plug the NUCLEO.
# If you were added to the "dialout" group, log out/in or reboot.
```

Quick checks:

```bash
pio device list
ls /dev/ttyACM*
```

This script:
- Creates a **virtualenv** at `.venv/` and installs `bridge/requirements.txt` (pyserial).
- Installs **PlatformIO udev rules** and reloads udev.
- Adds the current user to the **dialout** group for serial access.

---

## 5) Open the workspace & tasks (VS Code)

Open `obd2_development.code-workspace` (repo root). Install the recommended extensions when prompted.

Use `Terminal → Run Task…` to run:

- **PIO: Build (firmware)** — build in `firmware/`
- **PIO: Upload (firmware)** — flash via ST‑LINK
- **PIO: Monitor (firmware)** — open serial monitor at **115200 baud**
- **Bridge: Serial logger** — run the Pi logger against `/dev/ttyACM*` and write CSV into `logs/`

The tasks always run inside `firmware/` so paths are consistent.

---

## 6) Build / Upload / Monitor (CLI)

```bash
# Build
cd firmware
pio run

# Upload (flash)
pio run -t upload

# Monitor (115200 baud)
pio device monitor --baud 115200
```

Artifacts:
- `firmware/.pio/build/nucleo_f446re/firmware.elf`

---

## 7) Firmware notes (printf, float, JSON)

**Retarget `printf()` to USART2 (VCP)**

`firmware/src/retarget.c`:

```c
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <sys/unistd.h> // write
extern UART_HandleTypeDef huart2;
int _write(int file, char *ptr, int len) {
  (void)file;
  HAL_UART_Transmit(&huart2, (uint8_t*)ptr, (uint16_t)len, HAL_MAX_DELAY);
  return len;
}
```

**Float formatting with newlib‑nano**

To ensure `printf("%.1f", some_float)` actually prints, `platformio.ini` adds the linker symbol for float IO:

```ini
build_flags =
  -Wl,--undefined,_printf_float
  -Wl,--gc-sections
```

**Example JSON (10 Hz) in `main.c`**

```c
static uint32_t seq = 0;
uint32_t t = HAL_GetTick();
int rpm = 1650;
float speed_kph = 42.1f;
int coolant_c = 78;
float batt_v = 12.1f;

printf("{\"seq\":%lu,\"tx_ms\":%lu,\"rpm\":%d,\"speed_kph\":%.1f,\"coolant_c\":%d,\"batt_v\":%.1f}\r\n",
       (unsigned long)seq++,
       (unsigned long)t,
       rpm, speed_kph, coolant_c, batt_v);

HAL_Delay(100); // ~10 Hz
```

> Tip: Make sure `USART2` is initialized in CubeMX (or your HAL init code) and the default SB settings for VCP are intact.

**PlatformIO on ARM64 (Pi) — toolchain pin**

If needed on aarch64 Pis, `platformio.ini` pins a known‑good GCC toolchain:

```ini
platform_packages = platformio/toolchain-gccarmnoneeabi@=1.100301.220327
```

---

## 8) Bridge (Pi) — CSV logger

**Run from VS Code tasks** (“Bridge: Serial logger”) or directly:**

```bash
# ensure venv is active if running manually
source .venv/bin/activate
python bridge/serial_logger.py --baud 115200 --outdir logs
# optional: --port /dev/ttyACM0
```

Behavior:
- Auto‑detects `/dev/ttyACM*` / `/dev/ttyUSB*` (can override with `--port`).
- Writes `logs/telemetry_YYYYmmdd_HHMMSS.csv` with a header derived from the first JSON object.
- Skips malformed lines but prints them to stderr for debugging.

---

## 9) CI (GitHub Actions)

Every push/PR that touches `firmware/**` triggers a build in CI (`.github/workflows/firmware-ci.yml`). This protects against config drift and keeps team builds reproducible.

---

## 10) Roadmap (bench‑only MVP)

1. UART JSON → Pi CSV logger (latency, packet loss)
2. Pi BLE bridge (BlueZ GATT server; fixed service/characteristics; 5–10 Hz notify)
3. Trip logic (start/stop thresholds) + CSV export parity
4. Alerts (unplug/tow) — event JSON
5. CAN bench (Pi + CANable ↔ SN65HVD230 ↔ STM32) — fill the same JSON fields

**Targets** (later): median latency < 200 ms; packet loss < 1%; trip start/stop > 95%; alerts < 1 s.

---

## 11) Development standards

- **C/C++ formatting**: `.clang-format` (LLVM‑style, width=100, 2‑space indents)
- **Editor defaults**: `.editorconfig` (LF, UTF‑8, final newline)
- **Python**: `pre-commit` runs **black**, **ruff**, and **isort**

Install hooks locally:

```bash
pip install pre-commit
pre-commit install
```

---

## 12) Troubleshooting (fast)

- **No serial device / permissions**
  - Re‑run `./scripts/bootstrap_pi.sh`, re‑plug the board, and confirm `dialout` group.
  - `pio device list`, `ls /dev/ttyACM*`

- **Serial shows nothing**
  - Confirm **baud 115200**, VCP is enabled by default (see SBs), and your `main.c` prints **newline‑terminated** JSON.
  - Check the right port: the NUCLEO usually appears as `/dev/ttyACM0`.

- **Floats print `0.0` or garbage**
  - Ensure `-Wl,--undefined,_printf_float` is present in `platformio.ini` (see §7).

- **VS Code squiggles on HAL headers**
  - Open the workspace file, then run “**PlatformIO: Rebuild IntelliSense Index**”.

- **Upload errors (Linux)**
  - Make sure **udev rules** are installed and current; re‑plug after installing.

---

## 13) Useful commands

```bash
pio device list
pio device monitor --baud 115200
pio run                 # build
pio run -t upload       # flash
```

---

## 14) Credits / Board

- Target board: **NUCLEO‑F446RE** (STM32F446RET6, 180 MHz, 512 KB Flash, 128 KB RAM).
- Tooling: **VS Code + PlatformIO** (PIO Core CLI under the hood).

---

## 15) License

MIT — see `LICENSE`.

---

### References

- ST UM1724 (STM32 Nucleo‑64, MB1136) — USART2 ↔ ST‑LINK VCP default SBs; VCP mapping.  
  https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf
- PlatformIO udev rules (Linux).  
  https://docs.platformio.org/en/stable/core/installation/udev-rules.html
- newlib‑nano & float printf background.  
  https://mcuoneclipse.com/2023/01/28/which-embedded-gcc-standard-library-newlib-newlib-nano/
