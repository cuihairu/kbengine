# 资料与说明

> 本页汇总当前整理所依赖的仓库内资料，并明确区分“已按原始资料核对”的页面与“人工整理分析页”。

## 当前资料

- 协议文件：`LICENSE.txt`
- KBEngine API 中文文档：`docs/api/kbengine/kbengine_api(cn).chm`
- BigWorld 源码与资料：`BigWorld-Engine-14.4.1/`
- WebConsole 中文文档：`docs/WebConsole_Guide(cn).pdf`
- 仓库说明：`README.md`

## 说明

- `api/kbengine/**` 与 `guide/webconsole.md` 已有原始资料可对照。
- `api/bigworld/**` 是基于 BigWorld 源码注释和绑定点整理的源码抽取版 API。
- `study/**` 与 `architecture/**` 主要属于人工整理、源码分析与设计说明，不是 CHM/PDF 原文转写。
- 本站整理时优先以仓库内现有资料与源码为准。
- 新增说明性内容需要逐项核对后再补充。
- [文档来源边界与核对状态](/resources/provenance.md)
- [文档核对结果](/resources/verification-status.md)
- 本地可使用 `npm run docs:verify` 复核 API 与 WebConsole 页面是否仍和 CHM/PDF 保持一致。
