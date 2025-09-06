# Dev log (append entires)

**Date:** 2025-09-05
**Log #:** 001
**Topics Covered:** Created project scaffold; verified Nucleo is visible (`st-info --probe`); confirmed `/dev/ttyACM*` appears
**Evidence:**
    `st-info --probe` -> 
                Found 1 stlink programmers
                version:    V2J46S31
                serial:     0669FF3833584B3043110731
                flash:      524288 (pagesize: 131072)
                sram:       131072
                chipid:     0x0421
                descr:      F446
    `ls /dev/ttyACM*` -> /dev/ttyACM0
**Next:** Add the tiniest firmware that prints one JSON line at 5-10 Hz over USART2, and a simple pi reader to confirm

**Date:** 2025-09-06
**Log #:** 002
**Goal:** Fix PlatformIO build segfault on Pi (ARM64).
**Change:** Pinned toolchain in `firmware/platformio.ini` to `platformio/toolchain-gccarmnoneeabi@=1.100301.220327` (GCC 10.3.1).
**Evidence:** Build completes (no Error 139); Upload via ST-LINK succeeds.
**Next:** Provide minimal `main()` so PlatformIO can link and flash.

**Date:** 2025-09-06
**Log #:** 003
**Goal:** Provide minimal `main()` so PlatformIO can link and flash.
**Actions:** Added `firmware/src/main.c` with HAL_Init() and idle loop; built and uploaded via ST-LINK.
**Evidence:** Build SUCCESS; Upload OK.
**Outcome:** End-to-end Build -> Link -> Flash works from VS Code on the Pi.
**Next:** Add a tiny JSON emitter over USART2 at 5–10 Hz, plus a Pi-side reader to confirm the pipe.

**Date:** 2025-09-06
**Log #:** 004
**Goal:** Use repo root as single VS Code workspace; fix HAL IntelliSense; run PlatformIO from root.
**Files:**
  `obb2_development.code-workspace`
  `.vscode/tasks.json`
**Results:** HAL include OK in `firmware/src/main.c`; root tasks build & upload succeed; serial monitor opens (`/dev/ttyACM*` @ 115200) but **no output yet** (JSON pipe not implemented).
**Next:** Implement UART JSON emitter in firmware, then add Pi CSV logger at `bridge/serial_logger.py`.

**Date:** 2025-09-06
**Log #:** 005
**Goal:** Finalize repo-scoped docs and workflow so the team can reproduce the STM32 build/flash/monitor flow from the repo root; prepare for UART JSON work next.
**Files:**
  `README.md` (brand-new, team-ready)
**Changes:**
  Added a clean, formatted `README.md` with TL;DR, SSH/HTTPS clone steps, USB/udev + `dialout` instructions, PlatformIO notes (aarch64 toolchain pin), troubleshooting, roadmap, and useful commands.
  Confirmed repo remains a single VS Code workspace (`obb2_development.code-workspace`) with root tasks invoking PlatformIO in `firmware/`.
**Results:** Teammates can clone, open the workspace, build, upload, and open the serial monitor from the repo root. Docs clearly state that JSON output is not implemented yet.
**Evidence:** `Terminal -> Run Task…` -> PIO tasks succeed (Build/Upload/Monitor @ 115200). `pio device list` shows `/dev/ttyACM*`. HAL IntelliSense OK in `firmware/src/main.c`.
**Next:** Implement UART JSON emitter in `firmware/src/main.c` (newline-terminated JSON @ 115200, ~10 Hz) and add Pi CSV logger at `bridge/serial_logger.py`. Then iterate toward BLE bridge and simulated CAN bench.


**Date:** 2025-09-06
**Log #:** 006
**Goal:** Get continuous UART output on /dev/ttyACM0 @115200 via ST-LINK VCP.
**Actions:** Pinned monitor_speed=115200; verified Nucleo VCP bridges (SB13/SB14 ON; SB61/SB62/SB63 OFF) per UM1724; ran minimal LED+UART test; restored JSON loop without %f (or enabled -u _printf_float).
**Results:** Serial monitor shows newline-terminated output (~10 Hz) reliably.
**Next:** Start Pi logger (CSV) and compute (rate/jitter/drops).

**Date:** 2025-09-06
**Log #:** 007
**Goal:** Make UART output visible and stable on `/dev/ttyACM0 @ 115200` via ST-LINK VCP (Nucleo-F446RE).
**Files:**
  - `firmware/platformio.ini` (pin `monitor_speed = 115200`, optional `monitor_port = /- dev/ttyACM0`)
  - `firmware/src/main.c` (explicit `MX_USART2_UART_Init()` on PA2/PA3 AF7; minimal UART loop)
  - `firmware/src/stm32f4xx_it.c` (adds `SysTick_Handler()` → `HAL_IncTick()` so `HAL_Delay()` works)
**Actions:**
  Verified the exact port: `pio device list` → `/dev/ttyACM0 (ST-Link VCP)`.
  Opened monitor explicitly: `pio device monitor -p /dev/ttyACM0 --baud 115200`.
  Added minimal transmit loop after UART init (`"BOOT\r\n"` then `"hello\r\n"` every 200 ms).
  Ensured HAL tick is alive by defining `SysTick_Handler()`; LED heartbeat confirms main loop runs.
**Results:** Serial monitor now shows continuous output (`BOOT`, then repeated `hello`). Path from USART2 (PA2/PA3) → ST-LINK VCP is confirmed working at 115200 baud.
**Evidence:** 
  - Port list: `/dev/ttyACM0` present (ST-Link VCP).
  - Monitor prints: `BOOT` then `hello` every ~200 ms.
**Next:** Restore the JSON loop (10 Hz). To avoid `%f` issues with newlib-nano, either (A) emit scaled ints (`speed_kph_x10`, `batt_v_x10`) or (B) enable float printf with `-u _printf_float`. Then run the Pi CSV logger + compute.

