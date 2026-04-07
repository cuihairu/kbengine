#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import shutil
import subprocess
import sys
from pathlib import Path


SCRIPT_PATH = Path(__file__).resolve()
SCRIPT_DIR = SCRIPT_PATH.parent
PAYLOAD_DIRNAME = ".kbengine_typing_payload"
GENERATOR_NAME = "generate_kbengine_entity_stubs.py"
SERVER_ERRORS_GENERATOR_NAME = "generate_kbengine_server_errors_stub.py"
VALIDATOR_NAME = "validate_kbengine_typing.py"

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
REQUIRED_TOOL_FILES = (GENERATOR_NAME,)
OPTIONAL_TOOL_FILES = (
    SERVER_ERRORS_GENERATOR_NAME,
    VALIDATOR_NAME,
)


def is_bundle_root(candidate: Path) -> bool:
    return (
        (candidate / "typings").exists()
        and (candidate / "tools" / GENERATOR_NAME).exists()
    )


def resolve_bundle_root(bundle_root: Path | None) -> Path:
    if bundle_root is not None:
        candidate = bundle_root.resolve()
        if is_bundle_root(candidate):
            return candidate
        raise FileNotFoundError(f"未找到可用的 typing payload: {candidate}")

    candidates: list[Path] = []
    payload_candidate = (SCRIPT_DIR / PAYLOAD_DIRNAME).resolve()
    candidates.append(payload_candidate)
    candidates.append(SCRIPT_DIR.resolve())
    if SCRIPT_DIR.name == "tools":
        candidates.append(SCRIPT_DIR.parent.resolve())

    checked: set[Path] = set()
    for candidate in candidates:
        if candidate in checked:
            continue
        checked.add(candidate)
        if is_bundle_root(candidate):
            return candidate

    locations = " / ".join(path.as_posix() for path in checked)
    raise FileNotFoundError(f"未找到可用的 typing payload。已检查: {locations}")


def resolve_project_root(project_root: Path | None, bundle_root: Path) -> Path:
    if project_root is not None:
        return project_root.resolve()

    if bundle_root == (SCRIPT_DIR / PAYLOAD_DIRNAME).resolve():
        return SCRIPT_DIR.resolve()

    return Path.cwd().resolve()


def resolve_scripts_payload_dir(bundle_root: Path) -> Path:
    bundle_layout = bundle_root / "scripts"
    if (bundle_layout / "base" / "KBEngine" / "__init__.pyi").exists():
        return bundle_layout

    repo_layout = bundle_root / "kbe" / "res" / "sdk_templates" / "server" / "python_assets" / "scripts"
    if (repo_layout / "base" / "KBEngine" / "__init__.pyi").exists():
        return repo_layout

    raise FileNotFoundError("未找到 scripts payload 目录，既不是 bundle 布局，也不是仓库源码布局。")


def copy_file(src: Path, dst: Path) -> None:
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, dst)


def copy_tree(src: Path, dst: Path) -> None:
    for path in src.rglob("*"):
        if path.is_dir():
            continue
        copy_file(path, dst / path.relative_to(src))


def relativize(path: Path, base: Path) -> str:
    try:
        return path.resolve().relative_to(base.resolve()).as_posix()
    except ValueError:
        return path.resolve().as_posix()


def build_pyright_config(project_root: Path, scripts_dir: Path) -> dict[str, object]:
    scripts_rel = relativize(scripts_dir, project_root)
    common_rel = f"{scripts_rel}/common"

    execution_envs: list[dict[str, object]] = []
    for component in COMPONENT_NAMES:
        component_root = f"{scripts_rel}/{component}"
        extra_paths = [component_root, common_rel, "typings"]
        execution_envs.append({"root": component_root, "extraPaths": extra_paths})

    execution_envs.append({"root": common_rel, "extraPaths": [common_rel, "typings"]})

    return {
        "include": [scripts_rel],
        "exclude": ["**/__pycache__", ".tmp", "node_modules", "build"],
        "stubPath": "typings/stubs",
        "executionEnvironments": execution_envs,
        "pythonVersion": "3.12",
        "typeCheckingMode": "basic",
        "reportMissingModuleSource": "none",
    }


def write_json(path: Path, payload: dict[str, object]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def resolve_bundle_tool_paths(bundle_root: Path) -> dict[str, Path]:
    tool_paths: dict[str, Path] = {}
    for tool_name in (*REQUIRED_TOOL_FILES, *OPTIONAL_TOOL_FILES):
        candidate = bundle_root / "tools" / tool_name
        if candidate.exists():
            tool_paths[tool_name] = candidate

    repo_tools_root = bundle_root / "tools"
    if repo_tools_root.exists():
        for tool_name in (*REQUIRED_TOOL_FILES, *OPTIONAL_TOOL_FILES):
            candidate = repo_tools_root / tool_name
            if candidate.exists():
                tool_paths[tool_name] = candidate

    missing = [tool_name for tool_name in REQUIRED_TOOL_FILES if tool_name not in tool_paths]
    if missing:
        missing_text = ", ".join(missing)
        raise FileNotFoundError(f"未找到必需工具文件: {missing_text}")

    return tool_paths


def ensure_install_target(bundle_root: Path, project_root: Path, scripts_dir: Path) -> None:
    if bundle_root == project_root:
        raise ValueError(
            "目标项目根目录不能与 typing payload 根目录相同。"
            "drop-in 模式请把安装器和 .kbengine_typing_payload 一起复制到旧项目根目录后再运行；"
            "源码/传统 bundle 模式请显式传入 --project-root。"
        )

    if not scripts_dir.exists():
        raise FileNotFoundError(f"未找到目标 scripts 目录: {scripts_dir}")


def install_typings(bundle_root: Path, project_root: Path, scripts_dir: Path) -> list[Path]:
    written: list[Path] = []
    scripts_payload_dir = resolve_scripts_payload_dir(bundle_root)
    bundle_tool_paths = resolve_bundle_tool_paths(bundle_root)

    target_typings_dir = project_root / "typings"
    copy_tree(bundle_root / "typings", target_typings_dir)
    written.append(target_typings_dir)

    target_tools_dir = project_root / "tools"
    for tool_name, tool_src in sorted(bundle_tool_paths.items()):
        tool_dst = target_tools_dir / tool_name
        copy_file(tool_src, tool_dst)
        written.append(tool_dst)

    for component in COMPONENT_NAMES:
        component_src = scripts_payload_dir / component / "KBEngine"
        component_dst = scripts_dir / component / "KBEngine"
        copy_tree(component_src, component_dst)
        written.append(component_dst)

    for relative_file in GENERAL_SCRIPT_FILES:
        src = scripts_payload_dir / relative_file
        dst = scripts_dir / relative_file
        copy_file(src, dst)
        written.append(dst)

    return written


def maybe_write_pyright_config(project_root: Path, scripts_dir: Path, mode: str) -> Path | None:
    config_payload = build_pyright_config(project_root, scripts_dir)
    pyright_path = project_root / "pyrightconfig.json"
    snippet_path = project_root / "pyrightconfig.kbengine_typing.json"

    if mode == "never":
        write_json(snippet_path, config_payload)
        return snippet_path

    if mode == "always":
        write_json(pyright_path, config_payload)
        return pyright_path

    if pyright_path.exists():
        write_json(snippet_path, config_payload)
        return snippet_path

    write_json(pyright_path, config_payload)
    return pyright_path


def run_generator(project_root: Path, scripts_dir: Path, python_executable: str) -> int:
    generator_path = project_root / "tools" / GENERATOR_NAME
    command = [
        python_executable,
        str(generator_path),
        "--scripts-dir",
        str(scripts_dir),
    ]
    completed = subprocess.run(command, cwd=str(project_root))
    return completed.returncode


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="将 KBEngine typing payload 安装到旧项目，并可选生成实体旁路 .pyi。"
    )
    parser.add_argument(
        "--bundle-root",
        type=Path,
        help="typing payload 根目录。drop-in 模式下默认自动识别为脚本旁边的 .kbengine_typing_payload。",
    )
    parser.add_argument(
        "--project-root",
        type=Path,
        help="目标项目根目录。drop-in 模式下默认是当前安装脚本所在目录，其余模式默认是当前工作目录。",
    )
    parser.add_argument(
        "--scripts-dir",
        type=Path,
        help="目标项目的 scripts 目录。默认使用 <project-root>/scripts。",
    )
    parser.add_argument(
        "--pyright-config",
        choices=("auto", "always", "never"),
        default="auto",
        help=(
            "如何写入 pyright 配置。"
            "auto=若无 pyrightconfig.json 则直接写入，否则输出 pyrightconfig.kbengine_typing.json；"
            "always=总是覆盖写入 pyrightconfig.json；"
            "never=只输出 pyrightconfig.kbengine_typing.json。"
        ),
    )
    parser.add_argument(
        "--skip-generate",
        action="store_true",
        help="只安装通用 typing，不自动生成实体旁路 .pyi。",
    )
    parser.add_argument(
        "--python",
        default=sys.executable,
        help="运行生成器使用的 Python 可执行文件路径。",
    )
    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()

    try:
        bundle_root = resolve_bundle_root(args.bundle_root)
        project_root = resolve_project_root(args.project_root, bundle_root)
        scripts_dir = (args.scripts_dir or (project_root / "scripts")).resolve()
        ensure_install_target(bundle_root, project_root, scripts_dir)
    except (FileNotFoundError, ValueError) as exc:
        print(str(exc), file=sys.stderr)
        return 2

    project_root.mkdir(parents=True, exist_ok=True)

    try:
        installed = install_typings(bundle_root, project_root, scripts_dir)
    except FileNotFoundError as exc:
        print(str(exc), file=sys.stderr)
        return 2

    config_path = maybe_write_pyright_config(project_root, scripts_dir, args.pyright_config)

    print("已安装 KBEngine typing 文件：")
    for path in installed:
        print(f"- {path}")

    if config_path is not None:
        print(f"已输出 pyright 配置: {config_path}")

    if args.skip_generate:
        print("已跳过实体 .pyi 生成。")
        return 0

    result = run_generator(project_root, scripts_dir, args.python)
    if result != 0:
        print("实体 .pyi 生成失败，请检查 scripts/entity_defs 是否完整。", file=sys.stderr)
        return result

    print("实体 .pyi 已生成完成。")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
