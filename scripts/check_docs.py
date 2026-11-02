"""Check repository text for broken local Markdown links and private paths."""

from __future__ import annotations

import os
import re
import sys
from pathlib import Path
from urllib.parse import unquote, urlsplit


ROOT = Path(__file__).resolve().parents[1]
SKIP_DIRS = {".git", ".venv", "venv", "build", "dist", "node_modules", "__pycache__"}
MARKDOWN_SUFFIXES = {".md", ".markdown"}
LINK_PATTERN = re.compile(
    r"(?<!!)\[[^\]]+\]\((?:<([^>]+)>|([^\s)]+))(?:\s+[^)]*)?\)"
)
WINDOWS_PROFILE_PATH = re.compile(
    r"(?i)(?:[a-z]:[\\/]+)" + "Users" + r"[\\/]+[^\\/\s\"'<>]+"
)
EM_DASH = chr(0x2014)


def repository_text_files():
    for current, directories, filenames in os.walk(ROOT):
        directories[:] = sorted(directory for directory in directories if directory not in SKIP_DIRS)
        for filename in sorted(filenames):
            path = Path(current) / filename
            try:
                content = path.read_bytes()
            except OSError:
                continue
            if b"\0" in content:
                continue
            try:
                text = content.decode("utf-8-sig")
            except UnicodeDecodeError:
                continue
            yield path, text


def local_markdown_target(source: Path, target: str):
    target = unquote(target.strip())
    if not target or target.startswith("#"):
        return None
    parsed = urlsplit(target)
    if parsed.scheme or parsed.netloc:
        return None
    relative_path = parsed.path
    if not relative_path:
        return None
    candidate = (source.parent / relative_path).resolve()
    try:
        candidate.relative_to(ROOT)
    except ValueError:
        return candidate
    return candidate


def main() -> int:
    errors = []
    markdown_files = []
    scanned_files = 0

    for path, text in repository_text_files():
        scanned_files += 1
        relative = path.relative_to(ROOT).as_posix()
        if EM_DASH in text:
            line = text[: text.index(EM_DASH)].count("\n") + 1
            errors.append(f"{relative}:{line}: em dash is not permitted")
        for line_number, line in enumerate(text.splitlines(), start=1):
            if WINDOWS_PROFILE_PATH.search(line):
                errors.append(f"{relative}:{line_number}: account-specific Windows path found")
        if path.suffix.lower() not in MARKDOWN_SUFFIXES:
            continue
        markdown_files.append(path)
        for match in LINK_PATTERN.finditer(text):
            target = match.group(1) or match.group(2) or ""
            candidate = local_markdown_target(path, target)
            if candidate is not None and not candidate.exists():
                errors.append(
                    f"{relative}: local Markdown target does not exist: {target}"
                )

    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        return 1

    print(
        f"Documentation checks passed: {scanned_files} text files scanned, "
        f"{len(markdown_files)} Markdown files checked."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
