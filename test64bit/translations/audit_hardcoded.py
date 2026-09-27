#!/usr/bin/env python3
"""Find hardcoded Chinese in QML not passed through translateText."""
from __future__ import annotations

import re
from pathlib import Path

CJK = re.compile(r"[\u4e00-\u9fff\u3000-\u303f\uff00-\uffef]+")
SKIP = re.compile(
    r"translateText|//|font\.|source:|image/|\.png|\.svg|\.qml|import |console\.|#"
)

qml_root = Path(__file__).resolve().parent.parent / "QMLContent"
hits: list[tuple[str, int, str]] = []
for path in sorted(qml_root.rglob("*.qml")):
    for i, line in enumerate(path.read_text(encoding="utf-8", errors="ignore").splitlines(), 1):
        if SKIP.search(line):
            continue
        for m in CJK.finditer(line):
            frag = m.group().strip()
            if len(frag) >= 2:
                hits.append((str(path.relative_to(qml_root)), i, line.strip()[:120]))

print(f"hardcoded CJK lines: {len(hits)}")
for rel, ln, text in hits[:80]:
    print(f"{rel}:{ln}: {text}")
if len(hits) > 80:
    print(f"... {len(hits)-80} more")
