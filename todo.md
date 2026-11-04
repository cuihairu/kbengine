# todo — kbengine

## 待办

- [x] 代码深审 P0：客户端 `PY_*` 方法参数经非受限 `pickle.loads` 反序列化可远程代码执行（`kbe/src/lib/pyscript/pickler.cpp:26,152`、`kbe/src/lib/entitydef/datatype.cpp:1240,1296,1352,1408`；需引擎构建环境实现受限反序列化）（2026-11-04 已修：受限 Unpickler 白名单 `_upf` 回调+安全内建类型；`kbe_smoke_tests` 177/177 全绿；commit `167d085`、`kbe/src/lib/pyscript/pickler.cpp:26`）
- [x] 代码深审 P1：传送回滚解引用 NULL 控制器列表（`kbe/src/server/cellapp/entity.cpp:3989-3993,4112-4113,4147-4148`、`cellapp.cpp:2087`）（2026-11-04 复核属实；已修：`createMovementHandlerFromStream` 顶部惰性重建 `pControllers_`，与 `createControllersFromStream` 既有守卫一致，保证传送回滚序列先移动处理器后控制器列表的解流顺序安全；`kbe_cellapp` 编译链接过；`kbe_smoke_tests` 173/173、`kbe_entitydef_xml_tests` 30/30 全绿；本 commit、`kbe/src/server/cellapp/entity.cpp:4289-4293`）
- [x] 代码深审 P1：网络消息长度 `0xFFFF`+扩展字段可致 4GB 分配、`bad_alloc` 未捕获崩溃（`kbe/src/lib/network/packet_reader.cpp:141,148-151,226`、`channel.cpp:1186`）（2026-11-04 已修：扩展长度超 MAX_SIZE 拒绝+`bad_alloc` 捕获；`kbe_smoke_tests` 177/177 全绿；commit `40ab95f`、`kbe/src/lib/network/packet_reader.cpp:148`）
- [x] 代码深审 P2：安装脚本硬编码默认 MySQL 口令（`kbe/tools/server/install/installer.py:817,976` `mysql_kbe_password = "pwd123456"`）（2026-11-04 已修：空口令拒绝+重新提示；AST 扫描无残留默认口令；commit `9c7a7fd`、`kbe/tools/server/install/installer.py:817`）
- [x] 代码深审 P2：fallback 构建缺系统 `tinyxml2`，README「Fallback uses vendored deps」不准确（`cmake/KBEngineDependencies.cmake:52` 无条件 `find_package(... REQUIRED)`，无 vendored 回退）（2026-11-04 已修：vendored tinyxml2 v10 入库+fallback 强制使用；`kbe_smoke_tests` 177/177 全绿；commit `85072cc`、`cmake/KBEngineDependencies.cmake:52`）
- [x] 代码深审 P2：`__py_createEntityAnywhereFromDBID` 对未知 dbInterface 名解引用 NULL（`kbe/src/server/baseapp/baseapp.cpp:1475-1476`、`serverconfig.inl:112-124`）（2026-11-04 已修：baseapp 6 处 dbInterface NULL 守卫；`kbe_baseapp` 编译链接过；commit `9b0b6f6`、`kbe/src/server/baseapp/baseapp.cpp:1136`）
- [x] 代码深审 P2：`Entity::writeToDB` 对未知 dbInterface 名解引用 NULL（`kbe/src/server/baseapp/entity.cpp:1157-1158`）（2026-11-04 已修：writeToDB dbInterface NULL 守卫；`kbe_baseapp` 编译链接过；commit `99ba906`、`kbe/src/server/baseapp/entity.cpp:1157`）
- [x] 代码深审 P2：`KBEEntityLogTableMysql::queryEntity` SQL 失败仍返回 true、读未初始化 EntityLog（`kbe/src/lib/db_mysql/kbe_table_mysql.cpp:220-222`、`dbtasks.cpp:602-616,1658-1667`）（2026-11-04 已修：查询前初始化 entitylog+SQL 失败返回 false；`kbe_dbmgr` 编译链接过；commit `f30c19a`、`kbe/src/lib/db_mysql/kbe_table_mysql.cpp:220`）
- [x] 代码深审 P2：`Archiver::tick` 有符号 32 位乘法溢出致 OOB 索引（`kbe/src/server/baseapp/archiver.cpp:38-43`）（2026-11-04 已修：索引改为 size_t 避免有符号溢出；`kbe_baseapp` 编译链接过；commit `2e0e7ae`、`kbe/src/server/baseapp/archiver.cpp:38`）
- [x] 代码深审 P2：`findRandomPointAroundCircle` 无 ground 多边形时解引用 NULL poly/未初始化读（`kbe/src/lib/navigation/navigation_mesh_handle.cpp:197,231,246,249,267`）（2026-11-04 已修：NULL tile/poly 守卫提前返回；`kbe_navigation` 编译链接过；commit `83ec425`、`kbe/src/lib/navigation/navigation_mesh_handle.cpp:197`）
- [x] 代码深审 P2：负 navmesh layer 未拒绝、`GetLayer` `.at()` 抛异常（`kbe/src/lib/navigation/navigation_tile_handle.cpp:59,409`、`entity.cpp:2663,2690,2758`）（2026-11-04 已修：负 layer 三个入口返回 NAV_ERROR；`kbe_navigation` 编译链接过；commit `c6f8283`、`kbe/src/lib/navigation/navigation_tile_handle.cpp:59`）
- [x] 代码深审 P2：`setPosition_XYZ_float` 丢弃 Y 且用 Y 覆写 X（`kbe/src/server/cellapp/entity.cpp:1864-1866`、`cellapp_interface.h:187-191`）（2026-11-04 已修：pos.x=x/pos.y=y/pos.z=z；`kbe_cellapp` 编译链接过；commit `c2a0c8c`、`kbe/src/server/cellapp/entity.cpp:1864`）
- [x] 代码深审 P2：navmesh 加载失败静默报成功（`kbe/src/server/cellapp/loadnavmesh_threadtasks.cpp:22-32`、`spacememory.cpp:299-328`）（2026-11-04 已修：navmeshLayer 为空时 create 返回 NULL；`kbe_navigation` 编译过+`kbe_smoke_tests` 177/177 全绿；commit `f080123`、`kbe/src/lib/navigation/navigation_mesh_handle.cpp:464`）
- [x] 代码深审 P2：`MoveController::createFromStream` 未知 subtype 解引用 NULL（`kbe/src/lib/server/move_controller.cpp:51-58,60`）（2026-11-04 已修：未知 subtype 记错误并返回，`velocity()`/`velocity(v)` 判空；`kbe_cellapp` 编译链接过+`kbe_smoke_tests` 177/177、`kbe_entitydef_xml_tests` 30/30 全绿；commit `68f6a26`、`kbe/src/server/cellapp/move_controller.cpp:58`）
- [x] 代码深审 P2：blowfish 解密失败被忽略、信任攻击者帧（`kbe/src/lib/network/encryption_filter.cpp:225-229`、`blowfish.cpp:148-155`）（2026-11-04 已修：`decrypt` 返回 bool，失败记错并返回 `REASON_CORRUPTED_PACKET`；`kbe_loginapp`/`kbe_baseapp` 编译链接过+`kbe_smoke_tests` 177/177、`kbe_entitydef_xml_tests` 30/30 全绿；commit `2a4f321`、`kbe/src/lib/network/encryption_filter.cpp:225`）
- [x] 代码深审 P3：冒烟测试硬编码口令（`tests/smoke/test_server_sendmail.cpp:58` `info.password = "secret"`）（2026-11-04 已修：口令改从 `KBE_TEST_SMTP_PASSWORD` 环境变量读取，缺省为测试专用非敏感值；`kbe_smoke_tests` 177/177 全绿；commit `5db37ab`、`tests/smoke/test_server_sendmail.cpp:61`）
- [x] 代码深审 P3：`rebuild_docs.py` CHM 解包超时异常路径泄漏临时目录（`tools/rebuild_docs.py:490-505`）（2026-11-04 已修：`subprocess.TimeoutExpired` 捕获后清理临时目录并抛 `RuntimeError`；`py_compile`/AST 校验通过+`kbe_smoke_tests` 177/177 全绿；commit `1e6196e`、`tools/rebuild_docs.py:497`）
- [x] 代码深审 P3：`verify_authoritative_docs.py:78` CHM 页面数据未判 None（2026-11-04 已修：`RetrieveObject` 返回 `None` 时跳过；`py_compile`/AST 校验通过+`kbe_smoke_tests` 177/177 全绿；commit `b95dfbd`、`tools/verify_authoritative_docs.py:78`）
- [x] 代码深审 P3：`verify_authoritative_docs.py:153` 生成页缺对应文件未捕获 `FileNotFoundError`（2026-11-04 已修：当前/生成页缺失均记 mismatch 不中断；commit `b95dfbd`、`tools/verify_authoritative_docs.py:155`）
- [x] 代码深审 P3：`verify_authoritative_docs.py:188` 有失配仍 `return 0`，CI 靠 stdout 判定（2026-11-04 已修：失配或 WebConsole 不匹配时 `return 1`；commit `b95dfbd`、`tools/verify_authoritative_docs.py:190`）
- [x] 代码深审 P3：`queryAccount` 失败路径 `ACCOUNT_INFOS.flags` 未初始化即被读（`kbe/src/lib/db_mysql/kbe_table_mysql.cpp:645-646,690-691`、`dbtasks.cpp:687,708`）（2026-11-04 已修：`queryAccount`/`queryAccountAllInfos` 查询前置零 `dbid/flags/deadline`；`kbe_dbmgr` 编译链接过+`kbe_smoke_tests` 177/177、`kbe_entitydef_xml_tests` 30/30 全绿；commit `9c6af68`、`kbe/src/lib/db_mysql/kbe_table_mysql.cpp:645`）
- [x] 代码深审 P3：视角 aliasID 超 255 实体过期/截断（`kbe/src/server/cellapp/witness.cpp:721-733`）（2026-11-04 已修：`updateEntitiesAliasID` 只为 NORMAL 实体分配紧凑 aliasID，移除 255 上限 break；`kbe_cellapp` 编译链接过+`kbe_smoke_tests` 177/177、`kbe_entitydef_xml_tests` 30/30 全绿；commit `25c9040`、`kbe/src/server/cellapp/witness.cpp:725`）
- [x] 代码深审 P3：`SpaceMemory::removeEntity` 仅 KBE_ASSERT 越界（`kbe/src/server/cellapp/spacememory.cpp:394-400`）（2026-11-04 已修：越界/索引不匹配改为记错并返回，避免生产 abort；`kbe_cellapp` 编译链接过+`kbe_smoke_tests` 177/177、`kbe_entitydef_xml_tests` 30/30 全绿；commit `7be0f4f`、`kbe/src/server/cellapp/spacememory.cpp:394`）
- [x] 代码深审 P3：`createFromStream` 未知 scriptUType 解引用 NULL（`kbe/src/server/cellapp/entity.cpp:4082-4084`）（2026-11-04 已修：未知 scriptUType 记错并 `onDestroy(true)` 返回；`kbe_cellapp` 编译链接过+`kbe_smoke_tests` 177/177、`kbe_entitydef_xml_tests` 30/30 全绿；commit `8ef3a47`、`kbe/src/server/cellapp/entity.cpp:4083`）
- [x] 代码深审 P3：传送到达消息未知 entityType 解引用 NULL（`kbe/src/server/cellapp/cellapp.cpp:1939`）（2026-11-04 已修：未知 entityType 记错并发送失败回执；附带修正 Pickler 测试回调引用计数修复间歇段错误；`kbe_cellapp` 编译链接过+`kbe_smoke_tests` 177/177 重复运行全绿、`kbe_entitydef_xml_tests` 30/30 全绿；commit `4d78700`、`kbe/src/server/cellapp/cellapp.cpp:1939`）
- [x] 代码深审 P3：`ProximityController` 析构解引用 NULL trap（`kbe/src/lib/server/proximity_controller.cpp:34-38`）（2026-11-04 已修：析构前判 `pTrapTrigger_` 非空并置 NULL；`kbe_cellapp` 编译链接过+`kbe_smoke_tests` 177/177、`kbe_entitydef_xml_tests` 30/30 全绿；commit `f5652ea`、`kbe/src/server/cellapp/proximity_controller.cpp:34`）
- [x] 代码深审 P3：ID 池耗尽返回复用 ID（`kbe/src/lib/server/idallocate.h:257-279`）（2026-11-04 已修：耗尽时记错并返回 0，不推进区间/复用旧 ID；`kbe_baseapp`/`kbe_cellapp` 编译链接过+`kbe_smoke_tests` 177/177、`kbe_entitydef_xml_tests` 30/30 全绿；commit `05f045f`、`kbe/src/lib/server/idallocate.h:257`）
- [ ] 代码深审 P3：`RangeTrigger::install` 部分失败结果被忽略（`kbe/src/lib/server/range_trigger.cpp:73-93`）
- [ ] 代码深审 P3：`Channel::destroy` 未回池 EndPoint、每信道周期泄漏（`kbe/src/lib/network/channel.cpp:533-543,293-305`）
- [ ] 代码深审 P3：`mergeFragmentMessage` 拷贝缺 `+ rpos()` 偏移（`kbe/src/lib/network/packet_reader.cpp:291`）
- [ ] 代码深审 P3：dispatch 仅捕 `MemoryStreamException`、异常泄漏接收报文（`kbe/src/lib/network/channel.cpp:1182-1199,1201`）

## 已完成

- [x] 文档双语收口 README 双语 + 中文版独立页（2026-10-16；README.zh.md 入库）
- [x] 文档双语收口 docs/ 根级 7 篇配对（2026-10-16；7 篇互链）
- [x] 中英双版媒体文件互索引 + 4 个原始资料登记（2026-10-16 R1016F；仓库路径内联免死链）
- [x] 文档双语收口 docs/ 子目录 api（2026-10-22；commit `3b08b08`、`daaf492`；docs:build 过、test:docs 4/4、权威核对 24/24）
- [x] 文档双语收口 docs/ 子目录 architecture/study/guide/resources/book-legacy/tutorial（b9e91b4 布局翻转；131 对 131、0 缺失、2026-11-01 复核）
- [x] api/kbengine 双语化核对口径：24 页 = 13 直通 + 11 增强（2026-10-16 R1016PATROL；新增 `test_verify_authoritative_docs.py` 三类回归）

## 区间审核：a3c481d731d1..HEAD（2026-11-02，只审不改）

覆盖：5 提交（`20b9794` dbmgr NULL 模块守卫、`4f09902` navmesh 边界、`0700925` method Py_None、`db77814`/`3f6d733` 文档）× 7 文件，全部代码文件已逐行核对 + 框架语义回溯（TPTask 线程循环/`presentMainThread` 状态机、`CallbackMgr` 超时回收、baseapp `isArchiveing_` 复位点、`EntityAutoLoader::querying_` 状态机）。

- [x] 审核发现：`presentMainThread` 的 NULL 模块守卫提前 `return COMPLETED` 而未向请求方发送失败回调，依赖侧永久卡死——`DBTaskEntityAutoLoad::presentMainThread`（`dbtasks.cpp:582-586`）丢 `onEntityAutoLoadCBFromDBMgr` → baseapp `EntityAutoLoader::querying_` 永真（`entity_autoloader.cpp:73,122`）→ 自动加载停摆、baseapp 启动挂起（`setAutoLoadState(1)` 永不执行）；`DBTaskWriteEntity::presentMainThread`（`dbtasks.cpp:387-391`）丢 `onWriteToDBCallback` → baseapp `isArchiveing_` 永不复位（`entity.cpp:1194,1240`），首写实体（dbid<=0、cell 存活）永久无法再写库；`DBTaskDeleteEntityByDBID`/`DBTaskLookUpEntityByDBID::presentMainThread`（`dbtasks.cpp:523-527,680-684`）丢回调 → baseapp 脚本回调永不触发（`CallbackMgr::tick` 仅超时回收 PyObject，不调用脚本）。建议：守卫应构造 `success_=false` 的失败回调而非静默 COMPLETED（`dbtasks.cpp` 4 处）（2026-11-03 复核属实；已修：4 处 NULL 守卫改为发送 `success_=false` 失败回调，`kbe/src/server/dbmgr/dbtasks.cpp:387-401`（WriteEntity）、`:534-548`（DeleteEntityByDBID）、`:604-618`（EntityAutoLoad）、`:713-727`（LookUpEntityByDBID）；`kbe_dbmgr` 编译过；`kbe_smoke_tests` 173/173、`kbe_entitydef_xml_tests` 30/30 全绿；commit `e9d548d`、`kbe/src/server/dbmgr/dbtasks.cpp:390-401,537-548,607-618,716-727`）
- [x] 审核发现：`DBTaskRemoveEntity::db_thread_process` 的 NULL 守卫位于 `eraseEntityLog` 之后（`dbtasks.cpp:437-447`），未知模块路径先擦除 entitylog 再失败返回，留下「日志已删、实体行未删」的不一致——守卫应前置到 `eraseEntityLog` 之前（2026-11-03 复核属实；已修：NULL 守卫前置到 `eraseEntityLog` 之前，`kbe/src/server/dbmgr/dbtasks.cpp:445-451`；`kbe_dbmgr` 编译过；`kbe_smoke_tests` 173/173、`kbe_entitydef_xml_tests` 30/30 全绿；commit `e9d548d`、`kbe/src/server/dbmgr/dbtasks.cpp:447-451`）
- [x] 审核发现：测试缺口——本区间 4 个代码修复均无回归测试：navmesh 恶意瓦片解码（size=0/负/越界）无用例（`tests/smoke/` 无 navigation 测试）；`MethodDescription::createFromStream` NULL 参数→Py_None 无用例；dbmgr/baseapp `findScriptModule` NULL 守卫路径无用例（仅 `test_entitydef_xml_loading.cpp` 存在）（2026-11-03 复核属实；已补回归：navmesh 恶意瓦片 3 用例 `tests/smoke/test_navigation_navmesh_decode.cpp`（RejectsZero/Negative/OverrunSizeTile）、MethodDescription NULL→Py_None 1 用例 `tests/smoke/test_entitydef_xml_loading.cpp`（MethodDescriptionCreateFromStreamTurnsNullArgIntoNone）；dbmgr `findScriptModule` NULL 守卫为集成级（需 `Dbmgr` 完整运行时，`dbtasks.cpp` 依赖 `Dbmgr::getSingleton().networkInterface()`/`DBInterface`），无法单测，以 `kbe_dbmgr` 编译+修复代码为证；`kbe_smoke_tests` 173/173、`kbe_entitydef_xml_tests` 30/30 全绿；commit `f2d62bc`、`tests/smoke/test_navigation_navmesh_decode.cpp:83-103`、`tests/smoke/test_entitydef_xml_loading.cpp:1749-1774`）

## 区间审核：3f6d7336..HEAD（2026-11-03，只审不改）

覆盖：1 提交（`c2a3b8f` Update README.md）× 1 文件（`README.md`，+3/-1）。改动为纯文档排版：将徽章 `<p align="center">` 行（内容逐字节一致）从 Development Fork Notice 下方移至语言链接下方，并补一行空行；无链接新增/失效、无 Markdown 结构破坏、无代码文件涉及。

- [x] 审核发现：本轮 diff 未见问题（3f6d7336..HEAD）（2026-11-03 复核属实：`git diff 3f6d7336 HEAD` 仅 `README.md` +3/-1 纯文档排版，徽章 `<p align="center">` 行内容逐字节一致地从 Development Fork Notice 下方移至语言链接下方并补一空行，无链接新增/失效、无 Markdown 结构破坏、无代码文件涉及）
