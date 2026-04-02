@echo off
REM 启动 VuePress 开发服务器（无警告模式）

SET SILENCE_DEPRECATIONS=*
SET VERBOSE=false

echo Starting VuePress development server without warnings...
echo.
npm run docs:dev
