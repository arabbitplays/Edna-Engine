#!/usr/bin/env python3
"""Count lines of code across the project, excluding subprojects and build output."""

from __future__ import annotations

import argparse
from collections import defaultdict
from pathlib import Path

SOURCE_EXTENSIONS = {
    ".cpp", ".hpp", ".h", ".c",
    ".glsl", ".vert", ".frag", ".comp",
    ".rchit", ".rgen", ".rmiss",
    ".py", ".sh",
    ".nix",
    ".yaml", ".yml",
    ".build",
}

EXCLUDED_DIRS = {"subprojects", "buildDir", "build", ".git", "__pycache__", ".venv", "venv"}


def count_lines(path: Path) -> int:
    with path.open("rb") as f:
        return sum(1 for _ in f)


def collect(root: Path) -> dict[str, tuple[int, int]]:
    totals: dict[str, list[int]] = defaultdict(lambda: [0, 0])
    for path in root.rglob("*"):
        if not path.is_file():
            continue
        if any(part in EXCLUDED_DIRS for part in path.relative_to(root).parts):
            continue
        ext = path.suffix if path.suffix else path.name
        if ext not in SOURCE_EXTENSIONS:
            continue
        totals[ext][0] += 1
        totals[ext][1] += count_lines(path)
    return {ext: (files, lines) for ext, (files, lines) in totals.items()}


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("root", nargs="?", default=".", type=Path,
                        help="Project root (default: cwd)")
    args = parser.parse_args()

    root = args.root.resolve()
    stats = collect(root)

    print(f"LOC report for {root}")
    print(f"{'Extension':<10} {'Files':>8} {'Lines':>10}")
    print("-" * 30)
    total_files = total_lines = 0
    for ext in sorted(stats, key=lambda e: stats[e][1], reverse=True):
        files, lines = stats[ext]
        print(f"{ext:<10} {files:>8} {lines:>10}")
        total_files += files
        total_lines += lines
    print("-" * 30)
    print(f"{'TOTAL':<10} {total_files:>8} {total_lines:>10}")


if __name__ == "__main__":
    main()
