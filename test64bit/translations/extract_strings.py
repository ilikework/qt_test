#!/usr/bin/env python3
"""Extract qsTr/mmTr strings from sources and merge into gen_qm TABLE."""
from __future__ import annotations

import ast
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

PATTERNS = [
    re.compile(r'qsTr\("((?:\\.|[^"\\])*)"\)'),
    re.compile(r'mmTr\("((?:\\.|[^"\\])*)"\)'),
    re.compile(r'QCoreApplication::translate\("", "((?:\\.|[^"\\])*)"\)'),
]


def collect_strings() -> set[str]:
    found: set[str] = set()
    for path in list((ROOT / "QMLContent").rglob("*.qml")) + list(ROOT.glob("*.cpp")):
        if "del" in path.parts:
            continue
        text = path.read_text(encoding="utf-8", errors="ignore")
        for pat in PATTERNS:
            for m in pat.finditer(text):
                s = m.group(1).encode().decode("unicode_escape") if "\\" in m.group(1) else m.group(1)
                found.add(s)
    return found


def load_table_py() -> dict:
    gen = ROOT / "translations" / "gen_qm.py"
    ns: dict = {}
    code = gen.read_text(encoding="utf-8")
    exec(compile(code, str(gen), "exec"), ns, ns)
    return dict(ns.get("TABLE", {}))


def main() -> None:
    existing = load_table_py()
    found = collect_strings()
    missing = sorted(s for s in found if s not in existing)
    print(f"found {len(found)} unique, missing {len(missing)}")
    for s in missing:
        print("MISSING:", repr(s))


if __name__ == "__main__":
    main()
