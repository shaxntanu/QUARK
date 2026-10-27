# QUARK

QUARK is an ESP32-C3 desk display for Codex usage and live PC telemetry. The ESP32 drives a 128×64 SH1106 OLED, button, mode switch, buzzer, and RGB LED. The Windows companion (`quark.py`) reads local system stats and Codex App Server data, then sends them over USB serial.

## Hardware and pins

| Part | ESP32-C3 GPIO |
|---|---:|
| OLED SDA | 0 |
| OLED SCL | 1 |
| Page/refresh button | 3 |
| RGB red | 4 |
| Buzzer | 5 |
| AI/System switch | 6 |
| RGB green | 7 |
| RGB blue | 10 |

The button and mode switch use `INPUT_PULLUP` and should connect to GND when closed. Use a common-cathode RGB LED with suitable current-limiting resistors.

## OLED pages

The short button press advances the current mode's four pages. Hold it for about 0.8 seconds to request a refresh.

- **AI:** today's tokens, lifetime tokens, five-hour remaining quota, and weekly remaining quota.
- **System:** CPU/GPU/RAM overview, CPU details, GPU load and temperature, and RAM details.
- Quota and utilization pages show compact progress bars. The quota pages flag remaining allowance at or below 15%.
- The header shows the active mode, PC connection state, and current page.

## RGB status colors

| State | AI mode | System mode |
|---|---|---|
| Connected | Green | Green + Blue (cyan) |
| Refreshing | Blue | Blue + Red (magenta) |
| Offline or error | Red | Red + Green (yellow) |

## Flash the firmware

1. Install the Arduino ESP32 core 3.x and the U8g2 library.
2. Open `QUARK.ino` in Arduino IDE and select an ESP32-C3 board.
3. Enable **USB CDC On Boot** so the board exposes its USB serial port.
4. Upload the sketch. Close Serial Monitor before starting the companion app.

The OLED uses the U8g2 `U8G2_SH1106_128X64_NONAME_F_HW_I2C` driver. If your module uses a different controller, change that constructor to match the display.

## Run the Windows companion

The companion requires Python, the Codex CLI signed into the account to monitor, and the Python packages in `requirements.txt`.

```powershell
py -m pip install -r requirements.txt
py .\quark.py
```

It auto-detects Espressif serial ports. If more than one board is connected, set `PORT = "COMx"` near the top of `quark.py`. NVIDIA GPU load and temperature are read through `nvidia-smi`; without it, the GPU fields display `--`.

## USB serial protocol

The companion sends newline-terminated messages at 115200 baud:

```text
AI|TODAY=18.4K|LIFE=1.42M|H5=74|WEEK=61|H5R=2h14m|WR=3d4h
SYS|CPU=42|GPU=68|RAM=57|TEMP=71
```

The ESP32 accepts `ERR|AI`, `ERR|SYS`, `MODE|AI`, `MODE|SYS`, `REFRESH|AI`, and `REFRESH|SYS` as described by the companion protocol.
