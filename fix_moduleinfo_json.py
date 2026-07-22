#!/usr/bin/env python3
"""Strip trailing commas from JUCE/Steinberg-generated moduleinfo.json (strict JSON)."""
import json
import re
import sys


def strip_trailing_commas(text: str) -> str:
    prev = None
    while prev != text:
        prev = text
        text = re.sub(r",(\s*[}\]])", r"\1", text)
    return text


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: fix_moduleinfo_json.py <path/to/moduleinfo.json>", file=sys.stderr)
        return 2
    path = sys.argv[1]
    with open(path, encoding="utf-8") as f:
        raw = f.read()
    fixed = strip_trailing_commas(raw)
    json.loads(fixed)
    if fixed != raw:
        with open(path, "w", encoding="utf-8", newline="\n") as f:
            f.write(fixed)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
