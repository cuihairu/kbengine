# 文档核对结果

> 最后核对时间：2026-04-05

## 核对范围

- API 原始资料：`docs/api/kbengine_api(cn).chm`
- WebConsole 原始资料：`docs/WebConsole_Guide(cn).pdf`

## 核对方法

- API 页面使用本地 `pychm + chmlib` 直接读取 CHM 内 HTML。
- 读取出的 HTML 再复用 `tools/rebuild_docs.py` 中的 `HtmlConverter` 重新生成 Markdown。
- 生成结果逐页对比当前 `docs-vuepress/api/**`。
- `guide/webconsole.md` 使用 `tools/rebuild_docs.py` 中的 `build_pdf_markdown()` 重新从 PDF 生成后，与当前页面逐字对比。

## 当前结果

- API 总页数：24
- 与 CHM 转换结果完全一致：24 页
- 与 CHM 转换结果不完全一致：0 页
- WebConsole 指南与 PDF 转换结果完全一致
- `study/**` 与 `architecture/**` 不在本命令的逐页精确比对范围内

## API 不完全一致页面

- 当前无

## 差异性质

- 当前 API 正文页与 CHM 转换结果完全一致。
- 模块内跳转依赖 VuePress 侧边栏提供，不再向原文页追加人工导航块。
- `study` 与 `architecture` 的正确性需要额外依赖源码核读与人工审查，不能用“与 CHM/PDF 完全一致”来表述。

## 关键页状态

- `/api/client/KBEngine.md` 已直接对到 CHM
- `/api/baseapp/KBEngine.md` 已直接对到 CHM
- `/api/cellapp/KBEngine.md` 已直接对到 CHM
- `/guide/webconsole.md` 已直接对到 PDF

## 本地复核命令

```bash
npm run docs:verify
```

如果命令输出 `API mismatches: 0` 且 `WebConsole exact match: True`，说明当前站点正文仍与原始资料保持一致。
