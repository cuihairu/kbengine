# 网络与消息系统

> 本页分析 KBEngine 源码里的消息传递、序列化与连接管理机制。

## 建议覆盖的主题

- EventDispatcher / NetworkInterface / Channel / Bundle 的职责
- Message handler 注册与分发
- 内部组件通信与客户端通信的共同点和差异
- Bundle 序列化与 MemoryStream 使用方式
- 网络线程 / 主线程模型与事件循环

## 建议重点跟踪

- 网络层公共基类
- 典型消息从发送到处理的完整路径
- EntityCall 与暴露方法在网络层的落点
