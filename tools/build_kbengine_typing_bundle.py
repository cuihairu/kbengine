#!/usr/bin/env python3
from __future__ import annotations

import argparse
import shutil
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_OUTPUT = ROOT / ".tmp" / "kbengine-typing-bundle"
PAYLOAD_DIRNAME = ".kbengine_typing_payload"
GENERATOR_NAME = "generate_kbengine_entity_stubs.py"
SERVER_ERRORS_GENERATOR_NAME = "generate_kbengine_server_errors_stub.py"
VALIDATOR_NAME = "validate_kbengine_typing.py"
INSTALLER_NAME = "install_kbengine_typing_bundle.py"
INSTALLER_CMD_NAME = "install_kbengine_typing_bundle.cmd"

COMPONENT_NAMES = ("base", "cell", "interface", "login", "db", "logger", "bots")
GENERAL_SCRIPT_FILES = (
    Path("common/KBEDebug.pyi"),
    Path("common/Functor.pyi"),
    Path("base/kbemain.pyi"),
    Path("cell/kbemain.pyi"),
    Path("interface/kbemain.pyi"),
    Path("interface/Poller.pyi"),
    Path("login/kbemain.pyi"),
    Path("db/kbemain.pyi"),
    Path("logger/kbemain.pyi"),
    Path("bots/kbemain.pyi"),
)
PAYLOAD_TOOL_FILES = (
    GENERATOR_NAME,
    SERVER_ERRORS_GENERATOR_NAME,
    VALIDATOR_NAME,
)

BUNDLE_README = """# KBEngine Typing Bundle

这个目录是一个可直接丢到旧项目根目录的 drop-in 导入包。

## 推荐用法

1. 把当前目录整体复制到旧项目根目录
2. 在旧项目根目录运行：

```powershell
python .\\install_kbengine_typing_bundle.py
```

或者在 Windows 下直接运行：

```bat
install_kbengine_typing_bundle.cmd
```

## 为什么不是只复制 typings/

因为这套 typing 还依赖这些旁路 stub：

- `scripts/*/KBEngine/*.pyi`
- `scripts/*/kbemain.pyi`
- `scripts/interface/Poller.pyi`
- `scripts/common/KBEDebug.pyi`
- `scripts/common/Functor.pyi`

所以需要通过安装脚本把 payload 拆到旧项目的正确位置。

## Payload 结构

- `.kbengine_typing_payload/typings`
- `.kbengine_typing_payload/scripts`
- `.kbengine_typing_payload/tools`

## 说明

- 这个 drop-in 包不包含模板示例实体 `Account.pyi`
- 真实项目的实体 / 组件 `.pyi` 仍然要根据目标项目自己的 `scripts/entities.xml` 与 `scripts/entity_defs` 本地生成
"""

INSTALLER_CMD = """@echo off
setlocal
python "%~dp0install_kbengine_typing_bundle.py" %*
endlocal
"""


def copy_file(src: Path, dst: Path) -> None:
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, dst)


def copy_tree(src: Path, dst: Path) -> None:
    for path in src.rglob("*"):
        if path.is_dir():
            continue
        copy_file(path, dst / path.relative_to(src))


def build_bundle(output_base: Path) -> Path:
    bundle_root = output_base.resolve()
    if bundle_root.exists():
        shutil.rmtree(bundle_root)

    bundle_root.mkdir(parents=True, exist_ok=True)
    (bundle_root / "README.md").write_text(BUNDLE_README, encoding="utf-8")
    copy_file(ROOT / "tools" / INSTALLER_NAME, bundle_root / INSTALLER_NAME)
    (bundle_root / INSTALLER_CMD_NAME).write_text(INSTALLER_CMD, encoding="utf-8", newline="\r\n")

    payload_root = bundle_root / PAYLOAD_DIRNAME
    copy_tree(ROOT / "typings", payload_root / "typings")

    scripts_root = ROOT / "kbe" / "res" / "sdk_templates" / "server" / "python_assets" / "scripts"
    for component in COMPONENT_NAMES:
        copy_tree(
            scripts_root / component / "KBEngine",
            payload_root / "scripts" / component / "KBEngine",
        )

    for relative_file in GENERAL_SCRIPT_FILES:
        copy_file(scripts_root / relative_file, payload_root / "scripts" / relative_file)

    for tool_name in PAYLOAD_TOOL_FILES:
        copy_file(ROOT / "tools" / tool_name, payload_root / "tools" / tool_name)

    return bundle_root


def make_zip(bundle_root: Path) -> Path:
    archive_base = bundle_root.parent / bundle_root.name
    archive_path = shutil.make_archive(str(archive_base), "zip", root_dir=str(bundle_root.parent), base_dir=bundle_root.name)
    return Path(archive_path)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="打包 KBEngine typing drop-in 导入包，供旧项目直接复制到根目录后安装。"
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=DEFAULT_OUTPUT,
        help="bundle 输出目录。不带 .zip 后缀，默认输出到 .tmp/kbengine-typing-bundle。",
    )
    parser.add_argument(
        "--no-zip",
        action="store_true",
        help="只生成目录，不额外打 zip。",
    )
    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()

    output_base = args.output.resolve()
    output_base.parent.mkdir(parents=True, exist_ok=True)
    bundle_root = build_bundle(output_base)

    print(f"drop-in 目录已生成: {bundle_root}")
    if args.no_zip:
        return 0

    zip_path = make_zip(bundle_root)
    print(f"drop-in zip 已生成: {zip_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
