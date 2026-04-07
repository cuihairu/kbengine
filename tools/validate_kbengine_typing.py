#!/usr/bin/env python3
from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_SCRIPTS_DIR = ROOT / "kbe" / "res" / "sdk_templates" / "server" / "python_assets" / "scripts"

TYPING_TOOL_FILES = (
    ROOT / "tools" / "build_kbengine_typing_bundle.py",
    ROOT / "tools" / "generate_kbengine_entity_stubs.py",
    ROOT / "tools" / "generate_kbengine_server_errors_stub.py",
    ROOT / "tools" / "install_kbengine_typing_bundle.py",
    ROOT / "tools" / "validate_kbengine_typing.py",
)


@dataclass(frozen=True)
class Step:
    name: str
    command: list[str]


def build_steps(mode: str, scripts_dir: Path, python_executable: str, pyright_command: list[str]) -> list[Step]:
    server_errors_command = [
        python_executable,
        str(ROOT / "tools" / "generate_kbengine_server_errors_stub.py"),
    ]
    entity_stubs_command = [
        python_executable,
        str(ROOT / "tools" / "generate_kbengine_entity_stubs.py"),
        "--scripts-dir",
        str(scripts_dir),
    ]

    if mode == "check":
        server_errors_command.append("--check")
        entity_stubs_command.append("--check")

    py_compile_command = [
        python_executable,
        "-m",
        "py_compile",
        *(str(path) for path in TYPING_TOOL_FILES),
    ]

    return [
        Step("校验 SERVER_ERR stub", server_errors_command),
        Step("校验实体旁路 stub", entity_stubs_command),
        Step("编译 typing 工具脚本", py_compile_command),
        Step("运行 pyright", [*pyright_command, "-p", str(ROOT)]),
    ]


def run_step(step: Step) -> int:
    print(f"[kbengine-typing] {step.name}")
    command = list(step.command)
    resolved_executable = shutil.which(command[0])
    if resolved_executable is not None:
        command[0] = resolved_executable
    completed = subprocess.run(command, cwd=str(ROOT))
    return completed.returncode


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="统一执行 KBEngine typing 的生成/校验流程。",
    )
    parser.add_argument(
        "--mode",
        choices=("check", "sync"),
        default="check",
        help="check=检查生成结果是否最新；sync=先重写生成文件再跑后续校验。",
    )
    parser.add_argument(
        "--scripts-dir",
        type=Path,
        default=DEFAULT_SCRIPTS_DIR,
        help="entity stub 生成器使用的 scripts 目录。",
    )
    parser.add_argument(
        "--python",
        default=sys.executable,
        help="运行生成器和 py_compile 使用的 Python 可执行文件。",
    )
    parser.add_argument(
        "--pyright-command",
        nargs="+",
        default=["npx", "-y", "pyright"],
        help="运行 pyright 的命令。",
    )
    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()

    scripts_dir = args.scripts_dir.resolve()
    if not scripts_dir.exists():
        print(f"未找到 scripts 目录: {scripts_dir}", file=sys.stderr)
        return 2

    for step in build_steps(args.mode, scripts_dir, args.python, args.pyright_command):
        result = run_step(step)
        if result != 0:
            return result

    print("[kbengine-typing] 全部通过")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
