# Plan: Ch1 导读与阅读方法

## 目标

编写 `docs-vuepress/book/01-introduction-and-method.md`，替换旧版本。

## 写作风格
- 中英混合：关键术语附英文原文括号，如 实体定义（EntityDef）
- 嵌入关键代码片段：10-30 行核心代码
- 简洁、专业、不废话

## 文件结构

输出文件：`docs-vuepress/book/01-introduction-and-method.md`

## 约束
- 必须替换旧的 `01-introduction-and-method.md`
- 新版目录已在 `README.md` 中定义（Ch1 导读与阅读方法）
- 保留旧版中好的部分（每章统一结构、阅读三遍法、源码地址）
- 新增内容：项目目录导航、VuePress 配置说明、如何使用源码链接

- 注意：现有其他 .md 文件（02-14、appendix）编号是旧版，新版目录重新编号为 Ch1-Ch24 + 附录A-C，需要决定是否重命名所有文件

## 旧版保留的内容
- 每章统一结构（6 小节）
- 阅读三遍法
- 源码地址

## 新增内容
- 项目目录结构导航
- 两套源码的浏览建议（IDE 配置、跳转路径）
- VuePress 文档使用说明
- 章节依赖关系图（哪章依赖哪章）

## Ch1 大纲

```
# 1. 导读与阅读方法

## 1.1 这本书是什么

- 一句话定位
- 不是 API 手册，不是架构图集，是"为什么这样设计 + 代码在哪里"的结构化源码学习路径
- 覆盖范围：BigWorld Engine 14.4.1 + KBEngine 的完整源码
- 适合谁：有 C++ 基础、想理解 MMO 服务器引擎设计的人

## 1.2 这本书的组织方式

- 9 个 Part、24 章、2 个附录
- Part I 为什么长这样（Ch1-3）：建立心智模型
- Part II 运行骨架（Ch4-6）：组件启动、实体定义、脚本桥接
- Part III 基础设施层（Ch7-9）：并发、网络、分布式
- Part IV 通信与协作（Ch10-13）：序列化、RPC、广播、持久化
- Part V 空间与运动（Ch14-17）：AOI、拓扑、寻路、Ghost
- Part VI 脚本层行为（Ch18）：钩子、回调、定时器
- Part VII 前后端交互（Ch19）：客户端协议
- Part VIII 运维调试（Ch20-21）：监控、热更新
- Part IX 串联实战（Ch22-24）：生命周期、对照、走读
- 每章统一结构：
  1. 本章核心问题
  2. BigWorld 背景 / KBEngine 实现
  3. 关键源码入口
  4. 关键调用链
  5. 小结
  6. 待追踪问题

## 1.3 两套源码在哪里

- BigWorld: https://github.com/cuihairu/BigWorld-Engine-14.4.1
  - 核心目录：programming/bigworld/server/（服务端）、lib/（库）
- KBEngine: 本地 kbe/ 目录
  - 核心目录：kbe/src/server/（服务端）、kbe/src/lib/（库）

## 1.4 推荐阅读方式

- 第一遍：按顺序通读，建立整体模型
- 第二遍：配合 IDE 看每章列出的源码入口
  - 推荐 IDE：VS Code + C/C++ 扩展
  - 跳转配置：includePath 添加两套项目的 src/ 目录
- 第三遍：围绕单一主题深挖
  - 例如：RPC、持久化、AOI
- 每章末尾都有"建议源码走读路径"，可以直接按路径打开文件

## 1.5 源码链接约定

- BigWorld 源码链接格式：[BW:server/cellapp/witness.cpp]
- KBEngine 源码链接格式：[KBE:kbe/src/server/cellapp/witness.cpp]
- GitHub 可直接点击跳转（BigWorld）
- 本地文件可直接用 IDE 打开（KBEngine）

## 1.6 章节依赖关系

- Ch4-6 依赖 Ch2-3（先知道系统长什么样，再看运行骨架）
- Ch7-9 依赖 Ch4（先知道启动流程，再看基础设施）
- Ch10-13 依赖 Ch5+8（先知道 EntityDef + 网络层，再看通信）
- Ch14-17 依赖 Ch10+13（先知道 RPC + 持久化，再看空间）
- Ch18 依赖 Ch6（先知道脚本桥接，再看钩子语义）
- Ch19 依赖 Ch10-12（先知道服务端协议，再看客户端视角）
- Ch20-21 依赖全部前置章节
- Ch22-24 依赖全部前置章节
```

## 执行步骤

1. 写入 `docs-vuepress/book/01-introduction-and-method.md`（覆盖旧版）
2. 不需要修改其他文件（Ch1 是独立章节）
3. 注意：当前旧版文件编号（01-18）与新版目录不对应，后续章节需要重命名——但 Ch1 先独立完成，重命名作为后续任务
