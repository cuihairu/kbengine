#!/usr/bin/env bash

set -euo pipefail

echo "========================================="
echo "KBEngine 文档重建脚本"
echo "========================================="
echo

python3 -c "import bs4, pdfplumber" >/dev/null 2>&1 || {
  echo "缺少 Python 依赖，请先执行:"
  echo "  pip install beautifulsoup4 pdfplumber"
  exit 1
}

[ -f "docs/api/kbengine/kbengine_api(cn).chm" ] || {
  echo '未找到 CHM 源文件: "docs/api/kbengine/kbengine_api(cn).chm"'
  exit 1
}

[ -f "docs/WebConsole_Guide(cn).pdf" ] || {
  echo '未找到 PDF 源文件: "docs/WebConsole_Guide(cn).pdf"'
  exit 1
}

python3 "tools/rebuild_docs.py"

echo
echo "文档重建完成。"
echo "下一步:"
echo "  1. npm install"
echo "  2. npm run docs:dev"
echo "  3. npm run docs:build"
