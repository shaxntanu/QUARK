# QUARK

QUARK is an ESP32-C3 desk display for Codex usage and live PC telemetry. The ESP32 drives a 128×64 SH1106 OLED, a page/refresh button, a mode switch, a buzzer, and the working red and blue LED channels. The Windows companion reads local system stats and Codex App Server data, then sends them over USB serial.

## Hardware and pins

| Part | ESP32-C3 GPIO |
|---|---:|
| OLED SDA | 0 |
| OLED SCL | 1 |
| Page/refresh button | 3 |
| Status LED red | 4 |
| Buzzer | 5 |
| AI/System switch | 6 |
| Status LED blue | 10 |

The faulty green LED channel on GPIO 7 is unused. The button and mode switch use `INPUT_PULLUP` and should connect to GND when closed. Use current-limiting resistors for each working LED channel.

## Carrier PCB

The KiCad design for the ESP32-C3 SuperMini carrier is in [`hardware/QUARK_SuperMini_Carrier`](hardware/QUARK_SuperMini_Carrier/README.md). It includes the schematic, two-layer PCB, BOM, previews, and Gerber/drill files for fabrication. The carrier routes the external OLED, mode switch, page button, passive piezo, and red/blue status LEDs to the firmware pin map above.

## OLED pages

Short button presses move through the active mode's pages. Hold for about 0.8 seconds to request a refresh.

- **AI:** token activity and quota history, token counts, a five-hour allowance chart, and a weekly allowance chart.
- **System:** CPU/RAM trend overview, CPU detail, GPU load with temperature, and RAM detail.
- Charts use the OLED's 128×64 area; quota reset labels are kept beside their chart.
- The header shows the active mode, PC connection state, and page.

## Status LED

| State | LED |
|---|---|
| Connected in the active mode | Blue |
| Offline or an active-mode error | Red |
| Refresh in progress | Red + blue |

## Build and install the Windows companion

The companion detects the ESP32 USB serial port, reconnects after unplug/replug, keeps system telemetry moving while Codex data loads, and writes rotating logs to `%LOCALAPPDATA%\QUARK\quark.log`.

1. Build `dist\QUARK.exe` with Python 3.12:

   ```powershell
   .\scripts\build_exe.ps1
   ```

2. Install it for the current Windows user and start it now:

   ```powershell
   .\scripts\install_startup.ps1
   ```

   This copies the EXE to `%LOCALAPPDATA%\QUARK\QUARK.exe`, adds a shortcut to the current user's Startup folder, and launches it immediately. It starts again when that Windows user signs in and reconnects automatically when the ESP32 is plugged in.

3. Remove the startup shortcut if needed:

   ```powershell
   .\scripts\remove_startup.ps1
   ```

Codex usage requires the Codex CLI to be installed and signed in for the same Windows user. System telemetry works independently. For a diagnostic run before installing, use `py -3.12 .\quark.py --self-test`; list serial ports with `py -3.12 .\quark.py --list-ports`.

## Flash the firmware

1. Install the Arduino ESP32 core 3.x and the U8g2 library.
2. Open `QUARK.ino` in Arduino IDE and select an ESP32-C3 board.
3. Enable **USB CDC On Boot** so the board exposes its USB serial port.
4. Upload the sketch and close Serial Monitor before the companion connects.

The OLED uses `U8G2_SH1106_128X64_NONAME_F_HW_I2C`. Change that constructor if the display uses another controller. The companion never flashes firmware.

## USB serial protocol

The companion sends newline-terminated messages at 115200 baud:

```text
AI|TODAY=18.4K|LIFE=1.42M|H5=74|WEEK=61|H5R=2h14m|WR=3d4h
SYS|CPU=42|GPU=68|RAM=57|TEMP=71
```

The ESP32 responds to `PING` with `PONG` for port identification. It also accepts `ERR|AI`, `ERR|SYS`, `MODE|AI`, `MODE|SYS`, `REFRESH|AI`, and `REFRESH|SYS`.
