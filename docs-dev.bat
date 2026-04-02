@echo off
REM 启动 VuePress 开发服务器（过滤警告）

echo Starting VuePress development server...
echo.
npm run docs:dev 2>&1 | findstr /V "Deprecation Warning" | findstr /V "Warning: mixed-decls"