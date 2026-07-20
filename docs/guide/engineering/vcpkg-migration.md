# vcpkg 迁移状态

> 这页记录第三方依赖从仓库内 vendored 形态迁到包管理形态的现状，不讨论业务功能。

## 已切到包管理的依赖

当前 vcpkg 模式下，以下依赖已经通过 `vcpkg.json` 与 `cmake/KBEngineDependencies.cmake` 管理：

- `apr`
- `apr-util`
- `curl`
- `fmt`
- `hiredis`
- `jemalloc`
- `libmariadb`
- `log4cxx`
- `openssl`
- `python3`
- `tinyxml2`
- `utf8cpp`（通过 `utfcpp` 包）
- `zlib`

补充说明：

- `jwsmtp` 已经在应用层被 libcurl 的 SMTP 能力替代，不再属于活跃依赖图。
- `tinyxml` 不再是当前构建使用的 XML 依赖，活跃依赖已经转到 `tinyxml2`。

## 仍然保留 vendored 的活跃依赖

### `g3dlite`

- 当前由 `kbe/src/lib/CMakeLists.txt` 继续构建。
- 之所以仍然保留，是因为当前分支下没有确认可替代的稳定包管理来源。

### `tmxparser`

- 当前由 `cmake/KBEngineDependencies.cmake` 继续构建。
- `navigation_tile_handle.h` 仍直接依赖其头文件。
- 目前没有明确替代路径，因此暂时保留 vendored。

## 仍存在但不驱动当前 CMake 构建的目录

这些目录还在仓库里，但不是当前迁移的第一优先级：

- `apr`
- `apr-util`
- `curl`
- `expat`
- `jemalloc`
- `jwsmtp`
- `log4cxx`
- `tinyxml`
- `utf8cpp`
- `vsopenssl`
- `zlib`

## 需要单独评估的遗留目录

- `sigar`
- `vld`
- `openssl` 的额外 vendored 副本

这些目录是否可以删除，不应只看当前 CMake 路径，还要确认是否仍被其他平台或历史构建链依赖。

## 推荐后续动作

1. 评估 `tmxparser` 是否继续 vendored，还是替换成更稳定的外部集成方案。
2. 明确 `g3dlite` 的长期策略，是继续随仓库维护，还是寻找替代来源。
3. 在分支稳定后，再清理当前未参与活跃构建的历史依赖目录。

## 结论

- 当前迁移的核心已经不是“有没有接入 vcpkg”，而是“哪些 vendored 目录仍然实际影响构建图”。
- 先把活跃依赖和历史残留分开，后续清理才不会误删潜在平台路径。

## 兼容页面

- [/vcpkg-migration-status.md](/vcpkg-migration-status.md)
