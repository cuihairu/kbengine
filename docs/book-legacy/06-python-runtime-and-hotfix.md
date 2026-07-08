# 6. Python 运行时与脚本桥接

> 本文件是旧提纲，正文已迁移到 [`06-python-runtime-and-script-bridge.md`](/study/06-python-runtime-and-script-bridge.html)。阅读主线请直接进入新稿。

## 本章核心问题

- Python 为什么在 KBEngine 里占这么核心的位置？
- C++ 与 Python 的桥接是怎样工作的？
- `reloadScript` 一类热更新机制的边界在哪里？

## 建议重点源码

- `kbe/src/lib/pyscript`
- `kbe/src/lib/python`
- `kbe/src/server/baseapp/entity_component.*`
- `kbe/src/server/cellapp/entity_component.*`

## 建议覆盖的内容

- Python 运行时初始化
- 脚本对象创建
- 桥接调用链
- 热更新
- 优势与代价

## 与钩子章节的关系

- 本章关注“脚本运行时如何建立”。
- 具体到 `onInit`、`onReadyForLogin`、`onBaseAppReady`、`onGlobalData`、`addTimer`、`registerEvent` 等钩子的语义与调用时机，统一放到下一章。
