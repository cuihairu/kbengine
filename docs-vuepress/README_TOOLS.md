# KBEngine 文档系统

这是 KBEngine 项目的 VuePress 文档系统。

## 快速开始

### 1. 安装依赖

```bash
# 安装 Node.js 依赖
npm install

# 安装 Python 依赖
pip install beautifulsoup4 lxml pdfplumber
```

### 2. 转换文档

将现有的 HTML 和 PDF 文档转换为 Markdown 格式：

```bash
# Windows
tools\convert_all.bat

# Linux/Mac
bash tools/convert_all.sh
```

### 3. 本地开发

```bash
npm run docs:dev
```

文档将在 http://localhost:8080 启动。

### 4. 构建生产版本

```bash
npm run docs:build
```

构建产物将输出到 `docs-vuepress/.vuepress/dist` 目录。

## 项目结构

```
docs-vuepress/
├── .vuepress/          # VuePress 配置目录
│   ├── config.ts       # 主配置文件
│   └── public/         # 静态资源
├── guide/              # 指南文档
│   ├── README.md       # 指南首页
│   ├── installation.md
│   ├── configuration.md
│   └── webconsole.md
├── api/                # API 文档
│   ├── README.md
│   ├── logger.md
│   └── entity.md
├── tutorial/           # 教程
│   └── README.md
├── en/                 # 英文文档
│   ├── README.md
│   ├── guide/
│   ├── api/
│   └── tutorial/
└── README.md           # 首页
```

## 文档转换工具

### HTML 解析器 (`tools/html_splitter.py`)

将 KBEngine API HTML 文档转换为 Markdown 格式：

```bash
python tools/html_splitter.py
```

### PDF 提取器 (`tools/pdf_extractor.py`)

将 WebConsole PDF 指南转换为 Markdown 格式：

```bash
python tools/pdf_extractor.py
```

### 批量转换脚本

一次性转换所有文档：

- Windows: `tools\convert_all.bat`
- Linux/Mac: `bash tools/convert_all.sh`

## 部署

文档会自动通过 GitHub Actions 部署到 GitHub Pages。

部署地址：https://cuihairu.github.io/kbengine/

## 贡献指南

1. 编辑 Markdown 文件
2. 运行 `npm run docs:dev` 预览
3. 提交 Pull Request

## 许可证

MIT
