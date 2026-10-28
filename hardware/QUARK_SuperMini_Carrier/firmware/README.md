# Firmware image

`QUARK_ESP32-C3_SuperMini_CDC.bin` is a merged 4 MB flash image compiled for `esp32:esp32:esp32c3:CDCOnBoot=cdc` with ESP32 Arduino core 3.3.10 and U8g2 2.36.19. USB CDC on boot is enabled for the companion's serial connection.

When the selected mode changes from waiting/offline to live data, the firmware plays one short rising two-note acknowledgement on the passive piezo. It does not chime on each routine telemetry packet.

With the ESP32-C3 in download mode, flash it from PowerShell using the `esptool.exe` bundled with the Arduino ESP32 package:

```powershell
$esptool = Join-Path $env:LOCALAPPDATA 'Arduino15\packages\esp32\tools\esptool_py\5.3.0\esptool.exe'
& $esptool --chip esp32c3 --port COM5 --baud 460800 write-flash 0x0 .\QUARK_ESP32-C3_SuperMini_CDC.bin
```

Replace `COM5` with the port assigned to the board. Or upload `QUARK.ino` from Arduino IDE with **USB CDC On Boot** enabled. The `.bin` is a complete flash image; flashing it writes the 4 MB image at address `0x0`.
