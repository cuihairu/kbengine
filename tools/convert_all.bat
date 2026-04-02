@echo off
setlocal

echo =========================================
echo KBEngine 文档重建脚本
echo =========================================
echo.

python -c "import bs4, pdfplumber" 2>nul || (
    echo 缺少 Python 依赖，请先执行:
    echo   pip install beautifulsoup4 pdfplumber
    exit /b 1
)

if not exist "docs\api\kbengine_api(cn).chm" (
    echo 未找到 CHM 源文件: "docs\api\kbengine_api(cn).chm"
    exit /b 1
)

if not exist "docs\WebConsole_Guide(cn).pdf" (
    echo 未找到 PDF 源文件: "docs\WebConsole_Guide(cn).pdf"
    exit /b 1
)

python "tools\rebuild_docs.py"
if errorlevel 1 exit /b %errorlevel%

echo.
echo 文档重建完成。
echo 下一步:
echo   1. npm install
echo   2. npm run docs:dev
echo   3. npm run docs:build
