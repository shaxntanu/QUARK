import unittest
from datetime import date

import quark


class FormattingTests(unittest.TestCase):
    def test_compact_token_counts(self):
        self.assertEqual(quark.compact_tokens(999), "999")
        self.assertEqual(quark.compact_tokens(18_432), "18.4K")
        self.assertEqual(quark.compact_tokens(443_013_968), "443.0M")
        self.assertEqual(quark.compact_tokens(5_632_085_905), "5.63B")
        self.assertEqual(quark.compact_tokens(None), "--")
        self.assertEqual(quark.compact_tokens("not-a-number"), "--")

    def test_reset_countdown_formats_and_clamps(self):
        now = 1_000
        self.assertEqual(quark.format_eta(now + 42 * 60, now), "42m")
        self.assertEqual(quark.format_eta(now + 2 * 3600 + 14 * 60, now), "2h14m")
        self.assertEqual(quark.format_eta(now + 3 * 86400 + 4 * 3600, now), "3d4h")
        self.assertEqual(quark.format_eta(now - 1, now), "0m")
        self.assertEqual(quark.format_eta("bad", now), "--")

    def test_remaining_allowance_clamps_and_handles_missing_values(self):
        self.assertEqual(quark.remaining_percent({"usedPercent": 22}), "78")
        self.assertEqual(quark.remaining_percent({"usedPercent": 125}), "0")
        self.assertEqual(quark.remaining_percent({"usedPercent": -4}), "100")
        self.assertEqual(quark.remaining_percent({"usedPercent": None}), "--")
        self.assertEqual(quark.remaining_percent({"usedPercent": "bad"}), "--")
        self.assertEqual(quark.remaining_percent(None), "--")


class CodexParsingTests(unittest.TestCase):
    def test_limit_windows_are_identified_by_duration(self):
        response = {
            "rateLimits": {
                "primary": {"usedPercent": 22, "windowDurationMins": 10080},
                "secondary": {"usedPercent": 64, "windowDurationMins": 300},
            }
        }
        five_hour, weekly = quark.extract_windows(response)
        self.assertEqual(five_hour["usedPercent"], 64)
        self.assertEqual(weekly["usedPercent"], 22)

    def test_codex_limit_snapshot_is_selected(self):
        response = {
            "rateLimitsByLimitId": {
                "other": {"primary": {"windowDurationMins": 300}},
                "codex": {
                    "primary": {"usedPercent": 11, "windowDurationMins": 300},
                    "secondary": {"usedPercent": 35, "windowDurationMins": 10080},
                },
            }
        }
        five_hour, weekly = quark.extract_windows(response)
        self.assertEqual(five_hour["usedPercent"], 11)
        self.assertEqual(weekly["usedPercent"], 35)

    def test_malformed_rate_and_usage_structures_are_safe(self):
        self.assertEqual(quark.extract_windows(None), (None, None))
        self.assertIsNone(quark.extract_today_tokens(None))
        self.assertIsNone(quark.extract_lifetime_tokens({"summary": []}))
        self.assertEqual(quark.extract_lifetime_tokens({"summary": {"lifetimeTokens": "bad"}}), None)

    def test_today_bucket_uses_local_date_and_defaults_to_zero(self):
        today = date(2026, 10, 28)
        usage = {
            "dailyUsageBuckets": [
                {"startDate": "2026-10-27", "tokens": 900},
                {"startDate": "2026-10-28", "tokens": 443_013_968},
            ]
        }
        self.assertEqual(quark.extract_today_tokens(usage, today), 443_013_968)
        self.assertEqual(quark.extract_today_tokens({"dailyUsageBuckets": []}, today), 0)
        self.assertIsNone(quark.extract_today_tokens({"dailyUsageBuckets": "invalid"}, today))


class TelemetryTests(unittest.TestCase):
    def test_nvidia_smi_parses_sample_and_unavailable_values(self):
        self.assertEqual(quark.parse_nvidia_smi_output("42, 71\n"), ("42", "71"))
        self.assertEqual(quark.parse_nvidia_smi_output("N/A, [Not Supported]\n"), ("--", "--"))
        self.assertEqual(quark.parse_nvidia_smi_output("no data"), ("--", "--"))

    def test_serial_packets_have_expected_fields_and_safe_values(self):
        self.assertEqual(
            quark.format_ai_packet({
                "today": "443.0M",
                "lifetime": "5.63B",
                "five_remaining": "78",
                "weekly_remaining": "36",
                "five_reset": "2h14m",
                "weekly_reset": "3d4h",
            }),
            "AI|TODAY=443.0M|LIFE=5.63B|H5=78|WEEK=36|H5R=2h14m|WR=3d4h",
        )
        self.assertEqual(
            quark.format_system_packet({"cpu": 42, "gpu": "--", "ram": 57, "temp": "--"}),
            "SYS|CPU=42|GPU=--|RAM=57|TEMP=--",
        )
        self.assertIn("TODAY=--", quark.format_ai_packet({"today": "bad|field"}))
        self.assertEqual(quark.parse_esp_command("REFRESH|SYS\r\n"), "REFRESH|SYS")
        self.assertIsNone(quark.parse_esp_command("untrusted random text"))


if __name__ == "__main__":
    unittest.main()
