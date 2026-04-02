#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
CHM 文档提取器
将 KBEngine CHM 文档转换为 VuePress Markdown 格式
"""

import os
import sys
import subprocess
import shutil
from pathlib import Path


class CHMExtractor:
    """CHM 文件提取器"""

    def __init__(self, chm_file_path):
        self.chm_file_path = chm_file_path
        self.extract_dir = None
        self.output_dir = None

    def extract(self, output_dir):
        """
        提取 CHM 文件内容

        Args:
            output_dir: 输出目录
        """
        self.output_dir = output_dir
        os.makedirs(output_dir, exist_ok=True)

        print(f"正在提取 CHM 文件: {self.chm_file_path}")

        # 创建临时提取目录
        temp_dir = "temp_chm_extract"
        if os.path.exists(temp_dir):
            shutil.rmtree(temp_dir)
        os.makedirs(temp_dir)

        try:
            # 方法1: 使用 Windows hh.exe 命令
            if self._extract_with_hh(temp_dir):
                self._process_extracted_files(temp_dir, output_dir)
                print(f"✓ CHM 文件提取成功: {output_dir}")
                return True

            # 方法2: 使用 7-Zip
            elif self._extract_with_7zip(temp_dir):
                self._process_extracted_files(temp_dir, output_dir)
                print(f"✓ CHM 文件提取成功: {output_dir}")
                return True

            else:
                print("✗ 无法提取 CHM 文件")
                print("请尝试安装以下工具之一：")
                print("1. Windows Help Workshop (包含 hh.exe)")
                print("2. 7-Zip")
                return False

        finally:
            # 清理临时目录
            if os.path.exists(temp_dir):
                shutil.rmtree(temp_dir)

    def _extract_with_hh(self, temp_dir):
        """使用 Windows hh.exe 提取 CHM"""
        try:
            # 使用 hh.exe -decompile 命令
            cmd = ["hh.exe", "-decompile", temp_dir, self.chm_file_path]
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=60)

            if result.returncode == 0 and os.listdir(temp_dir):
                print("✓ 使用 hh.exe 提取成功")
                return True
            else:
                print(f"hh.exe 提取失败: {result.stderr}")
                return False

        except FileNotFoundError:
            print("hh.exe 未找到")
            return False
        except Exception as e:
            print(f"hh.exe 提取出错: {e}")
            return False

    def _extract_with_7zip(self, temp_dir):
        """使用 7-Zip 提取 CHM"""
        try:
            # 查找 7-Zip
            seven_zip_paths = [
                r"C:\Program Files\7-Zip\7z.exe",
                r"C:\Program Files (x86)\7-Zip\7z.exe",
            ]

            seven_zip = None
            for path in seven_zip_paths:
                if os.path.exists(path):
                    seven_zip = path
                    break

            if not seven_zip:
                print("7-Zip 未找到")
                return False

            # 使用 7-Zip 解压
            cmd = [seven_zip, "x", "-y", f"-o{temp_dir}", self.chm_file_path]
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=60)

            if result.returncode == 0 and os.listdir(temp_dir):
                print("✓ 使用 7-Zip 提取成功")
                return True
            else:
                print(f"7-Zip 提取失败: {result.stderr}")
                return False

        except Exception as e:
            print(f"7-Zip 提取出错: {e}")
            return False

    def _process_extracted_files(self, temp_dir, output_dir):
        """处理提取的文件"""
        # 查找 HTML 文件
        html_files = list(Path(temp_dir).rglob("*.html"))
        print(f"找到 {len(html_files)} 个 HTML 文件")

        if not html_files:
            print("警告：未找到 HTML 文件")
            return

        # 复制 HTML 文件到输出目录
        for html_file in html_files:
            try:
                # 相对路径
                rel_path = html_file.relative_to(temp_dir)
                dest_file = Path(output_dir) / rel_path

                # 创建目标目录
                dest_file.parent.mkdir(parents=True, exist_ok=True)

                # 复制文件
                shutil.copy2(html_file, dest_file)
                print(f"已复制: {rel_path}")

            except Exception as e:
                print(f"复制文件失败 {html_file}: {e}")

        # 复制图片和其他资源
        resource_extensions = ['.png', '.jpg', '.jpeg', '.gif', '.css', '.js']
        for ext in resource_extensions:
            for resource_file in Path(temp_dir).rglob(f"*{ext}"):
                try:
                    rel_path = resource_file.relative_to(temp_dir)
                    dest_file = Path(output_dir) / rel_path
                    dest_file.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copy2(resource_file, dest_file)
                    print(f"已复制资源: {rel_path}")
                except Exception as e:
                    print(f"复制资源失败 {resource_file}: {e}")


def main():
    """主函数"""
    # 配置路径
    chm_file = "docs/api/kbengine_api(cn).chm"
    output_dir = "docs-vuepress/api/chm-content"

    # 检查文件是否存在
    if not os.path.exists(chm_file):
        print(f"错误：文件不存在 - {chm_file}")
        return

    # 创建提取器
    extractor = CHMExtractor(chm_file)

    # 执行提取
    print(f"开始提取: {chm_file}")
    success = extractor.extract(output_dir)

    if success:
        print(f"\n提取完成！")
        print(f"输出目录: {output_dir}")
        print(f"\n下一步：")
        print(f"1. 查看提取的文件: {output_dir}")
        print(f"2. 将 HTML 文件转换为 Markdown 格式")
        print(f"3. 整理到 VuePress 文档结构中")
    else:
        print(f"\n提取失败！")
        print(f"\n建议：")
        print(f"1. 确保已安装 Windows Help Workshop 或 7-Zip")
        print(f"2. 或者直接使用原始 CHM 文件作为参考")


if __name__ == "__main__":
    main()
