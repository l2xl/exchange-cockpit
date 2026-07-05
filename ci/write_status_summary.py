#!/usr/bin/env python3
# XCockpit
# Copyright (c) 2026 l2xl (l2xl/at/proton.me)
# Distributed under the Intellectual Property Reserve License, v2 (IPRL)

"""Write a compact requirements-status table to the GitHub Actions job summary."""

import json
import os
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def render_summary(report):
    counts = {}
    for entry in report.values():
        counts[entry["status"]] = counts.get(entry["status"], 0) + 1
    lines = ["# Requirements status", "", "| Status | Count |", "|---|---|"]
    for status in ("test_passed", "test_failed", "partially_implemented", "not_implemented"):
        if status in counts:
            lines.append(f"| {status} | {counts[status]} |")
    lines.append("")
    lines.append("Full navigable tree with per-test logs: see the 'Requirements Status' check run on this commit.")
    return "\n".join(lines) + "\n"


def main():
    report = json.loads((ROOT / "req_status.json").read_text())
    summary = render_summary(report)
    summary_path = os.environ.get("GITHUB_STEP_SUMMARY")
    if summary_path:
        with open(summary_path, "a", encoding="utf-8") as f:
            f.write(summary)
    else:
        print(summary)
    return 0


if __name__ == "__main__":
    sys.exit(main())
