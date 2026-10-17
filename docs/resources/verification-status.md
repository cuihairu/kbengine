# 文档核对结果

> 最后核对时间：2026-04-05

## 核对范围

- KBEngine API 原始资料：`docs/api/kbengine/kbengine_api(cn).chm`
- BigWorld API 来源：`BigWorld-Engine-14.4.1` 源码注释与 Python 绑定点
- WebConsole 原始资料：`docs/WebConsole_Guide(cn).pdf`

## 核对方法

- API 页面使用本地 `pychm + chmlib` 直接读取 CHM 内 HTML。
- 读取出的 HTML 再复用 `tools/rebuild_docs.py` 中的 `HtmlConverter` 重新生成 Markdown。
- 13 个直通页与 CHM 生成结果逐字对比。
- `basetypes.md`、`keywords.md` 和 9 个已按源码行为增强的 API 页采用标题签名对比：保留 CHM 接口骨架，允许语义与源码解析增强。
- `guide/webconsole.md` 使用 `tools/rebuild_docs.py` 中的 `build_pdf_markdown()` 重新从 PDF 生成后，与当前页面逐字对比。

## 当前结果

- API 总页数：24（13 exact + 11 audited enhancements）
- 直通页与 CHM 转换结果完全一致：13 页
- 增强页与 CHM 接口签名一致：11 页
- 校验失败页数：0 页
- WebConsole 指南与 PDF 转换结果完全一致
- `study/**` 与 `architecture/**` 不在本命令的逐页精确比对范围内

## API 校验失败页面

- 当前无

## 差异性质

- 当前直通页与 CHM 转换结果逐字一致。
- 当前增强页保留 CHM 接口骨架，增强内容以源码行为与仓库源码解析为准。
- 模块内跳转依赖 VuePress 侧边栏提供，不再向原文页追加人工导航块。
- `study` 与 `architecture` 的正确性需要额外依赖源码核读与人工审查，不能用“与 CHM/PDF 完全一致”来表述。

## 关键页状态

- `/api/kbengine/client/KBEngine.md` 已直接对到 CHM
- `/api/kbengine/baseapp/KBEngine.md` 已直接对到 CHM
- `/api/kbengine/cellapp/KBEngine.md` 已直接对到 CHM
- `/guide/webconsole.md` 已直接对到 PDF

## 本地复核命令

```bash
npm run docs:verify
```

如果命令输出 `API mismatches: 0`、`API enhanced signature matches: 11` 且 `WebConsole exact match: True`，说明当前站点正文仍与原始资料核对策略保持一致。
