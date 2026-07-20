# Python 3.12 升级

> 这页汇总嵌入式 Python 运行时升级的已知阻塞、改造范围和推进顺序。

## 当前基线

- KBEngine 直接内置 CPython 源码，路径在 `kbe/src/lib/python`。
- 当前 vendored 版本是 `3.7.3`。
- Unix 构建依赖静态归档产物，Windows 工程文件直接链接 `python37` 系列库名。

## 为什么选择 3.12

- 相比 3.13/3.14，3.12 更适合作为大版本嵌入式运行时升级的第一落点。
- 能获得更现代的 Python 运行时，同时避免追逐仍在快速演进的新分支。

## 已确认阻塞

- `kbe/src/lib/Makefile` 依赖旧版 Python 静态库命名。
- `kbe/src/lib/pyscript/script.cpp` 仍调用已在现代 CPython 中移除的 `PyEval_InitThreads()`。
- 多个 Windows `vcxproj` 仍硬编码 `python37.lib` / `python37_d.lib`。
- `kbe/src/lib/python/PC/` 下部分 Windows 转发文件仍绑定旧 DLL 名称。

## 推荐迁移顺序

1. 替换 vendored `kbe/src/lib/python` 树到 `3.12.x`。
2. 重新落回 KBEngine 对 CPython 的定制修改。
3. 调整 Unix 构建胶水，适配新的静态库和扩展布局。
4. 批量更新 Windows 工程文件中的 `python37` 依赖名。
5. 审计所有嵌入层 C/C++ 调用，处理已移除或语义变更的 Python C API。
6. 先打通 Linux 服务端，再修 Windows 集成。

## 当前关注文件

- `kbe/src/lib/pyscript/script.cpp`
- `kbe/src/lib/python/PC/pyconfig.h`
- `kbe/src/lib/python/PC/python3.def`
- `kbe/src/server/baseapp/baseapp.vcxproj`
- `kbe/src/server/cellapp/cellapp.vcxproj`
- `kbe/src/server/dbmgr/dbmgr.vcxproj`
- `kbe/src/server/loginapp/loginapp.vcxproj`
- `kbe/src/server/tools/bots/bots.vcxproj`
- `kbe/src/server/tools/guiconsole/guiconsole.vcxproj`
- `kbe/src/server/tools/interfaces/interfaces.vcxproj`
- `kbe/src/server/tools/kbcmd/kbcmd.vcxproj`
- `kbe/src/server/tools/logger/logger.vcxproj`

## 结论

- 这不是“升级一个依赖版本”，而是一次嵌入式运行时升级。
- 风险主要集中在构建胶水、Windows 工程集成和已移除 C API，而不是纯脚本层兼容。
- 先把升级面和阻塞点集中在一页，后续才能按批次推进，而不是散落在提交说明里。

## 兼容页面

- [/python-3.12-upgrade-notes.md](/python-3.12-upgrade-notes.md)
