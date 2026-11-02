# KBEngine 代码深审报告（2026-11-01 起，2026-11-02 补完）

- 日期：2026-11-01 首轮（fork 自身代码）／2026-11-02 补完（引擎 C++ 三路深审回填）
- 范围：全仓 —— `kbe/src` 引擎（约 241k 行 C++）重点覆盖网络反序列化层、dbmgr/baseapp、cellapp/共享服务库三路；`tools/`（13 文件 4619 行）、`typings/`（11 文件 899 行）、`cmake/`、`.github/workflows/`、`tests/`
- 方法：① 安全静态扫描（确定性 grep 全仓 + 逐类结论）② 隐藏 bug 猎捕（fork 代码逐读 + 引擎三路子代理深审）③ 质量门（lint/类型/测试基线）④ 报告 + P0/P1 当场修
- 基线事实：本 fork 对 `kbe/src` 的改动仅根导入 commit `0bc93d5`；本轮修复为**首次引擎源码改动**（3 个 P1），一行一守卫、不改语义，静态核验（括号平衡/守卫配对）通过；**本机无引擎编译环境（见 G1），C++ 编译与 ctest 由 push 后 CI 三平台验证**

## ① 安全静态扫描（全仓）

| # | 类别 | 发现 | 证据 | 级别 | 处置 |
|---|---|---|---|---|---|
| S3 | 路径穿越 | CHM 归档成员名（archive-controlled）直接拼 `output_root/path`，`lstrip("/")` 不净化 `..`/绝对路径/反斜杠分隔符，可写出临时目录之外 | `tools/verify_authoritative_docs.py:78-86`（修复前） | **P1** | **已修复**（`a3c481d`：`PurePosixPath` 归一 `\`→`/`、拒绝绝对路径与 `..` 片段；5 用例全过） |
| S1 | 硬编码密钥 | 安装脚本内置默认 MySQL 口令 | `kbe/tools/server/install/installer.py:817,976` `mysql_kbe_password = "pwd123456"` | P2 | todo（上游代码，装库默认凭据；改动改安装行为） |
| S2 | 硬编码密钥 | 冒烟测试内置口令 | `tests/smoke/test_server_sendmail.cpp:58` `info.password = "secret"` | P3 | todo（仅测试） |

**无可利用注入（逐类结论，均已逐条复核签核）**：
- **Shell 注入**：`machine.cpp:1184` `system("kill -s 9 {}")` 中 pid 为整型经 `fmt::format`（不可注入）；`clientsdk_downloader.cpp:127` 用 `execv`（argv 数组无 shell 展开）；`installer.py:753` `Popen(shell=True)` 为上游开发工具静态命令。
- **SQL 注入**：全部跨信任边界字符串经 `mysql_real_escape_string`（`entity_table_mysql.cpp:1866,1934,1986,2038`、`kbe_table_mysql.cpp:163` 等 25 处）；`%s` 仅用于受信列名/配置/读回；`DBTaskExecuteRawDatabaseCommand` 为设计内管理端 raw-SQL 通道，非注入缺陷。
- **eval/exec**：无（`webconsole/six.py:699` 为 six shim；`verify_authoritative_docs.py:39` 为 `re.compile` 误报）。
- **反序列化**：无 `pickle` 于 fork 自身工具；**引擎侧见 N0（P0）**。
- **路径穿越**：S3 之外无（`clientsdk_downloader.cpp:200` 源自服务端配置目录枚举；`rsa.cpp` 密钥路径来自配置）。

## ② 隐藏 bug 猎捕

### fork 自身代码（逐读）

| # | 发现 | 证据 | 级别 | 处置 |
|---|---|---|---|---|
| B1 | `extract_chm_to_temp` 超时路径泄漏临时目录（仅 hh.exe 挂起时可达） | `tools/rebuild_docs.py:490-505` | P3 | todo |
| B2 | CHM 页面 `RetrieveObject` 数据未判 None 即 `write_bytes` | `tools/verify_authoritative_docs.py:78` | P3 | todo |
| B3 | 生成页缺对应文件未捕获 `FileNotFoundError` | `tools/verify_authoritative_docs.py:153` | P3 | todo |
| B4 | 有失配仍恒 `return 0`，CI 靠 stdout 判定 | `tools/verify_authoritative_docs.py:188` | P3 | todo |

### 引擎 C++ 三路深审（子代理回填，2026-11-02）

**网络反序列化层**（`kbe/src/lib/network/`、`lib/pyscript/`、`lib/entitydef/`）：

| # | 发现 | 证据 | 级别 | 处置 |
|---|---|---|---|---|
| N0 | **非受限反序列化 → RCE**：客户端可发含 `PY_DICT/PY_LIST/PY_TUPLE/PY_OBJECT` 参数的暴露方法调用，blob 经标准 `pickle.loads` 解包，`__reduce__` gadget 可在 cellapp/baseapp 进程执行任意代码 | `kbe/src/lib/pyscript/pickler.cpp:26`（`unPicklerMethod_ = pickle.loads`）、`:152`；`kbe/src/lib/entitydef/datatype.cpp:1240,1296,1352,1408`；`method.cpp:199`；`entity.cpp:1035` | **P0** | **登记待修**（修复需将 unpickler 改为受限 `find_class` 白名单实现——属安全语义/协议变更，**必须**引擎构建+行为验证后落地；本机无引擎编译环境，盲改风险高于价值。修复方向：`Pickler::initialize` 不再直取 `pickle.loads`，改注册只放行 `Vector2/3/4、FixedDict、EntityComponent、MemoryStream` 等已注册类 + 内建安全类型的受限 Unpickler） |
| N1 | 变长消息长度 `0xFFFF` 触发 4 字节扩展长度（文件上限 4GB），`new uint8[...]` 无上限；外部信道有 `>65535 → condemn` 守卫但 CLIENT/BOTS/内部信道跳过；`bad_alloc` 仅 `catch(MemoryStreamException&)` 不可捕 → 进程崩溃 | `kbe/src/lib/network/packet_reader.cpp:141,148-151,179-181,226`；`channel.cpp:1186` | P1 | 登记待修（分配上限与异常面扩捕属协议行为变更，需引擎构建环境验证） |
| N2 | 参数反序列化失败时 `PyTuple_SET_ITEM` 把 NULL 存入 args tuple，`PyObject_CallObject/checkArgs` 解引用 NULL → 崩溃 | `kbe/src/lib/entitydef/method.cpp:199-207,236`；`entity.cpp:1038` | P1 | **已修复**（`0700925`：失败参数置 `Py_None` 并 INCREF，tuple 永不持 NULL） |
| N3 | blowfish `decrypt()` 返回值被丢弃；长度非 8 字节倍数时密文原样进入消息解析器，`padSize_` 攻击者可控 | `kbe/src/lib/network/encryption_filter.cpp:141-144,225-229`；`blowfish.cpp:148-155` | P2 | todo |
| N4 | 信道销毁不回池 EndPoint（仅 `~Channel` 走 `finalise`），连接高频换手缓慢泄漏 | `channel.cpp:533-543,85-88,293-305` | P3 | todo |
| N5 | `mergeFragmentMessage` `memcpy` 缺 `+ rpos()`（现路径 rpos==0 为潜伏） | `packet_reader.cpp:291`（对照 `:233`） | P3 | todo |
| N6 | dispatch 仅捕 `MemoryStreamException`；`MemoryStreamWriteOverflow`/`bad_alloc` 未捕致 terminate，`RECLAIM_PACKET` 在 try 外致报文泄漏 | `channel.cpp:1182-1199,1201`；`memorystream.h:589-594` | P3 | todo |

**dbmgr / baseapp / db_mysql**：

| # | 发现 | 证据 | 级别 | 处置 |
|---|---|---|---|---|
| D1 | **系统性 NULL 脚本模块解引用**：`findScriptModule(sid_/entityType_)` 结果（sid 来自远程组件消息）在写/查/删/自动加载全路径无 NULL 检查即 `->getName()/->getUType()`；DBThread worker 段错误可致整个 dbmgr 停摆（所有持久化停止） | `dbtasks.cpp` 12 处任务方法 + `entity_table.cpp:296-320` + `entity_autoloader.cpp:88,105`（修复前） | **P1** | **已修复**（`20b9794`：`EntityTables::{write,remove,query}Entity` 3 个 choke 点 + dbtasks 12 处任务方法 + entity_autoloader 提升守卫，全部「记错并安全失败」——db_thread 返回 false、presentMainThread 返回 COMPLETED；静态核验括号平衡、14 守卫与 14 处 `findScriptModule` 一一配对） |
| D2 | `__py_createEntityAnywhereFromDBID` 未知 dbInterface 名解引用 NULL（脚本可达） | `baseapp.cpp:1475-1476`；根因 `serverconfig.inl:112-124` | P2 | todo |
| D3 | `Entity::writeToDB` 未知 dbInterface 名解引用 NULL（脚本可达） | `entity.cpp:1157-1158` | P2 | todo |
| D4 | `KBEEntityLogTableMysql::queryEntity` SQL 失败仍返回 true，调用方读未初始化 `EntityLog`（登录误路由到垃圾 componentID/ip） | `kbe_table_mysql.cpp:220-222`；`dbtasks.cpp:602-616,1658-1667` | P2 | todo |
| D5 | `queryAccount` 失败路径 `ACCOUNT_INFOS.flags` 未初始化即被读 | `kbe_table_mysql.cpp:645-646,690-691`；`dbtasks.cpp:687,708` | P3 | todo |
| D6 | `Archiver::tick` `int size * int archiveIndex_` 有符号 32 位溢出 → `arEntityIDs_[i]` OOB（规模/配置相关 UB） | `archiver.cpp:38-43` | P2 | todo |

**cellapp / 共享服务库 / 导航**：

| # | 发现 | 证据 | 级别 | 处置 |
|---|---|---|---|---|
| C1 | 传送回滚解引用 NULL 控制器列表：`changeToGhost` `SAFE_RELEASE` 置空 `pControllers_`，失败回滚路径 `createMovementHandlerFromStream` 先于 `createControllersFromStream` 使用 | `cellapp/entity.cpp:3989-3993,4112-4113,4147-4148,4300`；`cellapp.cpp:2087` | P1 | 登记待修（控制器生命周期顺序调整属逻辑变更，需引擎构建+回归验证） |
| C2 | **navmesh 瓦片解码无边界检查**：`memcpy(&tileHeader,&data[pos],size)` 与 `memcpy(tileData,&data[pos],size)`，`size=tileHeader.dataSize` 文件控制、`pos` 无检查越过 `readsize` → 堆 OOB 读 | `lib/navigation/navigation_mesh_handle.cpp:527,547`（修复前） | P1 | **已修复**（`4f09902`：两处 memcpy 前置 `pos+size > (int)readsize` 与 `size<=0 || size > (int)readsize-pos` 守卫，溢出安全写法，沿用既有 `DT_FAILURE+DT_INVALID_PARAM` 失败路径） |
| C3 | `findRandomPointAroundCircle` 无 ground 多边形时 NULL poly 解引用 + `allPolyAreas` 未初始化读 | `navigation_mesh_handle.cpp:197,231,246,249,267` | P2 | todo |
| C4 | 负 navmesh layer 未拒绝，`GetLayer` `.at()` 抛 `std::out_of_range` → terminate | `navigation_tile_handle.cpp:59,409`；`entity.cpp:2663,2690,2758` | P2 | todo |
| C5 | `setPosition_XYZ_float` 丢 Y 且用 Y 覆写 X（`pos.x=x; pos.x=y; pos.z=z;`），客户端消息驱动 | `cellapp/entity.cpp:1864-1866`；`cellapp_interface.h:187-191` | P2 | todo |
| C6 | navmesh 加载失败静默报成功，脚本误信几何已载入 | `loadnavmesh_threadtasks.cpp:22-32`；`spacememory.cpp:299-328` | P2 | todo |
| C7 | `MoveController::createFromStream` 未知 subtype 解引用 NULL（`KBE_ASSERT` 在 NDEBUG 为空操作） | `lib/server/move_controller.cpp:51-58,60` | P2 | todo |
| C8 | 视角 aliasID 超 255 实体过期/截断，线上重复 aliasID | `cellapp/witness.cpp:721-733,646-649` | P3 | todo |
| C9 | `SpaceMemory::removeEntity` 仅 KBE_ASSERT 越界（NDEBUG 空操作） | `spacememory.cpp:394-400` | P3 | todo |
| C10 | `createFromStream` 未知 scriptUType 解引用 NULL | `cellapp/entity.cpp:4082-4084` | P3 | todo |
| C11 | 传送到达消息未知 entityType 解引用 NULL（同型根因） | `cellapp.cpp:1939` | P3 | todo |
| C12 | `ProximityController` 析构解引用 NULL trap（默认构造路径未重建） | `lib/server/proximity_controller.cpp:34-38,25-31,48-52` | P3 | todo |
| C13 | ID 池耗尽返回复用 ID（双实体同 ID） | `lib/server/idallocate.h:257-279` | P3 | todo |
| C14 | `RangeTrigger::install` 部分失败结果被构造器忽略 | `proximity_controller.cpp:21`；`range_trigger.cpp:73-93` | P3 | todo |

### 深审否定性结论（防误报留档）

- **网络库无跨线程共享状态**（仅主线程 idle 钩子）；KCP 解析（`ikcp.c` 输入/接收/窗口）、MemoryStream 读越界、WebSocket 帧解析（64 位长度封顶 65535）、`Bundle::operator>>`、SSL 探测均无 OOB。
- **db_mysql 转义完备**（见 ①）；子表解析（`write/read_entity_helper.h`）边界成立；DB 句柄每线程独占无共享；缓冲任务序列无丢失/双跑。
- **AOI 坐标链表**移动/移除/轴扫有界；ghost 生命周期路由健全；timer 系统无泄漏/双释放；`appendPackXZ` 越界钳制。
- 搜索路径均确认存在后下否定结论（规范要求）。

## ③ 质量门

| 门 | 结果 |
|---|---|
| Lint | 未配置（无 flake8/ruff/eslint/clang-tidy 配置）→ N/A |
| 类型检查 | fork Python `py_compile` 通过（tools/ 13 文件）；`typings/*.pyi` 899 行存根完好 |
| 测试基线 | `test:docs` 4/4 通过；`docs:build` 通过（死链零容忍，EXIT=0）；CHM 权威核对本机不可跑（chmlib C 库未建，`.tmp/build/chmlib-0.40` 源缺失——环境限制非代码回归） |
| G1（P2） | 引擎 C++ 本机不可编译：fallback 模式 `find_package(tinyxml2 CONFIG REQUIRED)`（`cmake/KBEngineDependencies.cmake:52`）无条件且无 vendored 回退，本机缺 tinyxml2 头文件（hiredis 有 vendored 回退、libmariadb 头文件已装）；vcpkg 预设本机无 `VCPKG_ROOT`/二进制。README「Fallback mode: Uses vendored dependencies」不准确 → todo。**因此本轮 3 个引擎 P1 修复以 push 后 CI（Linux/Windows/macOS 三平台 CMake+ctest）为编译门** |

## 四、汇总与修复总账

| 级别 | 计数 | 当场修复 | 登记待修 |
|---|---|---|---|
| P0 | 1（N0 pickle RCE） | 0 | 1（需引擎构建环境实现受限反序列化；修复方向已给出） |
| P1 | 6（S3、D1、C1、C2、N1、N2） | **4**（S3 `a3c481d`、D1 `20b9794`、C2 `4f09902`、N2 `0700925`） | 2（C1 传送回滚——逻辑变更需回归；N1 分配上限——协议行为变更） |
| P2 | 12（S1、G1、D2-D4、D6、C3-C7、N3） | 0 | 12 |
| P3 | 16（S2、B1-B4、D5、C8-C14、N4-N6） | 0 | 16 |

- **发现总数 35**：已修 4（S3 + D1 + C2 + N2，commit `a3c481d`、`20b9794`、`4f09902`、`0700925`）；待修 31（P0×1、P1×2、P2×12、P3×16，全部登记 `todo.md` 一行一条带 file:line）。
- 修复原则：只做**加法守卫**（NULL 检查、边界检查、置 None、路径净化），不改协议/生命周期语义；改后静态核验（五文件括号/圆括号/方括号平衡全过、守卫与解引用一一配对）；引擎编译由 CI 兜底。
- N0（P0 RCE）与 C1/N1（P1）**未当场修**的统一理由：修复本身是安全语义/协议/生命周期变更，必须以引擎构建 + 行为回归验证后落地；本机无引擎编译环境（G1），盲改若错将破坏核心路径（如全部 `PY_*` 方法调用），风险大于收益。证据与修复方向已完整登记，供具备构建环境的会话直接实施。
- fork 首次改动 `kbe/src`：3 个 P1 修复共 5 文件（`db_interface/entity_table.cpp`、`dbmgr/dbtasks.cpp`、`baseapp/entity_autoloader.cpp`、`navigation/navigation_mesh_handle.cpp`、`entitydef/method.cpp`），全部为守卫式加固。
