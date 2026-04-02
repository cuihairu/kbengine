#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
PDF 文档提取器
将 KBEngine PDF 文档转换为 VuePress Markdown 格式
"""

import os
import re


class PDFExtractor:
    """PDF 内容提取器"""

    def __init__(self, pdf_file_path):
        self.pdf_file_path = pdf_file_path
        self.output_dir = None

    def extract(self, output_dir):
        """
        提取 PDF 内容并转换为 Markdown

        Args:
            output_dir: 输出目录
        """
        self.output_dir = output_dir
        os.makedirs(output_dir, exist_ok=True)

        print(f"正在读取 PDF 文件: {self.pdf_file_path}")

        try:
            import pdfplumber
        except ImportError:
            print("错误：需要安装 pdfplumber 库")
            print("请运行: pip install pdfplumber")
            return

        with pdfplumber.open(self.pdf_file_path) as pdf:
            print(f"PDF 总页数: {len(pdf.pages)}")

            # 提取所有页面的内容
            all_content = []
            for page_num, page in enumerate(pdf.pages, 1):
                print(f"正在处理第 {page_num}/{len(pdf.pages)} 页")
                content = self._extract_page(page, page_num)
                if content:
                    all_content.append(content)

            # 保存为单个 Markdown 文件
            output_file = os.path.join(output_dir, "webconsole.md")
            self._save_markdown(output_file, all_content)

    def _extract_page(self, page, page_num):
        """提取单页内容"""
        content = {
            'text': page.extract_text(),
            'tables': page.extract_tables(),
            'images': page.images,
        }

        markdown_parts = []

        # 提取文本
        if content['text']:
            lines = content['text'].split('\n')
            for line in lines:
                line = line.strip()
                if line:
                    # 尝试识别标题
                    if self._is_heading(line):
                        level = self._get_heading_level(line)
                        markdown_parts.append(f"\n{'#' * level} {line}\n")
                    else:
                        markdown_parts.append(f"{line}\n")

        # 提取表格
        if content['tables']:
            for table in content['tables']:
                markdown_table = self._convert_table(table)
                if markdown_table:
                    markdown_parts.append(f"\n{markdown_table}\n")

        return markdown_parts

    def _is_heading(self, text):
        """判断是否为标题"""
        # 简单的启发式规则
        if not text:
            return False

        # 短文本且可能是标题
        if len(text) < 100 and text.strip():
            # 常见标题模式
            patterns = [
                r'^第.+章',
                r'^\d+\.',
                r'^[一二三四五六七八九十]+、',
                r'^【.*】$',
            ]
            for pattern in patterns:
                if re.match(pattern, text.strip()):
                    return True

        return False

    def _get_heading_level(self, text):
        """获取标题级别"""
        text = text.strip()

        # 根据模式判断级别
        if re.match(r'^第.+章', text):
            return 1
        elif re.match(r'^\d+\.', text):
            return 2
        elif re.match(r'^[一二三四五六七八九十]+、', text):
            return 3
        else:
            return 2

    def _convert_table(self, table):
        """转换表格为 Markdown"""
        if not table or not table[0]:
            return ""

        rows = []
        for row in table:
            cells = [str(cell) if cell is not None else "" for cell in row]
            rows.append(' | '.join(cells))

        if rows:
            # 添加表头分隔符
            separator = ' | '.join(['---'] * len(rows[0].split(' | ')))
            rows.insert(1, separator)
            return '\n'.join(rows)

        return ""

    def _save_markdown(self, filepath, content_parts):
        """保存为 Markdown 文件"""
        # 合并所有内容
        all_content = []
        for parts in content_parts:
            all_content.extend(parts)

        # 添加文件头
        header = "---\n"
        header += "title: WebConsole 使用指南\n"
        header += "---\n\n"

        all_content.insert(0, header)

        # 写入文件
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write('\n'.join(all_content))

        print(f"已创建: {filepath}")


def main():
    """主函数"""
    # 配置路径
    pdf_file = "docs/WebConsole_Guide(cn).pdf"
    output_dir = "docs-vuepress/guide"

    # 检查文件是否存在
    if not os.path.exists(pdf_file):
        print(f"错误：文件不存在 - {pdf_file}")
        return

    # 检查依赖
    try:
        import pdfplumber
    except ImportError:
        print("错误：需要安装 pdfplumber 库")
        print("请运行: pip install pdfplumber")
        return

    # 创建提取器
    extractor = PDFExtractor(pdf_file)

    # 执行提取
    print(f"开始提取: {pdf_file}")
    extractor.extract(output_dir)
    print(f"提取完成！输出目录: {output_dir}")


if __name__ == "__main__":
    main()
