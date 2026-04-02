@echo off
REM 批量转换文档脚本 (Windows)

echo =========================================
echo KBEngine 文档转换脚本
echo =========================================
echo.

REM 检查 Python 依赖
echo 检查 Python 依赖...
python -c "import bs4" 2>nul || (
    echo 错误：缺少 beautifulsoup4 库
    echo 请运行: pip install beautifulsoup4 lxml pdfplumber
    exit /b 1
)

python -c "import pdfplumber" 2>nul || (
    echo 错误：缺少 pdfplumber 库
    echo 请运行: pip install pdfplumber
    exit /b 1
)

echo [OK] Python 依赖检查通过
echo.

REM 转换 HTML 文档
echo =========================================
echo 步骤 1/2: 转换 HTML 文档
echo =========================================
if exist "docs\kbengine_api(cn).html" (
    python tools\html_splitter.py
    echo [OK] HTML 文档转换完成
) else (
    echo 警告：未找到 HTML 文档 - docs\kbengine_api(cn).html
)
echo.

REM 转换 PDF 文档
echo =========================================
echo 步骤 2/2: 转换 PDF 文档
echo =========================================
if exist "docs\WebConsole_Guide(cn).pdf" (
    python tools\pdf_extractor.py
    echo [OK] PDF 文档转换完成
) else (
    echo 警告：未找到 PDF 文档 - docs\WebConsole_Guide(cn).pdf
)
echo.

echo =========================================
echo 转换完成！
echo =========================================
echo.
echo 下一步：
echo 1. 检查转换结果
echo 2. 安装 Node.js 依赖: npm install
echo 3. 启动开发服务器: npm run docs:dev
echo.

pause
