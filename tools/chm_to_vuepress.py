#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
CHM 文档完整提取和转换工具
将 KBEngine CHM 文档提取并转换为 VuePress Markdown 格式
"""

import os
import sys
import subprocess
import shutil
import re
from pathlib import Path
from bs4 import BeautifulSoup


class CHMToVuePressConverter:
    """CHM 到 VuePress 转换器"""

    def __init__(self, chm_file_path, output_base_dir):
        self.chm_file_path = chm_file_path
        self.output_base_dir = output_base_dir
        self.temp_dir = "temp_chm_extract"

    def convert(self):
        """执行完整的转换流程"""
        print(f"开始转换 CHM 文件: {self.chm_file_path}")

        # 1. 提取 CHM 文件
        if not self._extract_chm():
            print("[ERROR] CHM 文件提取失败")
            return False

        # 2. 分析文件结构
        structure = self._analyze_structure()
        print(f"找到 {len(structure['modules'])} 个模块")

        # 3. 转换 HTML 到 Markdown
        self._convert_html_to_markdown(structure)

        # 4. 创建 VuePress 配置
        self._create_vuepress_config(structure)

        # 5. 清理临时文件
        if os.path.exists(self.temp_dir):
            shutil.rmtree(self.temp_dir)

        print("[SUCCESS] 转换完成！")
        print(f"输出目录: {self.output_base_dir}")
        return True

    def _extract_chm(self):
        """提取 CHM 文件"""
        print("正在提取 CHM 文件...")

        # 清理旧的临时目录
        if os.path.exists(self.temp_dir):
            shutil.rmtree(self.temp_dir)
        os.makedirs(self.temp_dir)

        # 使用 hh.exe 提取
        try:
            cmd = ["hh.exe", "-decompile", self.temp_dir, self.chm_file_path]
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=60)

            if result.returncode != 0 or not os.listdir(self.temp_dir):
                print(f"提取失败: {result.stderr}")
                return False

            print("[SUCCESS] CHM 文件提取成功")
            return True

        except Exception as e:
            print(f"提取出错: {e}")
            return False

    def _analyze_structure(self):
        """分析文档结构"""
        print("正在分析文档结构...")

        structure = {
            'modules': {},
            'html_files': [],
            'index_file': None
        }

        # 查找所有 HTML 文件
        temp_path = Path(self.temp_dir)
        html_files = list(temp_path.rglob("*.html"))
        structure['html_files'] = [str(f) for f in html_files]

        # 查找主索引文件
        index_file = temp_path / "index.html"
        if index_file.exists():
            structure['index_file'] = str(index_file)

        # 分析模块结构
        for html_file in html_files:
            rel_path = html_file.relative_to(temp_path)
            parts = rel_path.parts

            # 识别模块 (baseapp, cellapp, client, etc.)
            if len(parts) >= 2 and parts[0] in ['baseapp', 'cellapp', 'client', 'dbmgr', 'loginapp', 'logger', 'bots', 'interfaces']:
                module_name = parts[0]
                if module_name not in structure['modules']:
                    structure['modules'][module_name] = {
                        'files': [],
                        'classes': {},
                        'functions': []
                    }

                # 分类文件
                if 'Classes' in parts:
                    class_name = html_file.stem
                    structure['modules'][module_name]['classes'][class_name] = str(html_file)
                elif 'Modules' in parts:
                    structure['modules'][module_name]['modules_file'] = str(html_file)
                else:
                    structure['modules'][module_name]['files'].append(str(html_file))

        print(f"找到 {len(structure['html_files'])} 个 HTML 文件")
        print(f"识别 {len(structure['modules'])} 个模块")
        for module_name, module_data in structure['modules'].items():
            print(f"  - {module_name}: {len(module_data['classes'])} 个类, {len(module_data['files'])} 个文件")

        return structure

    def _convert_html_to_markdown(self, structure):
        """转换 HTML 到 Markdown"""
        print("正在转换 HTML 到 Markdown...")

        # 创建输出目录
        api_dir = os.path.join(self.output_base_dir, "api")
        os.makedirs(api_dir, exist_ok=True)

        # 转换主索引文件
        if structure['index_file']:
            self._convert_index_file(structure['index_file'], api_dir)

        # 转换模块文件
        for module_name, module_data in structure['modules'].items():
            module_dir = os.path.join(api_dir, module_name)
            os.makedirs(module_dir, exist_ok=True)

            # 转换类文件
            for class_name, class_file in module_data['classes'].items():
                output_file = os.path.join(module_dir, f"{class_name}.md")
                self._convert_html_file(class_file, output_file, class_name)

            # 转换模块文件
            if 'modules_file' in module_data:
                output_file = os.path.join(module_dir, "README.md")
                self._convert_html_file(module_data['modules_file'], output_file, module_name)

    def _convert_index_file(self, index_file, output_dir):
        """转换索引文件"""
        print(f"转换索引文件: {index_file}")

        try:
            with open(index_file, 'r', encoding='utf-8') as f:
                html_content = f.read()

            soup = BeautifulSoup(html_content, 'html.parser')

            # 提取标题
            title = soup.find('h1')
            title_text = title.get_text() if title else "KBEngine API 文档"

            # 提取内容
            content = soup.find('body')
            if content:
                markdown_content = f"# {title_text}\n\n"
                markdown_content += self._html_to_markdown(content)

                # 保存
                output_file = os.path.join(output_dir, "README.md")
                with open(output_file, 'w', encoding='utf-8') as f:
                    f.write(markdown_content)

                print(f"[SUCCESS] 已创建: {output_file}")

        except Exception as e:
            print(f"转换索引文件失败: {e}")

    def _convert_html_file(self, html_file, output_file, page_title):
        """转换单个 HTML 文件"""
        try:
            with open(html_file, 'r', encoding='utf-8') as f:
                html_content = f.read()

            soup = BeautifulSoup(html_content, 'html.parser')

            # 提取标题
            title = soup.find('h1')
            if title:
                title_text = title.get_text()
            else:
                title_text = page_title

            # 提取内容
            body = soup.find('body')
            if body:
                markdown_content = f"# {title_text}\n\n"
                markdown_content += self._html_to_markdown(body)

                # 保存
                os.makedirs(os.path.dirname(output_file), exist_ok=True)
                with open(output_file, 'w', encoding='utf-8') as f:
                    f.write(markdown_content)

                print(f"[SUCCESS] 已创建: {output_file}")

        except Exception as e:
            print(f"转换文件失败 {html_file}: {e}")

    def _html_to_markdown(self, element):
        """HTML 到 Markdown 转换"""
        if not element:
            return ""

        markdown_parts = []

        for child in element.descendants:
            if child.name == 'h1':
                text = child.get_text().strip()
                if text:
                    markdown_parts.append(f"\n# {text}\n")
            elif child.name == 'h2':
                text = child.get_text().strip()
                if text:
                    markdown_parts.append(f"\n## {text}\n")
            elif child.name == 'h3':
                text = child.get_text().strip()
                if text:
                    markdown_parts.append(f"\n### {text}\n")
            elif child.name == 'p':
                text = child.get_text().strip()
                if text:
                    markdown_parts.append(f"{text}\n\n")
            elif child.name == 'a':
                href = child.get('href', '')
                text = child.get_text().strip()
                if href and text:
                    markdown_parts.append(f"[{text}]({href})")
            elif child.name == 'code':
                text = child.get_text()
                markdown_parts.append(f"`{text}`")
            elif child.name == 'pre':
                code = child.get_text()
                markdown_parts.append(f"\n```\n{code}\n```\n")
            elif child.name == 'ul':
                markdown_parts.append("\n")
            elif child.name == 'ol':
                markdown_parts.append("\n")
            elif child.name == 'li':
                text = child.get_text().strip()
                if text:
                    markdown_parts.append(f"- {text}\n")
            elif child.name == 'table':
                markdown_parts.append(self._convert_table(child))
            elif child.name == 'hr':
                markdown_parts.append("\n---\n")
            elif child.name in ['div', 'span', 'font', 'td', 'tr', 'br']:
                continue  # 忽略这些标签，只处理文本内容
            elif child.string and child.string.strip():
                # 处理纯文本节点
                text = child.string.strip()
                if text and not markdown_parts[-1].endswith(text):
                    markdown_parts.append(text)

        return ''.join(markdown_parts)

    def _convert_table(self, table):
        """转换表格"""
        try:
            rows = table.find_all('tr')
            if not rows:
                return ""

            markdown_table = []

            for row in rows:
                cells = row.find_all(['td', 'th'])
                if cells:
                    row_text = ' | '.join([cell.get_text().strip() for cell in cells])
                    markdown_table.append(f"| {row_text} |")

            if markdown_table:
                # 添加表头分隔符
                if len(markdown_table) > 1:
                    separator = ' | '.join(['---'] * len(markdown_table[0].split(' | ')))
                    markdown_table.insert(1, f"| {separator} |")

                return '\n'.join(markdown_table) + '\n\n'

        except Exception as e:
            print(f"转换表格失败: {e}")

        return ""

    def _create_vuepress_config(self, structure):
        """创建 VuePress 配置"""
        # 这个方法可以根据需要扩展，目前保持简单
        pass


def main():
    """主函数"""
    # 配置
    chm_file = "docs/api/kbengine_api(cn).chm"
    output_dir = "docs-vuepress"

    # 检查文件
    if not os.path.exists(chm_file):
        print(f"错误：文件不存在 - {chm_file}")
        return

    # 创建转换器
    converter = CHMToVuePressConverter(chm_file, output_dir)

    # 执行转换
    try:
        success = converter.convert()

        if success:
            print("\n[SUCCESS] 转换成功完成！")
            print(f"\n下一步：")
            print(f"1. 检查转换结果: ls {output_dir}/api/")
            print(f"2. 启动开发服务器: npm run docs:dev")
            print(f"3. 访问文档查看效果")
        else:
            print("\n[ERROR] 转换失败")

    except Exception as e:
        print(f"\n[ERROR] 转换过程出错: {e}")
        import traceback
        traceback.print_exc()


if __name__ == "__main__":
    main()