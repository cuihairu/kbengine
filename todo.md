# todo — kbengine

## 已完成

- [x] 双语收口（2026-10-16 修复单）：README.md 中文正文移出、英文版补对应内容、首屏顶部居中语言切换器；新建 README.zh.md 收中文原文

## 待办

- [x] 文档双语收口 — `docs/` 根级（2026-10-16 执行单）：根目录 7 篇全部配对完成，中文件 `CODE_COVERAGE_SETUP.md` / `CODECOV_SETUP.md` / `COVERAGE_IMPROVEMENTS.md` / `NETWORK_IO_RESEARCH_README.md` / `python-3.12-upgrade-notes.md` / `vcpkg-migration-status.md` 各出 `.en.md` 英文对应件、`index.md` 出 `index.en.md`（VitePress home 版 frontmatter 已译）；两向语言切换器均已加。注：`index.md` / `index.en.md` 为 `layout: home`，正文区不渲染，切换器放在 frontmatter 之后、正文首行——文件内位置靠前但首页不可见，是 home 布局的结构限制
- [x] 中英双版媒体文件互索引（2026-10-16 R1016F）：`docs/resources/index.md`、`docs/resources/provenance.md` 登记 4 个原始资料文件（`KBEngine overview(cn|en).pptx`、`WebConsole_Guide(cn|en).pdf`），`docs/guide/index.md`、`docs/api/index.md` 各补对应目录内的索引。写法用仓库路径内联代码而非站点链接：这 4 个二进制不在 `docs/public`，VitePress 不发布，链接在站点上必然 404；且 `pptx` 不在 VitePress 已知扩展名白名单，写成 Markdown 链接会被 `docs:build` 判为死链（`pdf` 在白名单内故不受检）
- [ ] 文档双语收口 — `docs/` 子目录（进行中）：入口层已完成 `docs/api/index.md`、`docs/api/bigworld/**`（5 篇）、`docs/architecture/index.md`、`docs/architecture/bigworld/**`（4 篇），各出 `.en.md` 并加两向切换器。剩余：`api/kbengine/**`（26 篇，受 CHM 权威核对保护，见下）、`architecture/source-analysis/**`（23）、`architecture/comparison/**`（4）、`architecture/redesign.md`、`study/**`（34，学习笔记类、优先级低）、`book-legacy/**`（13，低）、`guide/**`（7）、`resources/**`（4）
- [ ] `api/kbengine/**` 双语化（阻塞）：该目录由 `tools/verify_authoritative_docs.py` 从 `kbengine_api(cn).chm` 逐页重建后做**逐字节比对**（`docs:verify` 要求 `API mismatches: 0`）。因此中文页首行不能插入语言切换器，英文版只能作为新增 `.en.md` 独立文件存在，且无法从中文章进入。需要先定一个口径再动手：(a) 在核对脚本里放行首行切换器；(b) 只新增 `.en.md` 并在 `docs/api/index.md` 做目录级索引；(c) 暂缓
