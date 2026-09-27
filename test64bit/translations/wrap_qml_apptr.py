#!/usr/bin/env python3
"""Replace qsTr() with appTranslator.translateText() for reliable empty-context lookup."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / "QMLContent"
QSTR = re.compile(r'qsTr\("((?:\\.|[^"\\])*)"\)')
PROP_LINE = re.compile(
    r'^(\s+)([A-Za-z_][\w]*): qsTr\("((?:\\.|[^"\\])*)"\)\s*$',
    re.MULTILINE,
)
RETURN_QSTR = re.compile(r'return qsTr\("((?:\\.|[^"\\])*)"\)')


def process(text: str) -> str:
    text = PROP_LINE.sub(
        r'\1\2: { var _ = appTranslator.revision; return appTranslator.translateText("\3") }',
        text,
    )
    text = RETURN_QSTR.sub(r'return appTranslator.translateText("\1")', text)
    text = QSTR.sub(r'appTranslator.translateText("\1")', text)
    return text


def main() -> int:
    n = 0
    for path in ROOT.rglob("*.qml"):
        if "del" in path.parts:
            continue
        original = path.read_text(encoding="utf-8")
        updated = process(original)
        if updated != original:
            path.write_text(updated, encoding="utf-8")
            print("updated", path.relative_to(ROOT))
            n += 1
    print(f"done, {n} files")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
