# BigWorld API

> 本目录用于整理 `BigWorld-Engine-14.4.1` 的脚本 API。当前仓库没有等价于 KBEngine CHM 的 BigWorld API 原文包，因此这里先建立“源码抽取版 API”入口，后续逐步从源码注释和 Python 绑定点补齐。

## 来源边界

BigWorld API 的优先来源是：

- `BigWorld-Engine-14.4.1/programming/bigworld/server/**` 中的 `/*~ module ... */`、`/*~ class ... */`、`/*~ function ... */`、`/*~ attribute ... */` 注释
- `PY_AUTO_MODULE_FUNCTION`、`PY_METHOD`、`PY_ATTRIBUTE`、`PY_RO_ATTRIBUTE_DECLARE` 等 Python 绑定宏
- `BigWorld-Engine-14.4.1/docs/pdf/**` 中的服务器白皮书、安装指南、构建指南只作为背景资料，不当作逐项 API 原文

## 模块入口

- [BigWorld 模块](/api/bigworld/bigworld-module.md)
- [Base 实体](/api/bigworld/base.md)
- [Cell Entity](/api/bigworld/entity.md)
- [BWPersonality 回调](/api/bigworld/bwpersonality.md)

## 阅读方式

BigWorld 和 KBEngine 有大量同源概念，例如 Base/Cell、Mailbox/EntityCall、timer、SpaceData、`reloadScript`。阅读 BigWorld API 时不要直接套用 KBEngine 的组件名和语义，应同时参考：

- [BigWorld / KBEngine 对照](/study/23-bigworld-and-kbengine-comparison.md)
- [热更新](/study/21-hotupdate-fault-tolerance-and-ops.md)
- [脚本层源码分析](/architecture/source-analysis/scripting.md)
