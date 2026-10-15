# todo — kbengine

## 已完成

- [x] 双语收口（2026-10-16 修复单）：README.md 中文正文移出、英文版补对应内容、首屏顶部居中语言切换器；新建 README.zh.md 收中文原文

## 待办

- [x] 文档双语收口 — `docs/` 根级（2026-10-16 执行单）：根目录 7 篇全部配对完成，中文件 `CODE_COVERAGE_SETUP.md` / `CODECOV_SETUP.md` / `COVERAGE_IMPROVEMENTS.md` / `NETWORK_IO_RESEARCH_README.md` / `python-3.12-upgrade-notes.md` / `vcpkg-migration-status.md` 各出 `.en.md` 英文对应件、`index.md` 出 `index.en.md`（VitePress home 版 frontmatter 已译）；两向语言切换器均已加。注：`index.md` / `index.en.md` 为 `layout: home`，正文区不渲染，切换器放在 frontmatter 之后、正文首行——文件内位置靠前但首页不可见，是 home 布局的结构限制
- [ ] 文档双语收口 — `docs/` 子目录（下一轮）：123 篇仍为中文单语、无英文版也无语言切换器（api 32、architecture 33（含 source-analysis 23）、study 34、book-legacy 13、guide 12、resources 4 等）。建议路径：先补 api / architecture 的英文版或页面首屏切换器，study 与 book-legacy 归为学习笔记类、优先级低
- [ ] `docs/KBEngine overview(cn).pptx` / `(en).pptx` 与 `docs/WebConsole_Guide(cn).pdf` / `(en).pdf` 已有中英双版，但未在仓库文档里互相索引，建议在各目录 index.md 补链接
