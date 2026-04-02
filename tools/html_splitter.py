#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
HTML 文档解析器
将 KBEngine API HTML 文档转换为 VuePress Markdown 格式
"""

import os
import re
from bs4 import BeautifulSoup
from html.parser import HTMLParser


class HTMLToMarkdownConverter:
    """HTML 到 Markdown 转换器"""

    def __init__(self, html_file_path):
        self.html_file_path = html_file_path
        self.output_dir = None
        self.current_section = None

    def convert(self, output_dir):
        """
        转换 HTML 文档为 Markdown

        Args:
            output_dir: 输出目录
        """
        self.output_dir = output_dir
        os.makedirs(output_dir, exist_ok=True)

        print(f"正在读取 HTML 文件: {self.html_file_path}")

        # 使用迭代解析处理大文件
        with open(self.html_file_path, 'r', encoding='utf-8') as f:
            html_content = f.read()

        # 解析 HTML
        soup = BeautifulSoup(html_content, 'lxml')

        # 提取文档结构
        self._extract_structure(soup)

    def _extract_structure(self, soup):
        """提取文档结构"""
        # 查找主要内容区域
        content = self._find_main_content(soup)

        if not content:
            print("警告：未找到主要内容区域")
            return

        # 提取标题和内容
        self._extract_sections(content)

    def _find_main_content(self, soup):
        """查找主要内容区域"""
        # 尝试常见的内容区域选择器
        selectors = [
            'body',
            'div.content',
            'div.main',
            'article',
            'main',
        ]

        for selector in selectors:
            element = soup.select_one(selector)
            if element:
                return element

        return soup

    def _extract_sections(self, content):
        """提取章节内容"""
        # 查找所有标题
        headings = content.find_all(['h1', 'h2', 'h3', 'h4', 'h5', 'h6'])

        if not headings:
            print("警告：未找到任何标题")
            # 如果没有标题，提取所有段落
            self._extract_full_content(content)
            return

        # 根据标题组织内容
        current_file = None
        current_content = []

        for element in content.descendants:
            if element.name in ['h1', 'h2', 'h3', 'h4', 'h5', 'h6']:
                # 保存当前文件
                if current_file and current_content:
                    self._write_file(current_file, current_content)

                # 创建新文件
                heading_text = self._clean_text(element.get_text())
                filename = self._text_to_filename(heading_text)
                current_file = os.path.join(self.output_dir, f"{filename}.md")
                current_content = []

                # 添加标题
                level = int(element.name[1])  # h1 -> 1, h2 -> 2
                current_content.append(f"{'#' * level} {heading_text}\n")

            elif current_file:
                # 提取内容
                text = self._extract_element_content(element)
                if text:
                    current_content.append(text)

        # 保存最后一个文件
        if current_file and current_content:
            self._write_file(current_file, current_content)

    def _extract_full_content(self, content):
        """提取完整内容（当没有标题时）"""
        output_file = os.path.join(self.output_dir, "api-reference.md")
        content_lines = []

        for element in content.descendants:
            text = self._extract_element_content(element)
            if text:
                content_lines.append(text)

        if content_lines:
            self._write_file(output_file, content_lines)

    def _extract_element_content(self, element):
        """提取元素内容"""
        if element.name == 'p':
            text = self._clean_text(element.get_text())
            return f"{text}\n" if text else ""

        elif element.name == 'pre':
            code = element.get_text()
            language = self._detect_code_language(code)
            return f"\n```{language}\n{code}\n```\n"

        elif element.name == 'code':
            return f"`{element.get_text()}`"

        elif element.name == 'ul':
            items = [f"- {self._clean_text(li.get_text())}" for li in element.find_all('li', recursive=False)]
            return "\n".join(items) + "\n" if items else ""

        elif element.name == 'ol':
            items = [f"{i}. {self._clean_text(li.get_text())}" for i, li in enumerate(element.find_all('li', recursive=False), 1)]
            return "\n".join(items) + "\n" if items else ""

        elif element.name == 'table':
            return self._convert_table(element)

        elif element.name == 'a':
            href = element.get('href', '')
            text = self._clean_text(element.get_text())
            return f"[{text}]({href})" if href and text else text

        elif element.name == 'strong' or element.name == 'b':
            return f"**{element.get_text()}**"

        elif element.name == 'em' or element.name == 'i':
            return f"*{element.get_text()}*"

        return ""

    def _convert_table(self, table):
        """转换表格为 Markdown"""
        rows = []
        for row in table.find_all('tr'):
            cells = [self._clean_text(cell.get_text()) for cell in row.find_all(['td', 'th'])]
            rows.append(' | '.join(cells))

        if rows:
            # 添加表头分隔符
            if len(rows) > 1:
                separator = ' | '.join(['---'] * len(rows[0].split(' | ')))
                rows.insert(1, separator)
            return '\n'.join(rows) + '\n'

        return ""

    def _clean_text(self, text):
        """清理文本"""
        if not text:
            return ""
        # 移除多余的空白字符
        text = re.sub(r'\s+', ' ', text)
        return text.strip()

    def _text_to_filename(self, text):
        """将文本转换为文件名"""
        # 移除特殊字符
        text = re.sub(r'[^\w\s-]', '', text)
        # 替换空格为连字符
        text = re.sub(r'[-\s]+', '-', text)
        # 转换为小写
        text = text.lower().strip('-')
        return text if text else "untitled"

    def _detect_code_language(self, code):
        """检测代码语言"""
        if 'def ' in code or 'import ' in code:
            return 'python'
        elif '::' in code or '#include' in code:
            return 'cpp'
        elif 'function' in code or 'var ' in code:
            return 'javascript'
        return ''

    def _write_file(self, filepath, content):
        """写入文件"""
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write('\n'.join(content))
        print(f"已创建: {filepath}")


def main():
    """主函数"""
    # 配置路径
    html_file = "docs/kbengine_api(cn).html"
    output_dir = "docs-vuepress/api"

    # 检查文件是否存在
    if not os.path.exists(html_file):
        print(f"错误：文件不存在 - {html_file}")
        return

    # 创建转换器
    converter = HTMLToMarkdownConverter(html_file)

    # 执行转换
    print(f"开始转换: {html_file}")
    converter.convert(output_dir)
    print(f"转换完成！输出目录: {output_dir}")


if __name__ == "__main__":
    main()
