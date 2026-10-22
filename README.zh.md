# KBEngine

<p align="center">
<a href="https://github.com/cuihairu/kbengine/releases"><img alt="Status" src="https://img.shields.io/badge/status-in%20development-orange"></a>
</p>

> [English](README.md) | **中文**

> 英文 README（开发分叉说明、构建指南、排障）见 [README.md](README.md)。
> 本页保留中文项目介绍与社区入口。

## 社区

[官网](https://kbengine.github.io/cn/)，[论坛](https://github.com/kbengine/kbengine/discussions)，QQ交流群：461368412、16535321、716679842

## 什么是KBEngine?

一款 MMOG 游戏服务端引擎：

- 仅 Python 脚本即可简单高效地完成任何游戏逻辑（支持热更新）；
- 使用配套客户端插件能够快速与（Unity3D、UE4、OGRE、HTML5 等等）结合形成一个完整的客户端；
- 引擎使用 C++ 编写，开发者无需重复实现游戏服务端通用的底层技术，可把精力真正集中到游戏开发层面，稳定可靠且快速地打造各种网络游戏。

（经常被问到承载上限：KBEngine 底层架构被设计为多进程分布式动态负载均衡方案，理论上只需要不断扩展硬件就能够不断增加承载上限，单台机器的承载上限取决于游戏逻辑本身的复杂度。）

## 本仓库

这是 [kbengine/kbengine](https://github.com/kbengine/kbengine) 的开发分叉，用于向上游贡献改进；生产使用请用官方仓库与官方发布包。
