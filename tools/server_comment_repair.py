#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import re
import sys
from dataclasses import dataclass
from pathlib import Path


TEXT_SUFFIXES = {
    ".h",
    ".hpp",
    ".hh",
    ".hxx",
    ".c",
    ".cc",
    ".cpp",
    ".cxx",
    ".inl",
    ".ipp",
}

COMMENT_RE = re.compile(
    r"//[^\n]*|/\*.*?\*/",
    re.DOTALL,
)


@dataclass
class FileDecodeResult:
    path: Path
    encoding: str
    text: str


def iter_source_files(root: Path):
    for path in sorted(root.rglob("*")):
        if not path.is_file():
            continue
        if path.suffix.lower() not in TEXT_SUFFIXES:
            continue
        yield path


def decode_source(path: Path) -> FileDecodeResult | None:
    data = path.read_bytes()

    for encoding in ("utf-8", "utf-8-sig"):
        try:
            return FileDecodeResult(path=path, encoding=encoding, text=data.decode(encoding))
        except UnicodeDecodeError:
            pass

    for encoding in ("gb18030", "cp936", "gbk"):
        try:
            return FileDecodeResult(path=path, encoding=encoding, text=data.decode(encoding))
        except UnicodeDecodeError:
            pass

    return None


def list_candidates(root: Path) -> int:
    count = 0
    for path in iter_source_files(root):
        result = decode_source(path)
        if result is None:
            print(f"UNDECODABLE\t{path}")
            count += 1
            continue

        if result.encoding != "utf-8":
            print(f"NON_UTF8\t{path}\t{result.encoding}")
            count += 1

    return count


def extract_comments(root: Path, output: Path) -> int:
    rows = []

    for path in iter_source_files(root):
        result = decode_source(path)
        if result is None:
            continue

        for match in COMMENT_RE.finditer(result.text):
            comment = match.group(0)
            if not contains_non_ascii(comment):
                continue

            line = result.text.count("\n", 0, match.start()) + 1
            rows.append(
                {
                    "path": str(path),
                    "line": str(line),
                    "encoding": result.encoding,
                    "original_comment": normalize_comment(comment),
                    "english_comment": "",
                }
            )

    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open("w", newline="", encoding="utf-8") as fp:
        writer = csv.DictWriter(
            fp,
            fieldnames=[
                "path",
                "line",
                "encoding",
                "original_comment",
                "english_comment",
            ],
        )
        writer.writeheader()
        writer.writerows(rows)

    return len(rows)


def apply_translations(mapping_csv: Path, write: bool) -> int:
    by_file: dict[Path, list[tuple[str, str]]] = {}

    with mapping_csv.open("r", newline="", encoding="utf-8") as fp:
        reader = csv.DictReader(fp)
        for row in reader:
            original = (row.get("original_comment") or "").strip()
            english = (row.get("english_comment") or "").strip()
            if not original or not english:
                continue

            path = Path(row["path"])
            by_file.setdefault(path, []).append((original, english))

    changed_files = 0
    for path, replacements in sorted(by_file.items()):
        result = decode_source(path)
        if result is None:
            print(f"SKIP\t{path}\tunable to decode", file=sys.stderr)
            continue

        text = result.text
        updated = text
        applied = 0
        for original, english in replacements:
            replacement = rebuild_comment(original, english)
            if original in updated:
                updated = updated.replace(original, replacement)
                applied += 1

        if updated == text:
            continue

        changed_files += 1
        action = "WRITE" if write else "DRYRUN"
        print(f"{action}\t{path}\t{applied}")

        if write:
            path.write_text(updated, encoding="utf-8", newline="\n")

    return changed_files


def rewrite_utf8(root: Path, write: bool) -> int:
    changed = 0
    for path in iter_source_files(root):
        result = decode_source(path)
        if result is None:
            print(f"SKIP\t{path}\tunable to decode", file=sys.stderr)
            continue

        if result.encoding in ("utf-8", "utf-8-sig"):
            continue

        changed += 1
        action = "WRITE" if write else "DRYRUN"
        print(f"{action}\t{path}\t{result.encoding}->utf-8")
        if write:
            path.write_text(result.text, encoding="utf-8", newline="\n")

    return changed


def repair_mojibake_comments(root: Path, write: bool) -> int:
    changed = 0
    for path in iter_source_files(root):
        result = decode_source(path)
        if result is None:
            continue

        updated = COMMENT_RE.sub(_repair_comment_match, result.text)
        if updated == result.text:
            continue

        changed += 1
        action = "WRITE" if write else "DRYRUN"
        print(f"{action}\t{path}\tcomment-mojibake")
        if write:
            path.write_text(updated, encoding="utf-8", newline="\n")

    return changed


def normalize_lf(root: Path, write: bool) -> int:
    changed = 0
    for path in iter_source_files(root):
        result = decode_source(path)
        if result is None:
            continue

        normalized = result.text.replace("\r\n", "\n").replace("\r", "\n")
        current = path.read_bytes()
        target = normalized.encode("utf-8")

        if current == target:
            continue

        changed += 1
        action = "WRITE" if write else "DRYRUN"
        print(f"{action}\t{path}\tnormalize-lf")
        if write:
            path.write_bytes(target)

    return changed


def contains_non_ascii(text: str) -> bool:
    return any(ord(ch) > 127 for ch in text)


def contains_cjk(text: str) -> bool:
    return any("\u4e00" <= ch <= "\u9fff" for ch in text)


def normalize_comment(comment: str) -> str:
    return comment.replace("\r\n", "\n").replace("\r", "\n")


def rebuild_comment(original_comment: str, english_comment: str) -> str:
    if original_comment.startswith("//"):
        return f"// {english_comment}"

    lines = original_comment.splitlines()
    if len(lines) == 1:
        return f"/* {english_comment} */"

    rebuilt = [lines[0]]
    middle_count = max(len(lines) - 2, 1)
    english_lines = [english_comment] + [""] * (middle_count - 1)

    for index, line in enumerate(lines[1:-1]):
        stripped = line.lstrip()
        prefix_len = len(line) - len(stripped)
        prefix = line[:prefix_len]
        marker = "* " if stripped.startswith("*") else ""
        rebuilt.append(f"{prefix}{marker}{english_lines[index]}")

    rebuilt.append(lines[-1])
    return "\n".join(rebuilt)


def _repair_comment_match(match: re.Match[str]) -> str:
    comment = match.group(0)
    repaired = repair_comment_mojibake(comment)
    return repaired


def repair_comment_mojibake(comment: str) -> str:
    if contains_cjk(comment):
        return comment

    if not any(0x00C0 <= ord(ch) <= 0x00FF for ch in comment):
        return comment

    try:
        repaired = comment.encode("latin1").decode("gb18030")
    except UnicodeError:
        return comment

    if contains_cjk(repaired):
        return repaired

    return comment


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Scan and repair legacy comment encodings under kbe/src/server."
    )
    parser.add_argument(
        "--root",
        type=Path,
        default=Path("kbe/src/server"),
        help="Root directory to process. Default: kbe/src/server",
    )

    subparsers = parser.add_subparsers(dest="command", required=True)

    subparsers.add_parser("list", help="List non-UTF-8 source files under the root")

    extract_parser = subparsers.add_parser(
        "extract-comments",
        help="Extract non-ASCII comments into a CSV for manual translation",
    )
    extract_parser.add_argument(
        "--output",
        type=Path,
        default=Path("reports/server-comment-translation.csv"),
        help="CSV output path",
    )

    apply_parser = subparsers.add_parser(
        "apply",
        help="Apply english_comment values from a CSV back into source comments",
    )
    apply_parser.add_argument(
        "--mapping",
        type=Path,
        default=Path("reports/server-comment-translation.csv"),
        help="CSV mapping path",
    )
    apply_parser.add_argument(
        "--write",
        action="store_true",
        help="Actually rewrite files. Without this flag, runs in dry-run mode.",
    )

    rewrite_parser = subparsers.add_parser(
        "rewrite-utf8",
        help="Rewrite decodable legacy source files to UTF-8 without changing comment text",
    )
    rewrite_parser.add_argument(
        "--write",
        action="store_true",
        help="Actually rewrite files. Without this flag, runs in dry-run mode.",
    )

    repair_parser = subparsers.add_parser(
        "repair-mojibake-comments",
        help="Repair mojibake comments that can be reversed via latin1->gb18030",
    )
    repair_parser.add_argument(
        "--write",
        action="store_true",
        help="Actually rewrite files. Without this flag, runs in dry-run mode.",
    )

    normalize_parser = subparsers.add_parser(
        "normalize-lf",
        help="Rewrite decodable source files as UTF-8 with LF line endings",
    )
    normalize_parser.add_argument(
        "--write",
        action="store_true",
        help="Actually rewrite files. Without this flag, runs in dry-run mode.",
    )

    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()

    if args.command == "list":
        count = list_candidates(args.root)
        print(f"TOTAL\t{count}")
        return 0

    if args.command == "extract-comments":
        count = extract_comments(args.root, args.output)
        print(f"EXTRACTED\t{count}\t{args.output}")
        return 0

    if args.command == "apply":
        count = apply_translations(args.mapping, args.write)
        print(f"CHANGED\t{count}")
        return 0

    if args.command == "rewrite-utf8":
        count = rewrite_utf8(args.root, args.write)
        print(f"CHANGED\t{count}")
        return 0

    if args.command == "repair-mojibake-comments":
        count = repair_mojibake_comments(args.root, args.write)
        print(f"CHANGED\t{count}")
        return 0

    if args.command == "normalize-lf":
        count = normalize_lf(args.root, args.write)
        print(f"CHANGED\t{count}")
        return 0

    parser.error(f"unsupported command: {args.command}")
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
