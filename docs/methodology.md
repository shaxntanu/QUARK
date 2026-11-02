# Data collection and processing methodology

This document describes the implemented engineering workflow. It is not a scientific experimental protocol; the repository contains no controlled study or measured benchmark dataset.

## Windows metrics

The companion samples CPU use, CPU frequency, virtual memory, system-drive capacity, network byte counters, uptime, and battery state through `psutil`. The CPU percentage call samples for 0.15 seconds. Network throughput is derived from the change in cumulative byte counters between polling passes. The NVIDIA path calls `nvidia-smi` for utilization, temperature, and used/total VRAM; unsupported or missing values become `--`.

The reachability indicator opens a TCP connection to `chatgpt.com:443` with a two-second timeout, at most once every 30 seconds. It does not perform TLS, HTTP, login, or Codex API validation. An `OK` result therefore indicates only that a TCP connection succeeded at that time.

## Codex usage

The companion starts `codex app-server` as a child process and exchanges JSON-RPC messages through standard input/output. It requests token usage and rate limits, selects the Codex limit snapshot, derives daily counts/streak summaries, and normalizes seven daily token buckets for the relative OLED trend. The app-server is polled about once per minute; retries use bounded exponential backoff. The reader queues only response objects and does not retain unsolicited notification or request messages. A failure in this worker does not stop the separate system telemetry loop.

The App Server protocol and usage fields can evolve independently of QUARK. Missing, malformed, or unsupported values are rendered as `--` where the parser can safely continue.

## Serial and firmware processing

The companion builds newline-terminated ASCII `AI` and `SYS` records and sends them over USB CDC at 115200 baud. Values are sanitized to printable ASCII. Required fields are retained; optional fields are appended in priority order only while the complete record, including newline, fits the shared 160-byte limit.

The ESP32-C3 rejects malformed/oversized input, updates the selected data structure, and refreshes the OLED. It retains graph history for short-term metric charts; the Codex trend is a seven-day relative series. Physical inputs are debounced in firmware. Connection grace and stability timers prevent transient feed changes from triggering repeated acknowledgement sounds.

## Verification method

The Python suite tests parsing, formatting, missing-value handling, and protocol bounds without needing a Codex account or device. The firmware is compiled for the target FQBN. KiCad CLI ERC/DRC checks operate on the checked-in schematic and board. These checks establish source-level/build/CAD evidence only; they do not establish electrical behavior on a physical assembly, RF/EMC compliance, thermal margins, or user experience.
