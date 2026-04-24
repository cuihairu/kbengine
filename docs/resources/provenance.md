# 文档来源边界与核对状态

> 本页用于区分哪些页面直接来自仓库内原始资料，哪些页面属于人工整理或设计草案。

## 直接来源于原始资料的页面

- `docs-vuepress/api/**`
  来源：`docs/api/kbengine_api(cn).chm`
- `docs-vuepress/guide/webconsole.md`
  来源：`docs/WebConsole_Guide(cn).pdf`

## 站点导航与汇总页

- `docs-vuepress/README.md`
- `docs-vuepress/guide/README.md`
- `docs-vuepress/api/README.md`
- `docs-vuepress/resources/README.md`

这些页面用于组织目录、汇总入口，不是原始资料逐页转写页。

## 人工整理或草案页

- `docs-vuepress/architecture/README.md`
- `docs-vuepress/architecture/bigworld/**`
- `docs-vuepress/architecture/source-analysis/**`
- `docs-vuepress/architecture/comparison/**`
- `docs-vuepress/architecture/redesign.md`
- `docs-vuepress/study/**`

这些页面不来自 CHM/PDF 原文，阅读时应单独看待。

## 当前核对结论

- API 目录在提交 `d2c4d0f11`（2026-04-02）和 `ac97f19e2`（2026-04-02）中标记为从 CHM 重建。
- WebConsole 指南由 `tools/rebuild_docs.py` 中的 `build_pdf_markdown()` 从 PDF 提取生成。
- `architecture` 目录不是脚本从 CHM/PDF 提取的结果，必须按源码或原始资料另行核对。
- `study` 目录是围绕源码的结构化讲解主线，不是 CHM/PDF 的逐页重建结果。
- 最新逐页核对结果见 [verification-status.md](/resources/verification-status.md)。

## 后续处理原则

- 对 API 关键页，按 CHM 原文逐页抽检并补充核对记录。
- 对 WebConsole 指南，按 PDF 原文逐节抽检。
- 对 `study` / `architecture` 这类非原始资料页，必须显式标注“人工整理”或“设计草案”，并优先给出源码位置。
