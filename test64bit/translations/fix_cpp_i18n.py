#!/usr/bin/env python3
"""Replace user-facing QStringLiteral / raw Chinese strings in C++ with mmTr()."""
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


def has_cjk(s: str) -> bool:
    return any("\u4e00" <= c <= "\u9fff" for c in s)


def ensure_include(text: str) -> str:
    if '#include "MmTr.h"' in text:
        return text
    lines = text.splitlines(keepends=True)
    insert_at = 0
    for i, ln in enumerate(lines):
        if ln.startswith("#include"):
            insert_at = i + 1
    lines.insert(insert_at, '#include "MmTr.h"\n')
    return "".join(lines)


def fix_file(path: Path) -> bool:
    text = path.read_text(encoding="utf-8")
    original = text

    def qrepl(m: re.Match[str]) -> str:
        inner = m.group(1)
        if has_cjk(inner):
            return f'mmTr("{inner}")'
        return m.group(0)

    text = re.sub(r'QStringLiteral\("((?:\\.|[^"\\])*)"\)', qrepl, text)

    def emit_repl(m: re.Match[str]) -> str:
        s = m.group(2)
        if has_cjk(s):
            return f'{m.group(1)}mmTr("{s}")'
        return m.group(0)

    text = re.sub(
        r'(emit\s+(?:finished|errorMessage|openFinished|previewOpenFailed)\([^,]+,\s*)'
        r'"((?:\\.|[^"\\])*)"',
        emit_repl,
        text,
    )

    # openFinished(ok, ok ? QString() : QStringLiteral(...)) already handled by qrepl

    # struct initializer labels in cameraclient: "RGB 快门"
    text = re.sub(
        r'(\{\s*"[^"]+",\s*)"((?:\\.|[^"\\])*)"',
        lambda m: f'{m.group(1)}mmTr("{m.group(2)}")' if has_cjk(m.group(2)) else m.group(0),
        text,
    )

    if text != original:
        text = ensure_include(text)
        path.write_text(text, encoding="utf-8")
        return True
    return False


def main() -> int:
    n = 0
    for path in FILES:
        if path.exists() and fix_file(path):
            print("updated", path.name)
            n += 1
    print(f"done, {n} cpp files")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
