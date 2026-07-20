# 文档说明

> 这里不是学习入口，而是文档治理区。只放来源边界、核对状态、结构设计这类“关于文档本身”的信息。

## 你在这里能看到什么

- [文档来源边界与核对状态](/resources/provenance.md)
- [文档核对结果](/resources/verification-status.md)
- [文档结构设计](/resources/doc-structure.md)

## 原始资料范围

- KBEngine API 中文文档：`docs/api/kbengine/kbengine_api(cn).chm`
- WebConsole 中文文档：`docs/WebConsole_Guide(cn).pdf`
- BigWorld 源码与资料：`BigWorld-Engine-14.4.1/`
- 仓库说明：`README.md`

## 当前边界

- `api/kbengine/**` 与 `guide/webconsole.md` 可以直接对原始资料核对。
- `api/bigworld/**` 是基于 BigWorld 源码注释和绑定点整理的源码抽取版 API。
- `study/**`、`architecture/**`、`resources/doc-structure.md` 属于人工整理与结构设计，不是 CHM/PDF 原文转写。

## 本地校验

```bash
npm run docs:verify
```

如果输出 `API mismatches: 0` 且 `WebConsole exact match: True`，说明当前 API 与 WebConsole 正文仍和原始资料一致。
