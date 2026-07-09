#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from __future__ import annotations

import difflib
import importlib.util
import os
import sys
import tempfile
from pathlib import Path, PurePosixPath

from chm.chm import CHMFile
from chm import chmlib


ROOT = Path(__file__).resolve().parents[1]
DOCS_ROOT = ROOT / "docs"
CHM_FILE = ROOT / "docs" / "api" / "kbengine" / "kbengine_api(cn).chm"
PDF_FILE = ROOT / "docs" / "WebConsole_Guide(cn).pdf"


def load_rebuild_module():
    spec = importlib.util.spec_from_file_location("rebuild_docs", ROOT / "tools" / "rebuild_docs.py")
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def extract_chm_html(chm_path: Path, output_root: Path) -> None:
    book = CHMFile()
    if not book.LoadCHM(str(chm_path)):
        raise RuntimeError(f"无法打开 CHM 文件: {chm_path}")

    html_paths: list[str] = []

    def enum(_, ui, paths):
        path = ui.path.decode("utf-8", errors="replace")
        if path.endswith(".html"):
            paths.append(path)
        return chmlib.CHM_ENUMERATOR_CONTINUE

    chmlib.chm_enumerate(book.file, chmlib.CHM_ENUMERATE_ALL, enum, html_paths)

    for path in html_paths:
        result, unit = book.ResolveObject(path.encode())
        if result != 0 or unit is None:
            continue
        _, data = book.RetrieveObject(unit)
        target = output_root / path.lstrip("/")
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)

    book.CloseCHM()


def build_source_map(mod, chm_root: Path) -> tuple[list, dict[str, PurePosixPath]]:
    modules = mod.discover_modules(chm_root)
    source_to_output_map: dict[str, PurePosixPath] = {
        "basetypes.html": PurePosixPath("basetypes.md"),
        "keywords.html": PurePosixPath("keywords.md"),
    }

    for module in modules:
        source_to_output_map[module.intro_source.lower()] = PurePosixPath(f"{module.name}/index.md")
        if module.module_source:
            source_to_output_map[module.module_source.lower()] = PurePosixPath(f"{module.name}/KBEngine.md")
        for class_page in module.classes:
            source_to_output_map[f"{module.name}/classes/{class_page.lower()}"] = PurePosixPath(
                f"{module.name}/{Path(class_page).stem}.md"
            )

    return modules, source_to_output_map


def verify_api(mod, temp_root: Path) -> tuple[int, list[str]]:
    chm_root = temp_root / "chm"
    md_root = temp_root / "md"
    chm_root.mkdir(parents=True, exist_ok=True)
    md_root.mkdir(parents=True, exist_ok=True)

    extract_chm_html(CHM_FILE, chm_root)
    modules, source_to_output_map = build_source_map(mod, chm_root)
    converter = mod.HtmlConverter(chm_root, md_root, source_to_output_map)

    converter.convert_page("basetypes.html", "basetypes.md")
    converter.convert_page("keywords.html", "keywords.md")
    for module in modules:
        converter.convert_page(module.intro_source, f"{module.name}/index.md")
        if module.module_source:
            converter.convert_page(module.module_source, f"{module.name}/KBEngine.md")
        for class_page in module.classes:
            converter.convert_page(f"{module.name}/Classes/{class_page}", f"{module.name}/{Path(class_page).stem}.md")

    mismatches: list[str] = []
    generated_files = sorted(md_root.rglob("*.md"))
    for path in generated_files:
        rel = path.relative_to(md_root)
        current = (DOCS_ROOT / "api" / "kbengine" / rel).read_text(encoding="utf-8")
        generated = path.read_text(encoding="utf-8")
        if current != generated:
            mismatches.append(str(rel))

    return len(generated_files), mismatches


def verify_webconsole(mod) -> bool:
    current = (DOCS_ROOT / "guide" / "webconsole.md").read_text(encoding="utf-8")
    generated = mod.build_pdf_markdown(PDF_FILE)
    return current == generated


def main() -> int:
    if not CHM_FILE.exists():
        print(f"未找到 CHM 文件: {CHM_FILE}")
        return 1
    if not PDF_FILE.exists():
        print(f"未找到 PDF 文件: {PDF_FILE}")
        return 1

    mod = load_rebuild_module()
    with tempfile.TemporaryDirectory(prefix="kbengine_verify_") as temp_dir:
        api_total, api_mismatches = verify_api(mod, Path(temp_dir))
    webconsole_match = verify_webconsole(mod)

    print(f"API pages compared: {api_total}")
    print(f"API exact matches: {api_total - len(api_mismatches)}")
    print(f"API mismatches: {len(api_mismatches)}")
    for rel in api_mismatches:
        print(f"- {rel}")

    print(f"WebConsole exact match: {webconsole_match}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
