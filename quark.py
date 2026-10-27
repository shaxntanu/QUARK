import json
import math
import os
import queue
import re
import shutil
import subprocess
import sys
import threading
import time
from collections.abc import Mapping
from datetime import date

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
    parts = row.split(",", 1)
    if len(parts) != 2:
        return "--", "--"

    return _numeric_display(parts[0]), _numeric_display(parts[1])


def _protocol_value(value, max_length=24):
    if value is None:
        return "--"

    text = str(value).strip()
    if not text or any(character in text for character in "|\r\n"):
        return "--"
    return text[:max_length]


def format_ai_packet(data):
    data = data if isinstance(data, Mapping) else {}
    return (
        f"AI|TODAY={_protocol_value(data.get('today'), 12)}"
        f"|LIFE={_protocol_value(data.get('lifetime'), 12)}"
        f"|H5={_protocol_value(data.get('five_remaining'), 4)}"
        f"|WEEK={_protocol_value(data.get('weekly_remaining'), 4)}"
        f"|H5R={_protocol_value(data.get('five_reset'), 12)}"
        f"|WR={_protocol_value(data.get('weekly_reset'), 12)}"
    )


def format_system_packet(data):
    data = data if isinstance(data, Mapping) else {}
    return (
        f"SYS|CPU={_protocol_value(data.get('cpu'), 4)}"
        f"|GPU={_protocol_value(data.get('gpu'), 4)}"
        f"|RAM={_protocol_value(data.get('ram'), 4)}"
        f"|TEMP={_protocol_value(data.get('temp'), 4)}"
    )


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

def find_quark_port():

    ports = list(
        list_ports.comports()
    )

    # Manual override
    if PORT:

        return PORT


    candidates = []

    for port in ports:

        description = (
            port.description or ""
        ).lower()

        hwid = (
            port.hwid or ""
        ).lower()


        # Espressif USB VID
        if port.vid == 0x303A:

            candidates.append(
                port.device
            )

            continue


        if (
            "espressif" in description
            or
            "usb jtag" in description
            or
            "esp32" in description
            or
            "espressif" in hwid
        ):

            candidates.append(
                port.device
            )


    candidates = list(
        dict.fromkeys(candidates)
    )


    if len(candidates) == 1:

        return candidates[0]


    if len(candidates) > 1:

        print(
            "Multiple ESP ports found:",
            candidates
        )

        print(
            "Set PORT = \"COMx\" at the top of quark.py."
        )

        return None


    return None


# ============================================================
# SYSTEM TELEMETRY
# ============================================================

def get_nvidia_stats():
    executable = shutil.which(
        "nvidia-smi"
    )

    if executable is None:

        return "--", "--"


    try:

        command = [
            executable,
            "--query-gpu=utilization.gpu,temperature.gpu",
            "--format=csv,noheader,nounits"
        ]


        creation_flags = 0

        if os.name == "nt":
            creation_flags = subprocess.CREATE_NO_WINDOW


        output = subprocess.check_output(
            command,
            text=True,
            timeout=3,
            creationflags=creation_flags
        )


        # If there are multiple GPUs, the parser uses the first row.
        return parse_nvidia_smi_output(output)


    except Exception:

        return "--", "--"


def get_system_data():

    cpu = round(
        psutil.cpu_percent(
            interval=0.15
        )
    )


    ram = round(
        psutil.virtual_memory().percent
    )


    gpu, temp = (
        get_nvidia_stats()
    )


    return {
        "cpu": str(cpu),
        "ram": str(ram),
        "gpu": str(gpu),
        "temp": str(temp)
    }


# ============================================================
# CODEX APP SERVER
# ============================================================

class CodexAppServer:

    def __init__(self):

        self.process = None

        self.messages = (
            queue.Queue()
        )

        self.request_id = 1


    # --------------------------------------------------------
    # START
    # --------------------------------------------------------

    def start(self):

        codex = shutil.which(
            "codex"
        )


        if codex is None:

            raise RuntimeError(
                "Codex CLI not found in PATH."
            )


        # npm installs on Windows can resolve to codex.cmd.
        # subprocess needs cmd.exe for .cmd / .bat.

        if (
            os.name == "nt"
            and
            codex.lower().endswith(
                (".cmd", ".bat")
            )
        ):

            command = [
                "cmd",
                "/c",
                codex,
                "app-server"
            ]

        else:

            command = [
                codex,
                "app-server"
            ]


        self.process = subprocess.Popen(
            command,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            encoding="utf-8",
            errors="replace",
            bufsize=1
        )


        threading.Thread(
            target=self._stdout_reader,
            daemon=True
        ).start()


        threading.Thread(
            target=self._stderr_reader,
            daemon=True
        ).start()


        result = self.rpc(
            "initialize",
            {
                "clientInfo": {
                    "name": "quark",
                    "title": "QUARK",
                    "version": "1.0.0"
                }
            },
            timeout=15
        )


        self.notify(
            "initialized"
        )


        print(
            "[CODEX] App Server initialized"
        )


    # --------------------------------------------------------
    # STDOUT JSON READER
    # --------------------------------------------------------

    def _stdout_reader(self):

        for raw_line in self.process.stdout:

            line = raw_line.strip()

            if not line:
                continue


            try:

                message = json.loads(
                    line
                )

                self.messages.put(
                    message
                )


            except json.JSONDecodeError:

                print(
                    "[CODEX RAW]",
                    line
                )


    # --------------------------------------------------------
    # STDERR READER
    # --------------------------------------------------------

    def _stderr_reader(self):

        for raw_line in self.process.stderr:

            line = raw_line.strip()

            if line:

                print(
                    "[CODEX]",
                    line,
                    file=sys.stderr
                )


    # --------------------------------------------------------
    # SEND
    # --------------------------------------------------------

    def _send(self, message):

        if (
            self.process is None
            or
            self.process.poll() is not None
        ):

            raise RuntimeError(
                "Codex App Server is not running."
            )


        self.process.stdin.write(
            json.dumps(message)
            + "\n"
        )


        self.process.stdin.flush()


    # --------------------------------------------------------
    # NOTIFICATION
    # --------------------------------------------------------

    def notify(
        self,
        method,
        params=None
    ):

        message = {
            "method": method
        }


        if params is not None:

            message["params"] = (
                params
            )


        self._send(
            message
        )


    # --------------------------------------------------------
    # RPC
    # --------------------------------------------------------

    def rpc(
        self,
        method,
        params=None,
        timeout=15
    ):

        request_id = (
            self.request_id
        )


        self.request_id += 1


        message = {
            "id": request_id,
            "method": method
        }


        if params is not None:

            message["params"] = (
                params
            )


        self._send(
            message
        )


        deadline = (
            time.monotonic()
            +
            timeout
        )


        while True:

            remaining = (
                deadline
                -
                time.monotonic()
            )


            if remaining <= 0:

                raise TimeoutError(
                    f"Codex RPC timeout: {method}"
                )


            try:

                response = (
                    self.messages.get(
                        timeout=remaining
                    )
                )


            except queue.Empty:

                raise TimeoutError(
                    f"Codex RPC timeout: {method}"
                )


            # Notifications may appear between replies.

            if (
                response.get("id")
                !=
                request_id
            ):

                continue


            if "error" in response:

                raise RuntimeError(
                    f"{method}: "
                    f"{response['error']}"
                )


            return response.get(
                "result",
                {}
            )


    # --------------------------------------------------------
    # ACCOUNT TOKEN USAGE
    # --------------------------------------------------------

    def get_token_usage(self):

        return self.rpc(
            "account/usage/read",
            timeout=15
        )


    # --------------------------------------------------------
    # ACCOUNT RATE LIMITS
    # --------------------------------------------------------

    def get_rate_limits(self):

        return self.rpc(
            "account/rateLimits/read",
            timeout=15
        )


    # --------------------------------------------------------
    # STOP
    # --------------------------------------------------------

    def stop(self):

        if self.process is None:
            return


        try:

            self.process.terminate()

        except Exception:

            pass


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


    five_hour = None
    weekly = None


    if rate_result is not None:

        five_hour, weekly = (
            extract_windows(
                rate_result
            )
        )


    return {

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


# ============================================================
# SERIAL OUTPUT
# ============================================================

def send_line(
    device,
    line
):

    message = (
        line
        +
        "\n"
    ).encode(
        "utf-8"
    )


    device.write(
        message
    )


    device.flush()


# ============================================================
# RUN QUARK DEVICE
# ============================================================

def run_device(
    port,
    codex
):

    print(
        f"[QUARK] Connecting to {port}"
    )


    with serial.Serial(
        port,
        BAUD,
        timeout=0.05,
        write_timeout=2
    ) as device:


        # Opening CDC may reset the ESP.
        # Let logo + boot sound finish.

        time.sleep(3.5)


        print(
            "[QUARK] Connected"
        )


        next_system = 0
        next_codex = 0


        while True:

            now = (
                time.monotonic()
            )


            # ============================================
            # READ COMMANDS FROM ESP32
            # ============================================

            while device.in_waiting:

                raw = (
                    device.readline()
                )


                try:

                    line = (
                        raw
                        .decode(
                            "utf-8",
                            errors="ignore"
                        )
                        .strip()
                    )

                except Exception:

                    line = ""


                if not line:
                    continue


                print(
                    "[ESP]",
                    line
                )


                if (
                    line == "REFRESH|AI"
                    or
                    line == "MODE|AI"
                ):

                    next_codex = 0


                elif (
                    line == "REFRESH|SYS"
                    or
                    line == "MODE|SYS"
                ):

                    next_system = 0


            # ============================================
            # SYSTEM DATA
            # ============================================

            if (
                now >= next_system
            ):

                try:

                    data = (
                        get_system_data()
                    )


                    line = (
                        f"SYS"
                        f"|CPU={data['cpu']}"
                        f"|GPU={data['gpu']}"
                        f"|RAM={data['ram']}"
                        f"|TEMP={data['temp']}"
                    )


                    send_line(
                        device,
                        line
                    )


                    print(
                        "[SYSTEM]",
                        line
                    )


                except Exception as exc:

                    print(
                        "[SYSTEM ERROR]",
                        exc
                    )


                    send_line(
                        device,
                        "ERR|SYS"
                    )


                next_system = (
                    now
                    +
                    SYSTEM_INTERVAL
                )


            # ============================================
            # CODEX DATA
            # ============================================

            if (
                now >= next_codex
            ):

                if codex is None:

                    send_line(
                        device,
                        "ERR|AI"
                    )


                else:

                    try:

                        data = (
                            get_codex_data(
                                codex
                            )
                        )


                        line = (
                            f"AI"
                            f"|TODAY={data['today']}"
                            f"|LIFE={data['lifetime']}"
                            f"|H5={data['five_remaining']}"
                            f"|WEEK={data['weekly_remaining']}"
                            f"|H5R={data['five_reset']}"
                            f"|WR={data['weekly_reset']}"
                        )


                        send_line(
                            device,
                            line
                        )


                        print(
                            "[AI]",
                            line
                        )


                    except Exception as exc:

                        print(
                            "[AI ERROR]",
                            exc
                        )


                        send_line(
                            device,
                            "ERR|AI"
                        )


                next_codex = (
                    now
                    +
                    CODEX_INTERVAL
                )


            time.sleep(
                0.05
            )


# ============================================================
# MAIN
# ============================================================

def main():

    print()
    print("==========================")
    print(" QUARK Companion")
    print("==========================")
    print()


    # ================================================
    # START CODEX APP SERVER
    # ================================================

    codex = None


    try:

        codex = (
            CodexAppServer()
        )


        codex.start()


    except Exception as exc:

        print(
            "[CODEX ERROR]",
            exc
        )


        print(
            "System monitoring will still work."
        )


        print(
            "AI mode will remain red until Codex works."
        )


        codex = None


    # ================================================
    # SERIAL RECONNECT LOOP
    # ================================================

    try:

        while True:

            port = (
                find_quark_port()
            )


            if port is None:

                print(
                    "[QUARK] ESP32-C3 not found. "
                    "Retrying..."
                )


                time.sleep(
                    2
                )


                continue


            try:

                run_device(
                    port,
                    codex
                )


            except SerialException as exc:

                print(
                    "[QUARK] Disconnected:",
                    exc
                )


                time.sleep(
                    2
                )


            except OSError as exc:

                print(
                    "[QUARK] USB error:",
                    exc
                )


                time.sleep(
                    2
                )


    except KeyboardInterrupt:

        print()
        print(
            "Stopping QUARK..."
        )


    finally:

        if codex:

            codex.stop()


if __name__ == "__main__":

    main()
