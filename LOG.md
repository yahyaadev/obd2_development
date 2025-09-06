# Dev log (append entires)

- **Date:** 2025-09-05
- **Log #:** 001
- **Time Spent:** ~10-15 min
- **Topics Covered:** Created project scaffold; verified Nucleo is visible (`st-info --probe`); confirmed `/dev/ttyACM*` appears
- **Evidence:**
    - `st-info --probe` -> 
                Found 1 stlink programmers
                version:    V2J46S31
                serial:     0669FF3833584B3043110731
                flash:      524288 (pagesize: 131072)
                sram:       131072
                chipid:     0x0421
                descr:      F446
    - `ls /dev/ttyACM*` -> /dev/ttyACM0
- **Next Steps:** Add the tiniest firmware that prints one JSON line at 5-10 Hz over USART2, and a simple pi reader to confirm


- **Date:** 2025-09-06
- **Log #:** 003
- **Goal:** Provide minimal `main()` so PlatformIO can link and flash.
- **Actions:** Added `firmware/src/main.c` with HAL_Init() and idle loop; built and uploaded via ST-LINK.
- **Evidence:** Build SUCCESS; Upload OK.
- **Outcome:** End-to-end Build -> Link -> Flash works from VS Code on the Pi.
- **Next:** Add a tiny JSON emitter over USART2 at 5–10 Hz, plus a Pi-side reader to confirm the pipe.
