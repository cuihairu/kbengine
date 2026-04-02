#!/bin/bash
# 批量转换文档脚本

set -e

echo "========================================="
echo "KBEngine 文档转换脚本"
echo "========================================="
echo ""

# 检查 Python 依赖
echo "检查 Python 依赖..."
python3 -c "import bs4" 2>/dev/null || {
    echo "错误：缺少 beautifulsoup4 库"
    echo "请运行: pip install beautifulsoup4 lxml pdfplumber"
    exit 1
}

python3 -c "import pdfplumber" 2>/dev/null || {
    echo "错误：缺少 pdfplumber 库"
    echo "请运行: pip install pdfplumber"
    exit 1
}

echo "✓ Python 依赖检查通过"
echo ""

# 转换 HTML 文档
echo "========================================="
echo "步骤 1/2: 转换 HTML 文档"
echo "========================================="
if [ -f "docs/kbengine_api(cn).html" ]; then
    python3 tools/html_splitter.py
    echo "✓ HTML 文档转换完成"
else
    echo "警告：未找到 HTML 文档 - docs/kbengine_api(cn).html"
fi
echo ""

# 转换 PDF 文档
echo "========================================="
echo "步骤 2/2: 转换 PDF 文档"
echo "========================================="
if [ -f "docs/WebConsole_Guide(cn).pdf" ]; then
    python3 tools/pdf_extractor.py
    echo "✓ PDF 文档转换完成"
else
    echo "警告：未找到 PDF 文档 - docs/WebConsole_Guide(cn).pdf"
fi
echo ""

echo "========================================="
echo "转换完成！"
echo "========================================="
echo ""
echo "下一步："
echo "1. 检查转换结果: ls docs-vuepress/guide/ docs-vuepress/api/"
echo "2. 安装 Node.js 依赖: npm install"
echo "3. 启动开发服务器: npm run docs:dev"
echo ""
