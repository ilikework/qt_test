#!/usr/bin/env python3
"""Wrap visible Chinese QML string literals with qsTr()."""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / "QMLContent"
SKIP = {"del", "testWindow.qml", "testWindow2.qml", "TestDlg.qml", "RunPreRecord.qml", "main.qml"}


def has_cjk(s: str) -> bool:
    return any("\u4e00" <= c <= "\u9fff" for c in s)


def already_wrapped(line: str, idx: int) -> bool:
    return "qsTr(" in line[max(0, idx - 12) : idx]


def wrap_line(line: str) -> str:
    if "qsTr(" in line or "console." in line or line.strip().startswith("//"):
        return line

    props = (
        "text",
        "placeholderText",
        "boxTitle",
        "boxMessage",
        "btnLabel",
        "title",
        "toolTip",
    )

    out = line
    for prop in props:
        pattern = re.compile(
            rf"({re.escape(prop)}:\s*)\"((?:\\.|[^\"\\])*)\"",
            re.UNICODE,
        )

        def repl(m: re.Match[str]) -> str:
            prefix, s = m.group(1), m.group(2)
            if not has_cjk(s):
                return m.group(0)
            if already_wrapped(line, m.start()):
                return m.group(0)
            return f'{prefix}qsTr("{s}")'

        out = pattern.sub(repl, out)

    # ToolTip.text:
    out = re.sub(
        r"(ToolTip\.text:\s*)\"((?:\\.|[^\"\\])*)\"",
        lambda m: (
            m.group(0)
            if not has_cjk(m.group(2)) or already_wrapped(line, m.start())
            else f'{m.group(1)}qsTr("{m.group(2)}")'
        ),
        out,
    )

    # inline array elements: { text: "中文", ... }
    out = re.sub(
        r"(\{\s*text:\s*)\"((?:\\.|[^\"\\])*)\"",
        lambda m: (
            m.group(0)
            if not has_cjk(m.group(2)) or "qsTr(" in m.group(0)
            else f'{m.group(1)}qsTr("{m.group(2)}")'
        ),
        out,
    )

    return out


def process_file(path: Path) -> bool:
    text = path.read_text(encoding="utf-8")
    lines = text.splitlines(keepends=True)
    new_lines = [wrap_line(ln) for ln in lines]
    new_text = "".join(new_lines)
    if new_text != text:
        path.write_text(new_text, encoding="utf-8")
        return True
    return False


def main() -> int:
    changed = 0
    for path in sorted(ROOT.rglob("*.qml")):
        if any(part in SKIP for part in path.parts):
            continue
        if path.name in SKIP:
            continue
        if process_file(path):
            print("updated", path.relative_to(ROOT.parent))
            changed += 1
    print(f"done, {changed} files changed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
