#!/usr/bin/env python3
"""Fix mmTr(foo) missing quotes -> mmTr("foo")."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FILES = [
    ROOT / "FaceAnalyseManager.cpp",
    ROOT / "BackupManager.cpp",
    ROOT / "cameraclient.cpp",
    ROOT / "MM3DManager.cpp",
]

# mmTr( not followed by " — broken by earlier script
PAT = re.compile(r'mmTr\(([^")]+)\)')


def fix(text: str) -> str:
    def repl(m: re.Match[str]) -> str:
        inner = m.group(1)
        return f'mmTr("{inner}")'

    return PAT.sub(repl, text)


def main() -> int:
    for path in FILES:
        if not path.exists():
            continue
        text = path.read_text(encoding="utf-8")
        fixed = fix(text)
        if fixed != text:
            path.write_text(fixed, encoding="utf-8")
            print("repaired", path.name)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
