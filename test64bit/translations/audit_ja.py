#!/usr/bin/env python3
"""Audit Japanese translation coverage."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent
GEN = (ROOT / "gen_qm.py").read_text(encoding="utf-8")
start = GEN.index("TABLE:")
end = GEN.index("\nLOCALES", start)
ns: dict = {}
exec(compile(GEN[start:end], "gen_qm.py", "exec"), ns)
TABLE: dict[str, dict[str, str]] = ns["TABLE"]

HIRAGANA = re.compile(r"[\u3040-\u309f]")
KATAKANA = re.compile(r"[\u30a0-\u30ff]")


def has_kana(s: str) -> bool:
    return bool(HIRAGANA.search(s) or KATAKANA.search(s))


def is_cjk(s: str) -> bool:
    return any("\u4e00" <= c <= "\u9fff" for c in s)


# QML translateText keys
pat = re.compile(r'translateText\s*\(\s*"([^"]*)"')
missing_qml: set[str] = set()
qml_keys: set[str] = set()
for qml in (ROOT.parent / "QMLContent").rglob("*.qml"):
    text = qml.read_text(encoding="utf-8", errors="ignore")
    for m in pat.finditer(text):
        qml_keys.add(m.group(1))
        if m.group(1) not in TABLE:
            missing_qml.add(m.group(1))

# C++ mmTr keys (rough)
mmtr_pat = re.compile(r'mmTr\("([^"]+)"\)')
missing_cpp: set[str] = set()
for cpp in (ROOT.parent).rglob("*.cpp"):
    text = cpp.read_text(encoding="utf-8", errors="ignore")
    for m in mmtr_pat.finditer(text):
        if m.group(1) not in TABLE:
            missing_cpp.add(m.group(1))

no_kana_zh_keys: list[str] = []
for k, row in sorted(TABLE.items()):
    zh = row.get("zh_CN", "")
    ja = row.get("ja", "")
    if is_cjk(zh) and ja == zh and k == zh:
        no_kana_zh_keys.append(k)
    elif is_cjk(zh) and ja == zh and k != zh:
        no_kana_zh_keys.append(f"{k!r} -> {ja!r}")

print(f"TABLE size: {len(TABLE)}")
print(f"QML translateText keys: {len(qml_keys)}, missing: {len(missing_qml)}")
print(f"C++ mmTr keys missing: {len(missing_cpp)}")
print(f"ja==zh_CN (Chinese-looking, same kanji only): {len(no_kana_zh_keys)}")
print()
if missing_qml:
    print("=== Missing from TABLE (QML) ===")
    for s in sorted(missing_qml):
        print(repr(s))
if missing_cpp:
    print("=== Missing from TABLE (C++) ===")
    for s in sorted(missing_cpp):
        print(repr(s))
if no_kana_zh_keys:
    print("=== ja identical to zh (may look untranslated) ===")
    for s in no_kana_zh_keys[:100]:
        print(s)
    if len(no_kana_zh_keys) > 100:
        print(f"... and {len(no_kana_zh_keys) - 100} more")
