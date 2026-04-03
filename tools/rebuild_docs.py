#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from __future__ import annotations

import html
import os
import re
import shutil
import subprocess
import tempfile
from dataclasses import dataclass, field
from pathlib import Path, PurePosixPath
from typing import Iterable

import pdfplumber
from bs4 import BeautifulSoup, NavigableString, Tag


ROOT = Path(__file__).resolve().parents[1]
OUTPUT_DIR = ROOT / "docs-vuepress"
CHM_FILE = ROOT / "docs" / "api" / "kbengine_api(cn).chm"
PDF_FILE = ROOT / "docs" / "WebConsole_Guide(cn).pdf"
LOGO_OUTPUT = OUTPUT_DIR / ".vuepress" / "public" / "logo.png"
LOGO_GIT_PATH = "docs-vuepress/.vuepress/public/logo.png"

MODULE_ORDER = [
    "client",
    "cellapp",
    "baseapp",
    "loginapp",
    "dbmgr",
    "interfaces",
    "logger",
    "bots",
]

SKIP_CHM_PAGES = {
    "index.html",
    "keywords.html",
    "basetypes.html",
}

SCREENSHOT_PATTERN = re.compile(r"^\d+(?:-\d+)+")
ROMAN_SECTION_PATTERN = re.compile(r"^\(([一二三四五六七八九十]+)\)\s*(.+)$")
NUMBERED_PATTERN = re.compile(r"^(\d+)\.\s*(.+)$")
SUB_NUMBERED_PATTERN = re.compile(r"^(\d+)\)\s*(.+)$")


@dataclass
class ModuleInfo:
    name: str
    intro_source: str
    module_source: str | None = None
    classes: list[str] = field(default_factory=list)


class HtmlConverter:
    BLOCK_TAGS = {
        "div",
        "p",
        "pre",
        "table",
        "ul",
        "ol",
        "li",
        "h1",
        "h2",
        "h3",
        "h4",
        "h5",
        "h6",
        "dl",
        "dt",
        "dd",
        "blockquote",
    }

    def __init__(
        self,
        source_root: Path,
        output_root: Path,
        source_to_output_map: dict[str, PurePosixPath],
    ):
        self.source_root = source_root
        self.output_root = output_root
        self.source_to_output_map = source_to_output_map

    def convert_page(self, source_relative: str, output_relative: str) -> None:
        source_path = self.source_root / source_relative
        output_path = self.output_root / output_relative
        soup = BeautifulSoup(source_path.read_text(encoding="utf-8", errors="replace"), "html.parser")
        body = soup.body
        if body is None:
            raise ValueError(f"页面缺少 body: {source_relative}")

        self._strip_chrome(body)
        markdown = self._render_children(
            body,
            source_relative=PurePosixPath(source_relative),
            output_relative=PurePosixPath(output_relative),
        ).strip()
        markdown = self._post_process_markdown(markdown)

        output_path.parent.mkdir(parents=True, exist_ok=True)
        output_path.write_text(markdown + "\n", encoding="utf-8")

    def _strip_chrome(self, body: Tag) -> None:
        banner = body.find("table")
        if banner is not None:
            banner.decompose()

        for footer in body.find_all("p", class_="copyrightFooter"):
            footer.decompose()

        for hr in body.find_all("hr"):
            hr.decompose()

        for tag in body.find_all(["script", "style"]):
            tag.decompose()

    def _render_children(
        self,
        parent: Tag,
        *,
        source_relative: PurePosixPath,
        output_relative: PurePosixPath,
        base_heading_level: int = 0,
    ) -> str:
        blocks: list[str] = []
        inline_parts: list[str] = []
        pending_anchor: str | None = None

        def flush_inline() -> None:
            nonlocal inline_parts, pending_anchor
            text = self._normalize_inline("".join(inline_parts))
            inline_parts = []
            if not text:
                return
            if pending_anchor:
                text = f'<a id="{pending_anchor}"></a>\n\n{text}'
                pending_anchor = None
            blocks.append(text)

        for child in parent.children:
            if isinstance(child, NavigableString):
                text = self._normalize_text(str(child))
                if text:
                    inline_parts.append(text)
                continue

            if not isinstance(child, Tag):
                continue

            if child.name == "a" and child.get("name") and not child.get("href"):
                pending_anchor = child.get("name")
                continue

            if child.name == "br":
                inline_parts.append("\n")
                continue

            if child.name in self.BLOCK_TAGS:
                flush_inline()
                block = self._render_block(
                    child,
                    source_relative=source_relative,
                    output_relative=output_relative,
                    base_heading_level=base_heading_level,
                    pending_anchor=pending_anchor,
                )
                pending_anchor = None
                if block:
                    blocks.append(block)
                continue

            inline_parts.append(
                self._render_inline(
                    child,
                    source_relative=source_relative,
                    output_relative=output_relative,
                )
            )

        flush_inline()
        return "\n\n".join(block for block in blocks if block.strip())

    def _render_block(
        self,
        tag: Tag,
        *,
        source_relative: PurePosixPath,
        output_relative: PurePosixPath,
        base_heading_level: int,
        pending_anchor: str | None,
    ) -> str:
        if tag.name in {"h1", "h2", "h3", "h4", "h5", "h6"}:
            level = min(int(tag.name[1]) + base_heading_level, 6)
            text = self._normalize_inline(
                self._render_inline_children(
                    tag,
                    source_relative=source_relative,
                    output_relative=output_relative,
                )
            )
            if not text:
                return ""
            return self._prefix_anchor(f'{"#" * level} {text}', pending_anchor)

        if tag.name == "pre":
            code = tag.get_text("\n", strip=False).replace("\xa0", " ")
            code = code.strip("\n")
            if not code:
                return ""
            return self._prefix_anchor(f"```python\n{code}\n```", pending_anchor)

        if tag.name == "table":
            table_text = self._render_table(
                tag,
                source_relative=source_relative,
                output_relative=output_relative,
            )
            return self._prefix_anchor(table_text, pending_anchor)

        if tag.name in {"ul", "ol"}:
            list_text = self._render_list(
                tag,
                source_relative=source_relative,
                output_relative=output_relative,
            )
            return self._prefix_anchor(list_text, pending_anchor)

        if tag.name == "p" and "function_definition" in (tag.get("class") or []):
            heading = self._normalize_inline(
                self._render_inline_children(
                    tag,
                    source_relative=source_relative,
                    output_relative=output_relative,
                )
            )
            if not heading:
                return ""
            return self._prefix_anchor(f"### {heading}", pending_anchor)

        if tag.name == "p" and "function_definition" not in (tag.get("class") or []):
            paragraph = self._render_children(
                tag,
                source_relative=source_relative,
                output_relative=output_relative,
                base_heading_level=base_heading_level,
            )
            return self._prefix_anchor(paragraph, pending_anchor)

        if tag.name in {"div", "blockquote", "dl", "dt", "dd", "li"}:
            content = self._render_children(
                tag,
                source_relative=source_relative,
                output_relative=output_relative,
                base_heading_level=base_heading_level,
            )
            return self._prefix_anchor(content, pending_anchor)

        return ""

    def _render_list(
        self,
        tag: Tag,
        *,
        source_relative: PurePosixPath,
        output_relative: PurePosixPath,
    ) -> str:
        items: list[str] = []
        for li in tag.find_all("li", recursive=False):
            content = self._render_children(
                li,
                source_relative=source_relative,
                output_relative=output_relative,
            ).strip()
            if not content:
                continue
            content = content.replace("\n\n", "\n").replace("\n", " ")
            items.append(f"- {self._normalize_inline(content)}")
        return "\n".join(items)

    def _render_table(
        self,
        table: Tag,
        *,
        source_relative: PurePosixPath,
        output_relative: PurePosixPath,
    ) -> str:
        rows: list[list[str]] = []
        for tr in table.find_all("tr", recursive=False) or table.find_all("tr"):
            cells = tr.find_all(["th", "td"], recursive=False) or tr.find_all(["th", "td"])
            rendered_row: list[str] = []
            for cell in cells:
                cell_text = self._render_children(
                    cell,
                    source_relative=source_relative,
                    output_relative=output_relative,
                ).strip()
                cell_text = cell_text.replace("\n\n", "<br>").replace("\n", " ")
                rendered_row.append(self._normalize_inline(cell_text))
            if any(rendered_row):
                rows.append(rendered_row)

        if not rows:
            return ""

        column_count = max(len(row) for row in rows)
        normalized_rows = [row + [""] * (column_count - len(row)) for row in rows]

        if column_count == 1:
            return "\n".join(f"- {row[0]}" for row in normalized_rows if row[0])

        header = normalized_rows[0]
        if len(normalized_rows) == 1:
            return f"| {' | '.join(header)} |\n| {' | '.join(['---'] * column_count)} |"

        separator = ["---"] * column_count
        markdown_rows = [
            f"| {' | '.join(self._escape_table_cell(cell) for cell in header)} |",
            f"| {' | '.join(separator)} |",
        ]
        for row in normalized_rows[1:]:
            markdown_rows.append(
                f"| {' | '.join(self._escape_table_cell(cell) for cell in row)} |"
            )
        return "\n".join(markdown_rows)

    def _render_inline_children(
        self,
        tag: Tag,
        *,
        source_relative: PurePosixPath,
        output_relative: PurePosixPath,
    ) -> str:
        parts: list[str] = []
        for child in tag.children:
            if isinstance(child, NavigableString):
                parts.append(self._normalize_text(str(child)))
            elif isinstance(child, Tag):
                parts.append(
                    self._render_inline(
                        child,
                        source_relative=source_relative,
                        output_relative=output_relative,
                    )
                )
        return "".join(parts)

    def _render_inline(
        self,
        tag: Tag,
        *,
        source_relative: PurePosixPath,
        output_relative: PurePosixPath,
    ) -> str:
        if tag.name == "a":
            text = self._normalize_inline(
                self._render_inline_children(
                    tag,
                    source_relative=source_relative,
                    output_relative=output_relative,
                )
            )
            href = self._convert_href(tag.get("href", ""), source_relative, output_relative)
            if not text:
                return ""
            if not href:
                return text
            return f"[{text}]({href})"

        if tag.name == "code":
            text = self._normalize_inline(tag.get_text(" ", strip=True))
            return f"`{text}`" if text else ""

        if tag.name == "br":
            return "\n"

        if tag.name == "pre":
            code = tag.get_text("\n", strip=False).strip("\n")
            if not code:
                return ""
            return f"\n```python\n{code}\n```\n"

        if tag.name == "img":
            alt = tag.get("alt", "").strip() or "image"
            src = self._convert_href(tag.get("src", ""), source_relative, output_relative)
            return f"![{alt}]({src})" if src else ""

        return self._render_inline_children(
            tag,
            source_relative=source_relative,
            output_relative=output_relative,
        )

    def _convert_href(
        self,
        href: str,
        source_relative: PurePosixPath,
        output_relative: PurePosixPath,
    ) -> str:
        href = href.strip()
        if not href:
            return ""
        if href.startswith(("http://", "https://", "mailto:")):
            return href
        if href.startswith("#"):
            return href

        href_path, anchor = (href.split("#", 1) + [""])[:2]
        source_target = (source_relative.parent / href_path).as_posix()
        normalized_source_target = self._normalize_source_key(source_target)

        if normalized_source_target.endswith(".html"):
            target_relative = self.source_to_output_map.get(normalized_source_target)
            if target_relative is None:
                return ""
            output_parent = output_relative.parent
            relative_path = os.path.relpath(
                str(target_relative),
                str(output_parent),
            ).replace("\\", "/")
            if not relative_path.startswith("."):
                relative_path = f"./{relative_path}"
            return f"{relative_path}#{anchor}" if anchor else relative_path

        return href

    @staticmethod
    def _normalize_source_key(path: str) -> str:
        return str(PurePosixPath(path)).replace("\\", "/").lower()

    @staticmethod
    def _normalize_text(text: str) -> str:
        text = text.replace("\xa0", " ")
        text = re.sub(r"[ \t\r\f\v]+", " ", text)
        text = re.sub(r"\n\s*\n+", "\n", text)
        return text.strip()

    @staticmethod
    def _normalize_inline(text: str) -> str:
        lines = [line.strip() for line in text.splitlines()]
        lines = [line for line in lines if line]
        return "\n".join(lines).strip()

    @staticmethod
    def _post_process_markdown(text: str) -> str:
        text = re.sub(r"\b(def|class|import|from)\[", r"\1 [", text)
        text = re.sub(r"(?m)^(#{2,6}) \[(.+?)\]\(#\)$", r"\1 \2", text)
        return text

    @staticmethod
    def _escape_table_cell(text: str) -> str:
        return text.replace("|", r"\|")

    @staticmethod
    def _prefix_anchor(content: str, anchor: str | None) -> str:
        content = content.strip()
        if not content:
            return ""
        if anchor:
            return f'<a id="{anchor}"></a>\n\n{content}'
        return content


def extract_chm_to_temp(chm_file: Path) -> Path:
    if not chm_file.exists():
        raise FileNotFoundError(f"未找到 CHM 文件: {chm_file}")

    hh_exe = shutil.which("hh.exe")
    if not hh_exe:
        raise FileNotFoundError("未找到 hh.exe，当前环境无法直接解包 CHM")

    temp_root = Path(tempfile.mkdtemp(prefix="kbengine_chm_"))
    result = subprocess.run(
        [hh_exe, "-decompile", str(temp_root), str(chm_file)],
        capture_output=True,
        text=True,
        timeout=120,
    )

    if result.returncode != 0:
        shutil.rmtree(temp_root, ignore_errors=True)
        raise RuntimeError(f"hh.exe 解包失败: {result.stderr.strip()}")

    if not any(temp_root.iterdir()):
        shutil.rmtree(temp_root, ignore_errors=True)
        raise RuntimeError("CHM 解包后没有任何输出")

    return temp_root


def discover_modules_from_output(api_root: Path) -> list[ModuleInfo]:
    modules: list[ModuleInfo] = []
    for module_name in MODULE_ORDER:
        module_dir = api_root / module_name
        readme_path = module_dir / "README.md"
        if not readme_path.exists():
            continue

        module_source = f"{module_name}/Modules/KBEngine.html" if (module_dir / "KBEngine.md").exists() else None
        class_pages = sorted(
            path.name
            for path in module_dir.glob("*.md")
            if path.name not in {"README.md", "KBEngine.md"}
        )

        modules.append(
            ModuleInfo(
                name=module_name,
                intro_source=f"{module_name}/index.html",
                module_source=module_source,
                classes=class_pages,
            )
        )
    return modules


def discover_modules(chm_root: Path) -> list[ModuleInfo]:
    modules: list[ModuleInfo] = []
    for module_name in MODULE_ORDER:
        intro_source = f"{module_name}/index.html"
        intro_path = chm_root / intro_source
        if not intro_path.exists():
            continue

        module_source = f"{module_name}/Modules/KBEngine.html"
        class_dir = chm_root / module_name / "Classes"
        class_pages = []
        if class_dir.exists():
            class_pages = sorted(path.name for path in class_dir.glob("*.html"))

        modules.append(
            ModuleInfo(
                name=module_name,
                intro_source=intro_source,
                module_source=module_source if (chm_root / module_source).exists() else None,
                classes=class_pages,
            )
        )
    return modules


def build_pdf_markdown(pdf_file: Path) -> str:
    if not pdf_file.exists():
        raise FileNotFoundError(f"未找到 PDF 文件: {pdf_file}")

    with pdfplumber.open(str(pdf_file)) as pdf:
        pages = pdf.pages[2:]
        raw_lines: list[str] = []
        for page in pages:
            text = page.extract_text() or ""
            raw_lines.extend(text.splitlines())

    cleaned_lines = [clean_pdf_line(line) for line in raw_lines]
    cleaned_lines = [line for line in cleaned_lines if line]

    blocks: list[str] = [
        "# WebConsole 指南",
        "",
        "> 本页依据 `docs/WebConsole_Guide(cn).pdf` 重新整理，保留文字说明，省略截图编号。",
        "",
    ]

    paragraph_lines: list[str] = []
    bullet_lines: list[str] = []

    def flush_paragraph() -> None:
        nonlocal paragraph_lines
        if not paragraph_lines:
            return
        blocks.append("".join(paragraph_lines).strip())
        blocks.append("")
        paragraph_lines = []

    def flush_bullets() -> None:
        nonlocal bullet_lines
        if not bullet_lines:
            return
        blocks.extend(bullet_lines)
        blocks.append("")
        bullet_lines = []

    for line in cleaned_lines:
        if SCREENSHOT_PATTERN.match(line):
            continue

        section_match = ROMAN_SECTION_PATTERN.match(line)
        if section_match:
            flush_paragraph()
            flush_bullets()
            blocks.append(f"## {section_match.group(2).strip()}")
            blocks.append("")
            continue

        numbered_match = NUMBERED_PATTERN.match(line)
        if numbered_match:
            flush_paragraph()
            flush_bullets()
            blocks.append(f"### {numbered_match.group(1)}. {numbered_match.group(2).strip()}")
            blocks.append("")
            continue

        sub_match = SUB_NUMBERED_PATTERN.match(line)
        if sub_match:
            flush_paragraph()
            flush_bullets()
            blocks.append(f"#### {sub_match.group(1)}) {sub_match.group(2).strip()}")
            blocks.append("")
            continue

        if line.startswith("- "):
            flush_paragraph()
            bullet_lines.append(line)
            continue

        if bullet_lines:
            bullet_lines[-1] = f"{bullet_lines[-1]} {line}".strip()
            continue

        if paragraph_lines and should_join_paragraph(paragraph_lines[-1], line):
            paragraph_lines.append(line)
        else:
            flush_paragraph()
            paragraph_lines = [line]

    flush_paragraph()
    flush_bullets()
    return "\n".join(blocks).strip() + "\n"


def clean_pdf_line(line: str) -> str:
    line = line.replace("\xa0", " ").strip()
    if not line:
        return ""
    if line == "KBEngine" or line == "WebConsole Guide" or line == "目录":
        return ""
    line = line.replace("", "- ")
    if line == "-":
        return "- "
    line = re.sub(r"\s+", " ", line)
    line = line.replace(":-", "：")
    return line.strip()


def should_join_paragraph(previous: str, current: str) -> bool:
    if not previous:
        return False
    if previous.endswith(("：", ":", "，", ",", "（", "(")):
        return True
    if current.startswith(("“", "”", "、", "，", ",", "）", ")", "的", "并", "且", "而", "其")):
        return True
    if re.search(r"[A-Za-z0-9]$", previous) and re.match(r"^[A-Za-z0-9]", current):
        return True
    if re.search(r"[\u4e00-\u9fff]$", previous) and re.match(r"^[\u4e00-\u9fff]", current):
        return True
    return False


def build_homepage(modules: Iterable[ModuleInfo]) -> str:
    module_links = "\n".join(
        f"- [{module.name} API](/api/{module.name}/)"
        for module in modules
    )
    return (
        "---\n"
        "home: true\n"
        "heroImage: /logo.png\n"
        "heroText: KBEngine 中文文档\n"
        "tagline: 基于仓库内 CHM 与 PDF 整理的中文文档入口\n"
        "actions:\n"
        "  - text: 开始阅读\n"
        "    link: /guide/\n"
        "    type: primary\n"
        "  - text: 查看架构\n"
        "    link: /architecture/\n"
        "    type: secondary\n"
        "  - text: 查阅 API\n"
        "    link: /api/\n"
        "    type: secondary\n"
        "footer: 文档来源为仓库内已有资料\n"
        "---\n\n"
        "## 文档导航\n\n"
        "### 指南\n\n"
        "- [阅读入口](/guide/)\n"
        "- [WebConsole 指南](/guide/webconsole.md)\n"
        "\n"
        "### 架构\n\n"
        "- [架构总览](/architecture/)\n"
        "- [源码分析](/architecture/source-analysis.md)\n"
        "- [BigWorld 对照](/architecture/bigworld.md)\n"
        "\n"
        "### API 参考\n\n"
        "- [API 总览](/api/)\n"
        "- [基本数据类型](/api/basetypes.md)\n"
        "- [关键词释义](/api/keywords.md)\n\n"
        f"{module_links}\n\n"
        "### 资料\n\n"
        "- [资料与说明](/resources/)\n"
    )


def build_guide_index() -> str:
    return (
        "# 指南\n\n"
        "> 本目录整理现有使用类文档。\n\n"
        "## 阅读入口\n\n"
        "- [WebConsole 指南](/guide/webconsole.md)\n"
        "- [架构总览](/architecture/)\n"
        "- [API 总览](/api/)\n"
        "- [资料与说明](/resources/)\n"
    )


def build_api_index(modules: Iterable[ModuleInfo]) -> str:
    lines = [
        "# API 总览",
        "",
        "> 本目录来自 `docs/api/kbengine_api(cn).chm` 的中文 API 帮助文档。",
        "",
        "## 核心概念",
        "",
        "- [基本数据类型](/api/basetypes.md)",
        "- [关键词释义](/api/keywords.md)",
        "",
        "## 客户端",
        "",
        "- [client](/api/client/)",
        "- [bots](/api/bots/)",
        "",
        "## 服务端组件",
        "",
    ]
    for module in modules:
        if module.name in {"client", "bots"}:
            continue
        lines.append(f"- [{module.name}](/api/{module.name}/)")
    lines.append("")
    return "\n".join(lines)


def build_architecture_index() -> str:
    return (
        "# 架构总览\n\n"
        "> 本栏目当前仅搭建文档框架，具体内容待按源码、CHM、PDF 逐项核对后补充。\n\n"
        "## 待整理栏目\n\n"
        "- [源码分析](/architecture/source-analysis.md)\n"
        "- [BigWorld 对照](/architecture/bigworld.md)\n\n"
        "## 说明\n\n"
        "- 本页当前不提供结论性描述。\n"
        "- 后续补充时，以源码与仓库内现有资料为准。\n"
    )


def build_source_analysis_index() -> str:
    return (
        "# 源码分析\n\n"
        "> 占位页。后续按源码逐项核对后补充。\n\n"
        "## 计划整理范围\n\n"
        "- 组件启动流程\n"
        "- 实体与消息链路\n"
        "- Space 与 Cell 相关机制\n"
        "- 关键模块源码阅读索引\n\n"
        "## 资料边界\n\n"
        "- 以仓库源码为主。\n"
        "- 未核对前不写结论。\n"
    )


def build_bigworld_index() -> str:
    return (
        "# BigWorld 对照\n\n"
        "> 占位页。后续在核对现有资料与源码后补充。\n\n"
        "## 计划整理范围\n\n"
        "- 术语对应\n"
        "- 概念差异\n"
        "- 架构思路对照\n\n"
        "## 资料边界\n\n"
        "- 未核对前不写具体映射结论。\n"
    )


def build_resources_index() -> str:
    return (
        "# 资料与说明\n\n"
        "> 本页仅汇总当前整理所依赖的仓库内资料。\n\n"
        "## 当前资料\n\n"
        "- 协议文件：`LICENSE.txt`\n"
        "- API 中文文档：`docs/api/kbengine_api(cn).chm`\n"
        "- WebConsole 中文文档：`docs/WebConsole_Guide(cn).pdf`\n"
        "- 仓库说明：`README.md`\n\n"
        "## 说明\n\n"
        "- 本站整理时优先以仓库内现有资料为准。\n"
        "- 新增说明性内容需要逐项核对后再补充。\n"
    )


def build_vuepress_config(modules: Iterable[ModuleInfo]) -> str:
    server_children: list[str] = []
    for module in modules:
        if module.name in {"client", "bots"}:
            continue

        children = [f"'/api/{module.name}/README.md'"]
        if module.module_source:
            children.append(f"'/api/{module.name}/KBEngine.md'")
        for class_page in module.classes:
            children.append(f"'/api/{module.name}/{Path(class_page).stem}.md'")

        server_children.extend(children)

    server_sidebar = (
        "        {\n"
        "          text: '服务端组件',\n"
        f"          children: [{', '.join(server_children)}],\n"
        "        },\n"
    )

    return (
        "import { defaultTheme } from '@vuepress/theme-default'\n"
        "import { defineUserConfig } from 'vuepress/cli'\n"
        "import { viteBundler } from '@vuepress/bundler-vite'\n"
        "import { searchPlugin } from '@vuepress/plugin-search'\n\n"
        "export default defineUserConfig({\n"
        "  lang: 'zh-CN',\n"
        "  title: 'KBEngine 文档',\n"
        "  description: '基于 CHM 与 PDF 重建的 KBEngine 中文文档站点',\n"
        "  head: [\n"
        "    ['link', { rel: 'icon', href: '/logo.png' }],\n"
        "  ],\n"
        "  bundler: viteBundler({\n"
        "    viteOptions: {\n"
        "      css: {\n"
        "        preprocessorOptions: {\n"
        "          scss: {\n"
        "            quietDeps: true,\n"
        "            silenceDeprecations: ['import'],\n"
        "          },\n"
        "        },\n"
        "      },\n"
        "    },\n"
        "  }),\n"
        "  plugins: [\n"
        "    searchPlugin({\n"
        "      locales: {\n"
        "        '/': {\n"
        "          placeholder: '搜索文档',\n"
        "        },\n"
        "      },\n"
        "      maxSuggestions: 10,\n"
        "      isSearchable: (page) => !page.path.startsWith('/404'),\n"
        "    }),\n"
        "  ],\n"
        "  theme: defaultTheme({\n"
        "    logo: '/logo.png',\n"
        "    repo: 'cuihairu/kbengine',\n"
        "    editLink: false,\n"
        "    contributors: false,\n"
        "    lastUpdated: false,\n"
        "    navbar: [\n"
        "      { text: '首页', link: '/' },\n"
        "      { text: '指南', link: '/guide/' },\n"
        "      { text: '架构', link: '/architecture/' },\n"
        "      { text: 'API', link: '/api/' },\n"
        "      { text: '资料', link: '/resources/' },\n"
        "    ],\n"
        "    sidebar: {\n"
        "      '/architecture/': [\n"
        "        {\n"
        "          text: '架构',\n"
        "          children: [\n"
        "            '/architecture/README.md',\n"
        "            '/architecture/source-analysis.md',\n"
        "            '/architecture/bigworld.md',\n"
        "          ],\n"
        "        },\n"
        "      ],\n"
        "      '/guide/': [\n"
        "        {\n"
        "          text: '指南',\n"
          "          children: ['/guide/README.md', '/guide/webconsole.md'],\n"
        "        },\n"
        "      ],\n"
        "      '/api/': [\n"
        "        {\n"
        "          text: '核心概念',\n"
        "          children: ['/api/README.md', '/api/basetypes.md', '/api/keywords.md'],\n"
        "        },\n"
        "        {\n"
        "          text: '客户端',\n"
        "          children: ['/api/client/README.md', '/api/client/KBEngine.md', '/api/client/Entity.md', '/api/bots/README.md', '/api/bots/KBEngine.md', '/api/bots/Entity.md', '/api/bots/PyClientApp.md'],\n"
        "        },\n"
        f"{server_sidebar}"
        "      ],\n"
        "      '/resources/': [\n"
        "        {\n"
        "          text: '资料',\n"
        "          children: ['/resources/README.md'],\n"
        "        },\n"
        "      ],\n"
        "      '/': [],\n"
        "    },\n"
        "  }),\n"
        "})\n"
    )


def ensure_clean_output(output_dir: Path) -> None:
    if output_dir.exists():
        shutil.rmtree(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)


def write_text(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content.strip() + "\n", encoding="utf-8")


def restore_logo_from_git(output_path: Path) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    result = subprocess.run(
        ["git", "show", f"HEAD:{LOGO_GIT_PATH}"],
        cwd=ROOT,
        capture_output=True,
        timeout=30,
    )
    if result.returncode == 0 and result.stdout:
        output_path.write_bytes(result.stdout)


def rebuild_docs() -> None:
    temp_chm_root: Path | None = None
    modules: list[ModuleInfo]
    use_existing_api = False

    try:
        temp_chm_root = extract_chm_to_temp(CHM_FILE)
    except (FileNotFoundError, RuntimeError):
        existing_api_root = OUTPUT_DIR / "api"
        modules = discover_modules_from_output(existing_api_root)
        if not modules:
            raise
        use_existing_api = True
    else:
        modules = discover_modules(temp_chm_root)

    if not use_existing_api:
        ensure_clean_output(OUTPUT_DIR)
        restore_logo_from_git(LOGO_OUTPUT)

        try:
            source_to_output_map: dict[str, PurePosixPath] = {
                "basetypes.html": PurePosixPath("basetypes.md"),
                "keywords.html": PurePosixPath("keywords.md"),
            }
            for module in modules:
                source_to_output_map[module.intro_source.lower()] = PurePosixPath(f"{module.name}/README.md")
                if module.module_source:
                    source_to_output_map[module.module_source.lower()] = PurePosixPath(
                        f"{module.name}/KBEngine.md"
                    )
                for class_page in module.classes:
                    source_to_output_map[f"{module.name}/classes/{class_page.lower()}"] = PurePosixPath(
                        f"{module.name}/{Path(class_page).stem}.md"
                    )

            converter = HtmlConverter(temp_chm_root, OUTPUT_DIR / "api", source_to_output_map)

            converter.convert_page("basetypes.html", "basetypes.md")
            converter.convert_page("keywords.html", "keywords.md")

            for module in modules:
                converter.convert_page(module.intro_source, f"{module.name}/README.md")
                if module.module_source:
                    converter.convert_page(module.module_source, f"{module.name}/KBEngine.md")
                for class_page in module.classes:
                    converter.convert_page(
                        f"{module.name}/Classes/{class_page}",
                        f"{module.name}/{Path(class_page).stem}.md",
                    )
        finally:
            if temp_chm_root is not None:
                shutil.rmtree(temp_chm_root, ignore_errors=True)
    else:
        OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
        restore_logo_from_git(LOGO_OUTPUT)

    write_text(OUTPUT_DIR / "README.md", build_homepage(modules))
    write_text(OUTPUT_DIR / "guide" / "README.md", build_guide_index())
    write_text(OUTPUT_DIR / "guide" / "webconsole.md", build_pdf_markdown(PDF_FILE))
    write_text(OUTPUT_DIR / "architecture" / "README.md", build_architecture_index())
    write_text(OUTPUT_DIR / "architecture" / "source-analysis.md", build_source_analysis_index())
    write_text(OUTPUT_DIR / "architecture" / "bigworld.md", build_bigworld_index())
    write_text(OUTPUT_DIR / "api" / "README.md", build_api_index(modules))
    write_text(OUTPUT_DIR / "resources" / "README.md", build_resources_index())
    write_text(OUTPUT_DIR / ".vuepress" / "config.ts", build_vuepress_config(modules))


def main() -> None:
    rebuild_docs()
    print(f"文档已重建: {OUTPUT_DIR}")


if __name__ == "__main__":
    main()
