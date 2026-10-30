import argparse
import ctypes
import json
import logging
from logging.handlers import RotatingFileHandler
import math
import os
import queue
import re
import shutil
import socket
import subprocess
import sys
import threading
import time
from collections.abc import Mapping
from datetime import date, timedelta
from pathlib import Path

import psutil
import serial
from serial import SerialException
from serial.tools import list_ports


# ============================================================
# QUARK WINDOWS COMPANION
# ============================================================

BAUD = 115200

# Leave None for automatic ESP32 detection.
# If detection fails, set e.g.:
#
# PORT = "COM7"

PORT = None


SYSTEM_INTERVAL = 2.0

# Don't hammer the Codex backend.
CODEX_INTERVAL = 60.0
CODEX_RPC_TIMEOUT = 15.0
CODEX_RETRY_BASE = 5.0
CODEX_RETRY_MAX = 60.0
CODEX_MIN_REFRESH_INTERVAL = 5.0
AI_HEARTBEAT_INTERVAL = 60.0
SERIAL_MAX_LINE = 160
FIRMWARE_HANDSHAKE_RETRY = 30.0

_network_sample = None
_network_sample_time = 0.0
_link_probe_lock = threading.Lock()
_link_probe_state = {"status": "--", "checked_at": 0.0, "running": False}

logger = logging.getLogger("quark")


def log_directory():
    local_app_data = os.environ.get("LOCALAPPDATA")
    if local_app_data:
        return Path(local_app_data) / "QUARK"
    return Path.home() / ".local" / "share" / "QUARK"


def setup_logging():
    logger.setLevel(logging.INFO)
    logger.propagate = False
    if sys.stdout is None:
        sys.stdout = open(os.devnull, "w", encoding="utf-8")
    if sys.stderr is None:
        sys.stderr = open(os.devnull, "w", encoding="utf-8")
    if logger.handlers:
        return
    try:
        directory = log_directory()
        directory.mkdir(parents=True, exist_ok=True)
        handler = RotatingFileHandler(
            directory / "quark.log",
            maxBytes=1_000_000,
            backupCount=3,
            encoding="utf-8",
        )
        handler.setFormatter(logging.Formatter("%(asctime)s %(levelname)s %(message)s"))
        logger.addHandler(handler)
    except OSError:
        pass
    if sys.stderr is not None:
        console = logging.StreamHandler(sys.stderr)
        console.setFormatter(logging.Formatter("%(levelname)s %(message)s"))
        logger.addHandler(console)
    if not logger.handlers:
        logger.addHandler(logging.NullHandler())


def retry_delay(attempt, base=CODEX_RETRY_BASE, maximum=CODEX_RETRY_MAX):
    try:
        exponent = max(0, int(attempt) - 1)
        return min(float(maximum), float(base) * (2 ** exponent))
    except (TypeError, ValueError, OverflowError):
        return float(maximum)


def _no_window_creation_flags():
    return getattr(subprocess, "CREATE_NO_WINDOW", 0) if os.name == "nt" else 0


def resolve_codex_executable():
    candidates = []
    found = shutil.which("codex")
    if found:
        candidates.append(found)
    if os.name == "nt":
        try:
            result = subprocess.run(
                ["where.exe", "codex"],
                capture_output=True,
                text=True,
                timeout=3,
                creationflags=_no_window_creation_flags(),
                check=False,
            )
            candidates.extend(line.strip() for line in result.stdout.splitlines() if line.strip())
        except (OSError, subprocess.SubprocessError):
            pass
        app_data = os.environ.get("APPDATA")
        if app_data:
            npm = Path(app_data) / "npm"
            candidates.extend(str(npm / name) for name in ("codex.cmd", "codex.exe", "codex.ps1"))

    unique = list(dict.fromkeys(candidates))
    supported = [path for path in unique if Path(path).suffix.lower() != ".ps1"]
    for candidate in supported + [path for path in unique if path not in supported]:
        if Path(candidate).exists() or shutil.which(candidate):
            return candidate
    return None


def codex_command(executable, *arguments):
    extension = Path(executable).suffix.lower()
    if os.name == "nt" and extension in {".cmd", ".bat"}:
        shell = os.environ.get("COMSPEC", "cmd.exe")
        return [shell, "/d", "/c", "call", executable, *arguments]
    if os.name == "nt" and extension == ".ps1":
        powershell = shutil.which("powershell.exe") or "powershell.exe"
        return [powershell, "-NoLogo", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", executable, *arguments]
    return [executable, *arguments]


# ============================================================
# HELPERS
# ============================================================

def compact_tokens(value):
    if value is None:
        return "--"

    try:
        value = int(value)
    except (TypeError, ValueError, OverflowError):
        return "--"

    if value < 0:
        return "--"

    if value >= 1_000_000_000:
        return f"{value / 1_000_000_000:.2f}B"

    if value >= 1_000_000:
        return f"{value / 1_000_000:.1f}M"

    if value >= 1_000:
        return f"{value / 1_000:.1f}K"

    return str(value)


def format_eta(reset_epoch, now=None):
    if reset_epoch is None:
        return "--"

    try:
        reset_epoch = float(reset_epoch)
        current_time = time.time() if now is None else float(now)
    except (TypeError, ValueError, OverflowError):
        return "--"

    if not math.isfinite(reset_epoch) or not math.isfinite(current_time):
        return "--"

    seconds = max(0, int(reset_epoch - current_time))

    days, seconds = divmod(
        seconds,
        86400
    )

    hours, seconds = divmod(
        seconds,
        3600
    )

    minutes = seconds // 60

    if days:
        return f"{days}d{hours}h"

    if hours:
        return f"{hours}h{minutes}m"

    return f"{minutes}m"


def remaining_percent(window):
    if not isinstance(window, Mapping):
        return "--"

    used = window.get(
        "usedPercent"
    )

    if used is None:
        return "--"

    try:
        used_percent = float(used)
    except (TypeError, ValueError, OverflowError):
        return "--"

    if not math.isfinite(used_percent):
        return "--"

    remaining = max(0, min(100, round(100 - used_percent)))

    return str(remaining)


def _numeric_display(value):
    """Return a short integer string from nvidia-smi output, or '--'."""
    text = str(value or "").strip()
    if not text or text.upper() in {"N/A", "[NOT SUPPORTED]"}:
        return "--"

    match = re.match(r"^-?\d+(?:\.\d+)?", text)
    if not match:
        return "--"

    try:
        number = float(match.group(0))
    except ValueError:
        return "--"

    if not math.isfinite(number):
        return "--"
    return str(round(number))


def parse_nvidia_smi_output(output):
    """Parse the first utilization,temperature row emitted by nvidia-smi."""
    if not isinstance(output, str):
        return "--", "--"

    row = next((line.strip() for line in output.splitlines() if line.strip()), "")
    parts = [part.strip() for part in row.split(",")]
    if len(parts) < 2:
        return "--", "--"

    return _numeric_display(parts[0]), _numeric_display(parts[1])


def parse_nvidia_vram_percent(output):
    """Parse used/total MiB from a four-column nvidia-smi row."""
    if not isinstance(output, str):
        return "--"
    row = next((line.strip() for line in output.splitlines() if line.strip()), "")
    parts = [part.strip() for part in row.split(",")]
    if len(parts) < 4:
        return "--"
    used = _numeric_display(parts[2])
    total = _numeric_display(parts[3])
    if used == "--" or total == "--" or int(total) <= 0:
        return "--"
    return str(max(0, min(100, round(100 * int(used) / int(total)))))


def _protocol_value(value, max_length=24):
    if value is None:
        return "--"

    text = str(value).strip()
    if not text or any(character in text for character in "|\r\n"):
        return "--"
    return text[:max_length]


def format_ai_packet(data):
    data = data if isinstance(data, Mapping) else {}
    fields = [
        f"AI|TODAY={_protocol_value(data.get('today'), 12)}",
        f"LIFE={_protocol_value(data.get('lifetime'), 12)}",
        f"H5={_protocol_value(data.get('five_remaining'), 4)}",
        f"WEEK={_protocol_value(data.get('weekly_remaining'), 4)}",
        f"H5R={_protocol_value(data.get('five_reset'), 12)}",
        f"WR={_protocol_value(data.get('weekly_reset'), 12)}",
    ]
    optional_fields = (
        ("PEAK", "peak_daily", 12),
        ("STREAK", "current_streak", 4),
        ("BEST", "longest_streak", 4),
        ("LONGEST", "longest_turn", 12),
        ("D7", "daily_trend", 48),
    )
    for field, key, limit in optional_fields:
        value = _protocol_value(data.get(key), limit)
        if key in data:
            fields.append(f"{field}={value}")
    return "|".join(fields)


def format_system_packet(data):
    data = data if isinstance(data, Mapping) else {}
    fields = [
        f"SYS|CPU={_protocol_value(data.get('cpu'), 4)}",
        f"GPU={_protocol_value(data.get('gpu'), 4)}",
        f"RAM={_protocol_value(data.get('ram'), 4)}",
        f"TEMP={_protocol_value(data.get('temp'), 4)}",
    ]
    optional_fields = (
        ("VRAM", "vram", 4),
        ("DISK", "disk", 4),
        ("FREE", "memory_free", 8),
        ("DFREE", "disk_free", 8),
        ("DN", "down", 8),
        ("UP", "up", 8),
        ("DNP", "down_percent", 3),
        ("UPP", "up_percent", 3),
        ("FREQ", "freq", 6),
        ("SYSUP", "uptime", 12),
        ("BAT", "battery", 4),
        ("AC", "ac", 4),
        ("WEB", "link", 4),
    )
    for field, key, limit in optional_fields:
        value = _protocol_value(data.get(key), limit)
        if key in data:
            fields.append(f"{field}={value}")
    return "|".join(fields)


def parse_esp_command(line):
    """Return a recognized firmware command, ignoring arbitrary serial text."""
    command = str(line or "").strip()
    if command in {
        "PING",
        "REFRESH|AI",
        "REFRESH|SYS",
        "MODE|AI",
        "MODE|SYS",
        "QUARK|READY",
        "PONG",
    }:
        return command
    return None


# ============================================================
# ESP32 SERIAL PORT DISCOVERY
# ============================================================

def list_ports_info():
    ports = []
    for port in list_ports.comports():
        description = port.description or ""
        hwid = port.hwid or ""
        search = f"{description} {hwid}".lower()
        candidate = port.vid == 0x303A or any(
            token in search for token in ("espressif", "esp32", "usb jtag", "usb serial")
        )
        ports.append({
            "device": port.device,
            "description": description,
            "hwid": hwid,
            "vid": port.vid,
            "pid": port.pid,
            "candidate": candidate,
        })
    return ports


def _probe_quark_port(device_name, timeout=4.0):
    deadline = time.monotonic() + timeout
    try:
        with serial.Serial(device_name, BAUD, timeout=0.1, write_timeout=0.5) as device:
            while time.monotonic() < deadline:
                device.write(b"PING\n")
                device.flush()
                raw = device.readline()
                if raw.endswith(b"\n") and len(raw) <= SERIAL_MAX_LINE:
                    command = parse_esp_command(raw.decode("utf-8", errors="ignore"))
                    if command in {"PONG", "QUARK|READY"}:
                        return True
                time.sleep(0.15)
    except (SerialException, OSError):
        return False
    return False


def find_quark_port(port_override=None):
    override = port_override or PORT
    if override:
        return override

    candidates = [port for port in list_ports_info() if port["candidate"]]
    if len(candidates) == 1:
        return candidates[0]["device"]
    if len(candidates) > 1:
        matches = [port["device"] for port in candidates if _probe_quark_port(port["device"])]
        if len(matches) == 1:
            return matches[0]
        if matches:
            logger.warning("Multiple QUARK devices answered the serial handshake: %s", matches)
        else:
            logger.info("No ESP candidate answered the QUARK handshake")
    return None


# ============================================================
# SYSTEM TELEMETRY
# ============================================================

def get_nvidia_stats():
    executable = shutil.which("nvidia-smi")
    if executable is None:
        return "--", "--", "--"

    try:
        result = subprocess.run(
            [
                executable,
                "--query-gpu=utilization.gpu,temperature.gpu,memory.used,memory.total",
                "--format=csv,noheader,nounits",
            ],
            capture_output=True,
            text=True,
            timeout=2,
            creationflags=_no_window_creation_flags(),
            check=False,
        )
        if result.returncode != 0:
            logger.debug("nvidia-smi returned %s", result.returncode)
            return "--", "--", "--"
        gpu, temperature = parse_nvidia_smi_output(result.stdout)
        return gpu, temperature, parse_nvidia_vram_percent(result.stdout)
    except (OSError, subprocess.SubprocessError) as exc:
        logger.debug("NVIDIA telemetry unavailable: %s", exc)
        return "--", "--", "--"


def compact_gigabytes(byte_count):
    try:
        gib = max(0.0, float(byte_count)) / (1024 ** 3)
    except (TypeError, ValueError, OverflowError):
        return "--"
    if not math.isfinite(gib):
        return "--"
    return f"{gib:.1f}G" if gib < 10 else f"{gib:.0f}G"


def compact_rate(bytes_per_second):
    try:
        rate = max(0.0, float(bytes_per_second))
    except (TypeError, ValueError, OverflowError):
        return "--"
    if not math.isfinite(rate):
        return "--"
    if rate < 1024:
        return f"{round(rate)}B"
    if rate < 1024 ** 2:
        value, suffix = rate / 1024, "K"
    elif rate < 1024 ** 3:
        value, suffix = rate / (1024 ** 2), "M"
    else:
        value, suffix = rate / (1024 ** 3), "G"
    return f"{value:.1f}{suffix}" if value < 10 else f"{value:.0f}{suffix}"


def rate_chart_percent(bytes_per_second, ceiling=20 * 1024 ** 2):
    try:
        rate = max(0.0, float(bytes_per_second))
        limit = max(1.0, float(ceiling))
    except (TypeError, ValueError, OverflowError):
        return "--"
    if not math.isfinite(rate) or not math.isfinite(limit):
        return "--"
    return str(max(0, min(100, round(100 * rate / limit))))


def compact_duration(seconds):
    try:
        remaining = max(0, int(seconds))
    except (TypeError, ValueError, OverflowError):
        return "--"
    days, remaining = divmod(remaining, 86400)
    hours, remaining = divmod(remaining, 3600)
    minutes = remaining // 60
    if days:
        return f"{days}d{hours}h"
    if hours:
        return f"{hours}h{minutes}m"
    return f"{minutes}m"


def _summary_integer(usage_result, field):
    if not isinstance(usage_result, Mapping):
        return None
    summary = usage_result.get("summary")
    if not isinstance(summary, Mapping):
        return None
    try:
        value = summary.get(field)
        return max(0, int(value)) if value is not None else None
    except (TypeError, ValueError, OverflowError):
        return None


def extract_daily_token_trend(usage_result, today=None):
    """Return seven daily token buckets normalized to 0-100 for the OLED chart."""
    if not isinstance(usage_result, Mapping):
        return None
    buckets = usage_result.get("dailyUsageBuckets")
    if not isinstance(buckets, list):
        return None

    reference = today if isinstance(today, date) else date.today()
    if isinstance(today, str):
        try:
            reference = date.fromisoformat(today)
        except ValueError:
            return None

    values_by_day = {}
    for bucket in buckets:
        if not isinstance(bucket, Mapping):
            continue
        try:
            day = date.fromisoformat(str(bucket.get("startDate", "")))
            tokens = max(0, int(bucket.get("tokens")))
        except (TypeError, ValueError, OverflowError):
            continue
        values_by_day[day] = tokens

    values = [values_by_day.get(reference - timedelta(days=offset), 0) for offset in range(6, -1, -1)]
    peak = max(values, default=0)
    if peak <= 0:
        return ",".join("0" for _ in values)
    return ",".join(str(round(value * 100 / peak)) for value in values)


def _probe_chatgpt_link():
    try:
        with socket.create_connection(("chatgpt.com", 443), timeout=2.0):
            status = "OK"
    except OSError:
        status = "NO"
    with _link_probe_lock:
        _link_probe_state["status"] = status
        _link_probe_state["checked_at"] = time.monotonic()
        _link_probe_state["running"] = False


def get_chatgpt_link_status():
    now = time.monotonic()
    with _link_probe_lock:
        if not _link_probe_state["running"] and now - _link_probe_state["checked_at"] >= 30:
            _link_probe_state["running"] = True
            threading.Thread(target=_probe_chatgpt_link, name="QUARK-link-probe", daemon=True).start()
        return _link_probe_state["status"]


def get_network_rates():
    global _network_sample, _network_sample_time
    try:
        counters = psutil.net_io_counters()
    except (OSError, RuntimeError):
        logger.debug("Network throughput telemetry unavailable", exc_info=True)
        return "--", "--", "--", "--"
    now = time.monotonic()
    if counters is None:
        return "--", "--", "--", "--"

    current = (counters.bytes_recv, counters.bytes_sent)
    if _network_sample is None or now <= _network_sample_time:
        download = upload = 0.0
    else:
        elapsed = now - _network_sample_time
        download = max(0, current[0] - _network_sample[0]) / elapsed
        upload = max(0, current[1] - _network_sample[1]) / elapsed

    _network_sample = current
    _network_sample_time = now
    return (
        compact_rate(download),
        compact_rate(upload),
        rate_chart_percent(download),
        rate_chart_percent(upload),
    )


def get_system_data():
    cpu = round(psutil.cpu_percent(interval=0.15))
    memory = psutil.virtual_memory()
    ram = round(memory.percent)
    gpu, temp, vram = get_nvidia_stats()

    disk_percent = disk_free = "--"
    system_drive = os.environ.get("SystemDrive")
    disk_target = f"{system_drive}\\" if system_drive else Path.home().anchor or "."
    try:
        disk = psutil.disk_usage(disk_target)
        disk_percent = str(round(disk.percent))
        disk_free = compact_gigabytes(disk.free)
    except (OSError, ValueError):
        logger.debug("System drive telemetry unavailable", exc_info=True)

    down, up, down_percent, up_percent = get_network_rates()
    frequency = "--"
    try:
        clock = psutil.cpu_freq()
        if clock and math.isfinite(clock.current) and clock.current > 0:
            ghz = clock.current / 1000
            frequency = f"{ghz:.1f}G"
    except (OSError, AttributeError, TypeError, ValueError):
        logger.debug("CPU frequency telemetry unavailable", exc_info=True)

    try:
        uptime = compact_duration(time.time() - psutil.boot_time())
    except (OSError, ValueError):
        uptime = "--"

    battery_percent = ac_state = "--"
    try:
        battery = psutil.sensors_battery()
        if battery is not None:
            battery_percent = str(max(0, min(100, round(battery.percent))))
            ac_state = "ON" if battery.power_plugged else "OFF"
    except (OSError, AttributeError, ValueError):
        logger.debug("Battery telemetry unavailable", exc_info=True)

    return {
        "cpu": str(cpu),
        "ram": str(ram),
        "gpu": gpu,
        "temp": temp,
        "vram": vram,
        "memory_free": compact_gigabytes(memory.available),
        "disk": disk_percent,
        "disk_free": disk_free,
        "down": down,
        "up": up,
        "down_percent": down_percent,
        "up_percent": up_percent,
        "freq": frequency,
        "uptime": uptime,
        "battery": battery_percent,
        "ac": ac_state,
        "link": get_chatgpt_link_status(),
    }


# ============================================================
# CODEX APP SERVER
# ============================================================

class CodexAppServer:
    def __init__(self):
        self.process = None
        self.messages = queue.Queue()
        self.request_id = 1
        self.rpc_lock = threading.Lock()
        self.start_lock = threading.Lock()
        self.write_lock = threading.Lock()
        self.initialized = False
        self.failures = 0
        self.last_start_attempt = 0.0

    def start(self):
        with self.start_lock:
            if self.process is not None and self.process.poll() is None and self.initialized:
                return

            now = time.monotonic()
            wait_for = retry_delay(self.failures) - (now - self.last_start_attempt)
            if self.failures and wait_for > 0:
                raise RuntimeError(f"Codex CLI retrying in {wait_for:.0f}s")

            self.last_start_attempt = now
            executable = resolve_codex_executable()
            if executable is None:
                self.failures += 1
                raise RuntimeError("Codex CLI was not found in PATH or %APPDATA%\\npm")

            self.stop()
            try:
                self.messages = queue.Queue()
                command = codex_command(executable, "app-server")
                self.process = subprocess.Popen(
                    command,
                    stdin=subprocess.PIPE,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    text=True,
                    encoding="utf-8",
                    errors="replace",
                    bufsize=1,
                    creationflags=_no_window_creation_flags(),
                )
                process = self.process
                threading.Thread(
                    target=self._stdout_reader,
                    args=(process,),
                    name="QUARK-Codex-stdout",
                    daemon=True,
                ).start()
                threading.Thread(
                    target=self._stderr_reader,
                    args=(process,),
                    name="QUARK-Codex-stderr",
                    daemon=True,
                ).start()

                self._rpc_raw(
                    "initialize",
                    {
                        "clientInfo": {
                            "name": "quark",
                            "title": "QUARK",
                            "version": "1.2.0",
                        }
                    },
                    timeout=CODEX_RPC_TIMEOUT,
                )
                self._send({"method": "initialized"})
                self.initialized = True
                self.failures = 0
                logger.info("Codex App Server initialized")
            except Exception:
                self.failures += 1
                self.stop()
                raise

    def _stdout_reader(self, process):
        try:
            for raw_line in process.stdout:
                line = raw_line.strip()
                if not line:
                    continue
                try:
                    self.messages.put(json.loads(line))
                except json.JSONDecodeError:
                    logger.debug("Ignoring non-JSON Codex output: %s", line[:300])
        except (OSError, ValueError) as exc:
            logger.debug("Codex stdout reader stopped: %s", exc)

    def _stderr_reader(self, process):
        try:
            for raw_line in process.stderr:
                line = raw_line.strip()
                if line:
                    logger.info("Codex: %s", line[:500])
        except (OSError, ValueError) as exc:
            logger.debug("Codex stderr reader stopped: %s", exc)

    def _send(self, message):
        process = self.process
        if process is None or process.poll() is not None or process.stdin is None:
            raise RuntimeError("Codex App Server is not running")

        with self.write_lock:
            process.stdin.write(json.dumps(message, separators=(",", ":")) + "\n")
            process.stdin.flush()

    def _rpc_raw(self, method, params=None, timeout=CODEX_RPC_TIMEOUT):
        with self.rpc_lock:
            request_id = self.request_id
            self.request_id += 1
            message = {"id": request_id, "method": method}
            if params is not None:
                message["params"] = params
            self._send(message)

            deadline = time.monotonic() + timeout
            while True:
                remaining = deadline - time.monotonic()
                if remaining <= 0:
                    raise TimeoutError(f"Codex RPC timed out: {method}")
                try:
                    response = self.messages.get(timeout=remaining)
                except queue.Empty as exc:
                    raise TimeoutError(f"Codex RPC timed out: {method}") from exc

                if not isinstance(response, dict) or response.get("id") != request_id:
                    continue
                if "error" in response:
                    raise RuntimeError(f"{method}: {response['error']}")
                return response.get("result", {})

    def rpc(self, method, params=None, timeout=CODEX_RPC_TIMEOUT):
        self.start()
        return self._rpc_raw(method, params, timeout)

    def get_token_usage(self):
        return self.rpc("account/usage/read")

    def get_rate_limits(self):
        return self.rpc("account/rateLimits/read")

    def stop(self):
        self.initialized = False
        process = self.process
        self.process = None
        if process is None:
            return
        try:
            if process.poll() is None:
                process.terminate()
                try:
                    process.wait(timeout=2)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait(timeout=2)
        except (OSError, subprocess.SubprocessError):
            logger.debug("Error stopping Codex App Server", exc_info=True)


class FirmwareHandshakeError(SerialException):
    """An ESP32 serial device is present, but its firmware isn't ready for QUARK."""

# ============================================================
# CODEX DATA PARSING
# ============================================================

def select_codex_snapshot(result):
    if not isinstance(result, Mapping):
        return {}

    by_limit = result.get("rateLimitsByLimitId")
    if isinstance(by_limit, Mapping):
        snapshot = by_limit.get("codex")
        if isinstance(snapshot, Mapping):
            return snapshot

        for value in by_limit.values():
            if isinstance(value, Mapping) and value.get("limitId") == "codex":
                return value

    legacy = result.get("rateLimits")
    return legacy if isinstance(legacy, Mapping) else {}


def extract_windows(
    rate_result
):
    snapshot = select_codex_snapshot(rate_result)
    five_hour = None
    weekly = None

    for window in snapshot.values():
        if not isinstance(window, Mapping):
            continue
        try:
            duration = int(window.get("windowDurationMins"))
        except (TypeError, ValueError, OverflowError):
            continue

        if duration == 300:
            five_hour = window
        elif duration == 10080:
            weekly = window

    return five_hour, weekly


def extract_today_tokens(
    usage_result,
    today=None,
):
    if not isinstance(usage_result, Mapping):
        return None

    buckets = usage_result.get("dailyUsageBuckets")
    if not isinstance(buckets, list):
        return None

    today_string = today.isoformat() if hasattr(today, "isoformat") else today
    if today_string is None:
        today_string = date.today().isoformat()

    for bucket in buckets:
        if isinstance(bucket, Mapping) and bucket.get("startDate") == today_string:
            tokens = bucket.get("tokens")
            try:
                return max(0, int(tokens))
            except (TypeError, ValueError, OverflowError):
                return None

    # A valid bucket list with no matching date means zero tokens today.
    return 0


def extract_lifetime_tokens(
    usage_result
):
    if not isinstance(usage_result, Mapping):
        return None

    summary = usage_result.get("summary")
    if not isinstance(summary, Mapping):
        return None

    value = summary.get("lifetimeTokens")
    try:
        return max(0, int(value)) if value is not None else None
    except (TypeError, ValueError, OverflowError):
        return None


# ============================================================
# FETCH COMPLETE CODEX SNAPSHOT
# ============================================================

def get_codex_data(
    codex
):

    usage_result = None
    rate_result = None


    # Token profile
    try:

        usage_result = (
            codex.get_token_usage()
        )

    except Exception as exc:

        print(
            "[AI] Token usage unavailable:",
            exc
        )


    # Quotas
    try:

        rate_result = (
            codex.get_rate_limits()
        )

    except Exception as exc:

        print(
            "[AI] Rate limits unavailable:",
            exc
        )


    if (
        usage_result is None
        and
        rate_result is None
    ):

        raise RuntimeError(
            "No Codex account data available."
        )


    today_tokens = None
    lifetime_tokens = None
    peak_daily_tokens = None
    current_streak = None
    longest_streak = None
    longest_turn = None
    daily_trend = None


    if usage_result is not None:

        today_tokens = (
            extract_today_tokens(
                usage_result
            )
        )


        lifetime_tokens = (
            extract_lifetime_tokens(
                usage_result
            )
        )

        peak_daily_tokens = _summary_integer(usage_result, "peakDailyTokens")
        current_streak = _summary_integer(usage_result, "currentStreakDays")
        longest_streak = _summary_integer(usage_result, "longestStreakDays")
        longest_turn_seconds = _summary_integer(usage_result, "longestRunningTurnSec")
        longest_turn = compact_duration(longest_turn_seconds) if longest_turn_seconds is not None else None
        daily_trend = extract_daily_token_trend(usage_result)


    five_hour = None
    weekly = None


    if rate_result is not None:

        five_hour, weekly = (
            extract_windows(
                rate_result
            )
        )


    data = {

        "today":
            compact_tokens(
                today_tokens
            ),

        "lifetime":
            compact_tokens(
                lifetime_tokens
            ),

        "five_remaining":
            remaining_percent(
                five_hour
            ),

        "weekly_remaining":
            remaining_percent(
                weekly
            ),

        "five_reset":
            format_eta(
                five_hour.get(
                    "resetsAt"
                )
                if five_hour
                else None
            ),

        "weekly_reset":
            format_eta(
                weekly.get(
                    "resetsAt"
                )
                if weekly
                else None
            )
    }

    data.update({
        "peak_daily": compact_tokens(peak_daily_tokens),
        "current_streak": str(current_streak) if current_streak is not None else "--",
        "longest_streak": str(longest_streak) if longest_streak is not None else "--",
        "longest_turn": longest_turn or "--",
        "daily_trend": daily_trend or "--",
    })
    return data


# ============================================================
# SERIAL OUTPUT
# ============================================================

def send_line(device, line):
    message = (str(line) + "\n").encode("ascii", errors="strict")
    if len(message) > SERIAL_MAX_LINE:
        raise ValueError("Serial packet exceeds the firmware line limit")
    device.write(message)
    device.flush()


# ============================================================
# CODEX POLLING
# ============================================================

def codex_poll_worker(codex, state, state_lock, refresh_event, stop_event):
    last_attempt = 0.0
    while not stop_event.is_set():
        now = time.monotonic()
        wait_time = max(0.0, CODEX_INTERVAL - (now - last_attempt))
        refresh_event.wait(wait_time)
        if stop_event.is_set():
            return
        refresh_event.clear()

        cooldown = max(0.0, CODEX_MIN_REFRESH_INTERVAL - (time.monotonic() - last_attempt))
        if cooldown and stop_event.wait(cooldown):
            return

        last_attempt = time.monotonic()
        try:
            data = get_codex_data(codex)
            error = None
        except Exception as exc:
            data = None
            error = str(exc)
            logger.warning("Codex data unavailable: %s", exc)

        with state_lock:
            state["data"] = data
            state["error"] = error
            state["version"] += 1
            state["updated"] = time.monotonic()


def wait_for_device_handshake(device, port, timeout=8.0):
    deadline = time.monotonic() + timeout
    next_ping = 0.0
    last_write_error = None
    while time.monotonic() < deadline:
        while device.in_waiting:
            raw = device.read_until(b"\n", SERIAL_MAX_LINE + 1)
            if len(raw) > SERIAL_MAX_LINE or not raw.endswith(b"\n"):
                continue
            command = parse_esp_command(raw.decode("ascii", errors="ignore"))
            if command == "PONG":
                logger.info("QUARK firmware handshake succeeded on %s", port)
                return
            if command == "QUARK|READY":
                logger.info("QUARK firmware booted on %s", port)

        now = time.monotonic()
        if now >= next_ping:
            try:
                send_line(device, "PING")
                last_write_error = None
            except SerialException as exc:
                last_write_error = str(exc)
            next_ping = now + 0.5
        time.sleep(0.05)

    detail = f"; serial write error: {last_write_error}" if last_write_error else ""
    raise FirmwareHandshakeError(
        f"No QUARK PONG received on {port}{detail}. Check USB CDC On Boot and the flashed firmware."
    )


# ============================================================
# RUN QUARK DEVICE
# ============================================================

def run_device(port, ai_state, ai_state_lock, refresh_event):
    logger.info("Connecting to QUARK on %s", port)
    with serial.Serial(
        port,
        BAUD,
        timeout=0.1,
        write_timeout=0.5,
    ) as device:
        # Opening USB CDC may reset the ESP32-C3.
        time.sleep(3.5)
        wait_for_device_handshake(device, port)
        logger.info("QUARK connected on %s", port)

        next_system = 0.0
        last_ai_version = -1
        while True:
            now = time.monotonic()

            # Drain serial commands without allowing an oversized line to grow unbounded.
            while device.in_waiting:
                raw = device.read_until(b"\n", SERIAL_MAX_LINE + 1)
                if len(raw) > SERIAL_MAX_LINE or not raw.endswith(b"\n"):
                    logger.warning("Dropped malformed or oversized serial input")
                    continue
                command = parse_esp_command(raw.decode("ascii", errors="ignore"))
                if command is None:
                    continue
                logger.info("ESP command: %s", command)
                if command in {"REFRESH|AI", "MODE|AI"}:
                    refresh_event.set()
                elif command in {"REFRESH|SYS", "MODE|SYS"}:
                    next_system = 0.0

            if now >= next_system:
                try:
                    packet = format_system_packet(get_system_data())
                    send_line(device, packet)
                    logger.debug("System packet: %s", packet)
                except (SerialException, OSError):
                    raise
                except Exception as exc:
                    logger.exception("System telemetry update failed: %s", exc)
                    send_line(device, "ERR|SYS")
                next_system = now + SYSTEM_INTERVAL

            with ai_state_lock:
                version = ai_state["version"]
                data = ai_state["data"]
                error = ai_state["error"]

            if version != last_ai_version:
                if data is not None:
                    send_line(device, format_ai_packet(data))
                elif error is not None:
                    send_line(device, "ERR|AI")
                last_ai_version = version

            time.sleep(0.05)


# ============================================================
# SINGLE INSTANCE
# ============================================================

def acquire_single_instance():
    if os.name != "nt":
        return None
    kernel32 = ctypes.windll.kernel32
    handle = kernel32.CreateMutexW(None, False, "Local\\QUARK.Companion")
    if not handle:
        raise OSError("Could not create the QUARK single-instance mutex")
    if kernel32.GetLastError() == 183:  # ERROR_ALREADY_EXISTS
        kernel32.CloseHandle(handle)
        return False
    return handle


# ============================================================
# MAIN
# ============================================================

def main(argv=None):
    parser = argparse.ArgumentParser(description="QUARK Windows companion")
    parser.add_argument("--port", help="use a specific serial port, such as COM7")
    parser.add_argument("--list-ports", action="store_true", help="show detected serial ports and exit")
    parser.add_argument("--self-test", action="store_true", help="check telemetry and local dependencies")
    parser.add_argument("--version", action="version", version="QUARK Companion 1.2.0")
    args = parser.parse_args(argv)

    setup_logging()
    logger.info("QUARK Companion starting")

    if args.list_ports:
        print(json.dumps(list_ports_info(), indent=2))
        return 0

    if args.self_test:
        ports = list_ports_info()
        telemetry = get_system_data()
        logger.info("Python runtime: %s", sys.version.split()[0])
        logger.info("Codex CLI: %s", resolve_codex_executable() or "not found (AI mode unavailable)")
        logger.info("Serial ports: %s", ports or "none detected")
        logger.info("System telemetry: %s", telemetry)
        return 0

    mutex = acquire_single_instance()
    if mutex is False:
        logger.info("Another QUARK Companion instance is already running")
        return 0

    codex = CodexAppServer()
    ai_state = {"data": None, "error": None, "version": 0, "updated": 0.0}
    ai_state_lock = threading.Lock()
    refresh_event = threading.Event()
    stop_event = threading.Event()
    worker = threading.Thread(
        target=codex_poll_worker,
        args=(codex, ai_state, ai_state_lock, refresh_event, stop_event),
        name="QUARK-Codex-poller",
        daemon=True,
    )
    worker.start()

    try:
        port_override = args.port or PORT
        last_missing_log = 0.0
        while True:
            port = find_quark_port(port_override)
            if port is None:
                now = time.monotonic()
                if now - last_missing_log >= 30:
                    logger.info("No ESP32-C3 serial device found; waiting for USB connection")
                    last_missing_log = now
                time.sleep(2)
                continue

            try:
                run_device(port, ai_state, ai_state_lock, refresh_event)
            except FirmwareHandshakeError as exc:
                logger.warning("%s; retrying in %.0fs", exc, FIRMWARE_HANDSHAKE_RETRY)
                time.sleep(FIRMWARE_HANDSHAKE_RETRY)
            except (SerialException, OSError) as exc:
                logger.warning("QUARK disconnected from %s: %s", port, exc)
                time.sleep(2)
            except KeyboardInterrupt:
                raise
            except Exception:
                logger.exception("Unexpected QUARK connection error")
                time.sleep(2)
    except KeyboardInterrupt:
        logger.info("Stopping QUARK Companion")
        return 0
    finally:
        stop_event.set()
        refresh_event.set()
        codex.stop()
        if mutex and mutex is not False and os.name == "nt":
            ctypes.windll.kernel32.CloseHandle(mutex)


if __name__ == "__main__":
    raise SystemExit(main())
