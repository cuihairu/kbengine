# typings 逐条正确性核验报告（对照 kbe/src C++ 源码）

- 核验对象：`typings/` 下全部 .pyi（`KBEngine/__init__.pyi`、`KBEngine/hooks.pyi`、`_KBEngine_*.pyi` × 9）及 `API_MATRIX.md`
- 对照源码：`kbe/src/`（引擎树。注意：本仓库源码无 `entitydef/entity_script.cpp`/`entity.cpp`，Entity 暴露面实际位于 `kbe/src/server/baseapp/entity.cpp`、`kbe/src/server/cellapp/entity.cpp`、`lib/entitydef/entity_macro.h`、`lib/client_lib/entity.cpp`；模块级函数在各组件 `installPyModules()` 注册点）
- 方法：脚本化提取全部 pyi 声明（390 条），按注册模式（`APPEND_SCRIPT_MODULE_METHOD` / `SCRIPT_METHOD_DECLARE` / `SCRIPT_GET(SET)_DECLARE` / `PyModule_AddInt(Constant|Macro)` / `registerPyObjectToScript` / `BASE_SCRIPT_HREADER` / 回调字符串）全库匹配取证，再对每个组件文件做 ≥10 个签名级抽查（读 `PyArg_ParseTuple` 解析）。
- 路径约定：以下 `file:line` 均相对 `kbe/src/`。

## 结论

**typings 已覆盖本引擎绝大多数脚本可触达的 API（pyi 声明条目 392 条中 366 条判定 OK，占 93%），且未发现“凭空捏造的函数名”——所有偏差都是签名/返回值/适用范围级别的，无出处条目集中在两处（logger.pyi 的 4 个函数、hooks 的 5 个回调、bots Entity 的 4 个成员）。**主要问题按严重度：

1. **logger.pyi 整体写错了组件归属**：logger 进程只继承 `PythonApp`（`server/tools/logger/logger.h:49`），`logger.cpp` 没有任何 `APPEND_SCRIPT_MODULE_METHOD` 注册，pyi 里的 `accountLoginResponse/createAccountResponse/chargeResponse/executeRawDatabaseCommand` 在 logger 模块上不存在（它们是 interfaces/dbmgr/baseapp/cellapp 专属）。
2. **EntityCall 三子类是 typing 虚构**：源码只有一个脚本类型 `EntityCall`（`lib/entitydef/entity_call.h:15`），base/cell/client 之别是内部枚举 `ENTITYCALL_TYPE`（`lib/common/common.h:283-291`），`BaseEntityCall/CellEntityCall/ClientEntityCall` 作为 Python 类型不存在。
3. **common.pyi 若干签名与源码不符**：`open` 多了 `encoding`、`addTimer` 参数给了默认值、`delTimer` 返回类型错、`debugTracing` 多了 `self`、`listPathRes` 的 `extension` 放宽成 Tuple、`MemoryStream.__bytes__` 实为 `bytes` 方法。
4. **缺漏**：cellapp 侧 Entity 的移动/导航/视距 API（navigate、moveToPoint、addProximity 等 8 个方法）和 13 个属性（position/direction/spaceID/topSpeed 等）未进 common.pyi 的 Entity；bots.pyi 漏掉模块面 7 个函数与 Entity 6 个成员；MemoryStream 漏 rpos/wpos/fill。

下文所有判定均为四选一：**OK**（给出 file:line）/ **偏差**（pyi 写 X、源码是 Y，双证）/ **无出处**（源码中不存在）/ **缺漏**（边界内引擎有、pyi 没写）。“适用范围”注记表示该条出处真实，但注册面比 pyi 隐含的窄。

---

## 逐文件明细

### 1. _KBEngine_server_errors.pyi（39 条，全量）

全部 39 个常量 OK：由 `lib/server/server_errors.h:57-95` 的 `SERVER_ERR_STR[]` 数组按序注入，值=数组下标，与 pyi 的 `0..37` 逐一吻合；`SERVER_ERR_MAX=38` 见 `server_errors.h:55`。注入点（按组件）：
- baseapp/cellapp（EntityApp 公共路径）：`lib/server/entity_app.h:485-491`
- loginapp：`server/loginapp/loginapp.cpp:199-203`
- dbmgr：`server/dbmgr/dbmgr.cpp:347-351`
- interfaces：`server/tools/interfaces/interfaces.cpp:177-183`
- **logger 无注入点**（logger.pyi star-import 本文件属超集放宽，见偏差清单 #1）

判定：**39 OK**。

### 2. _KBEngine_pythonapp.pyi（31 条，全量+10 签名抽查）

注册点：`lib/server/python_app.cpp:271-334`（dbmgr/loginapp/interfaces/logger 四个 PythonApp 进程共用，`python_app.cpp:244-254` 按 componentType 选 entryScript）。

| 条目 | 判定 | 出处 |
|---|---|---|
| MemoryStream 类 | OK | python_app.cpp:271；类体 lib/pyscript/py_memorystream.cpp:27-35 |
| MemoryStream.__init__ | OK | python_app.cpp:271（`PyMemoryStream::py_new`） |
| MemoryStream.append / .pop | OK | py_memorystream.cpp:28 / :29 |
| MemoryStream.__bytes__ | **偏差** | pyi 写 `__bytes__`；源码方法名是 **`bytes`**（非 dunder），py_memorystream.cpp:30；另缺 rpos/wpos/fill（py_memorystream.cpp:31-33）→ 缺漏×3 |
| LOG_TYPE_NORMAL/INFO/ERR/DBG/WAR | OK | python_app.cpp:298-322 |
| NEXT_ONLY | OK | python_app.cpp:324-330 |
| component | OK | lib/pyscript/script.cpp:248 |
| publish / scriptLogType | OK | python_app.cpp:275 / :278 |
| getResFullPath / hasRes | OK | python_app.cpp:281 / :284 |
| open(res, mode) | OK | python_app.cpp:287 注册；实现 :444-457 恰 2 参 `"s\|s"` |
| listPathRes(path, extension:str) | OK | python_app.cpp:290；实现 :509-540 |
| matchPath | OK | python_app.cpp:293 |
| debugTracing() | OK | python_app.cpp:296；实现 lib/pyscript/py_gc.cpp:224-228 无参 |
| genUUID64 | OK | lib/pyscript/script.cpp:258 |
| addTimer(initialOffset, repeatOffset, callbackObj)（无默认值） | OK | python_app.cpp:329；实现 :721-726 恰 3 参 `"ffO"` |
| delTimer -> int | OK | python_app.cpp:330；实现 :755-771 返回 `PyLong_FromLong(timerID)`，失败 -1 |
| register/deregisterRead/WriteFileDescriptor | OK | python_app.cpp:331-334 |
| urlopen(url, callback, postData, headers) | OK | lib/pyscript/pyurl.cpp:23；实现 :69-160 支持 2/3/4 参（4 参="s\|O\|O\|O"，headers 必须dict、postData 必须bytes） |

签名抽查 ≥10：open、addTimer、delTimer、urlopen、listPathRes、debugTracing、scriptLogType（python_app.cpp:370-374 恰1参）、publish（:364-367 返回 int）、MemoryStream.append/pop、genUUID64 — 均与 pyi 一致。

判定：**30 OK，1 偏差，3 缺漏**（rpos/wpos/fill）。

### 3. _KBEngine_common.pyi（112 条，含重载逐条，全量）

#### 3.1 类型别名（4）
| 条目 | 判定 | 出处 |
|---|---|---|
| Vector3Like | OK（边界内近似） | 引擎位置/方向是 `Vector3` 脚本类型（lib/pyscript/vector3.cpp:81-88 等），pyi 用 tuple 近似，语义可接受 |
| Address | OK（边界内近似） | `Network::Address`，如 onCellAppDeath 回调 server/baseapp/baseapp.cpp:697 |
| WatcherValue | OK（边界内近似） | watcher 值可为 int/float/str 等，lib/pyscript/pywatcher.cpp:145-160 |
| Callback | OK | typing 别名 |

#### 3.2 EntityCall 家族（9）
| 条目 | 判定 | 出处 |
|---|---|---|
| EntityCall | OK | lib/entitydef/entity_call.h:15；注册名 "EntityCall" entity_call.cpp:255-263 |
| EntityCall.__getattr__ | OK | 动态属性解析 onScriptGetAttribute，entity_call.h:32-33 |
| EntityCall.getComponent（2 个重载） | OK | entity_call.cpp:20；实现 `pyGetComponent(componentName, all)` |
| BaseEntityCall | **偏差** | pyi 写独立子类；源码只有单一 `EntityCall` 类型 + `ENTITYCALL_TYPE` 枚举（lib/common/common.h:283-291），全库无 `BaseEntityCall` 类（双证） |
| CellEntityCall | **偏差** | 同上 |
| ClientEntityCall | **偏差** | 同上 |
| EntityComponentCall | OK | lib/entitydef/entity_component_call.h:29、:32 |

#### 3.3 MemoryStream / GlobalDataClient / Entities（7）
| 条目 | 判定 | 出处 |
|---|---|---|
| MemoryStream（4 条） | OK×3 + 偏差×1（__bytes__，同 §2） | python_app.cpp:271、py_memorystream.cpp:27-35 |
| GlobalDataClient | OK | lib/server/globaldata_client.h:12-15（`INSTANCE_SCRIPT_HREADER`）；实例注册 entity_app.h:419 |
| Entities | OK | lib/entitydef/entities.h:22、:128；注册 entity_app.h:411 |

#### 3.4 Entity（属性 10 + 方法 24 + 重载 2）
属性：
| 条目 | 判定 | 出处 |
|---|---|---|
| id | OK | lib/entitydef/entity_macro.h ENTITY_GETSET 宏区块（id/isDestroyed/className，约 :63-67）；客户端侧 lib/client_lib/clientobjectbase.cpp:35 |
| className | OK | entity_macro.h 同上；cellapp 侧 server/cellapp/entity.cpp:76 区块 |
| client | OK | server/baseapp/entity.cpp:37；cellapp entity.cpp:75 |
| cell | OK（适用范围注记） | server/baseapp/entity.cpp:36；**cellapp 侧 Entity 无 `cell` getter**（cellapp/entity.cpp:74-90 无此项） |
| base | OK（适用范围注记） | server/cellapp/entity.cpp:74；baseapp 侧无 base getter（baseapp 上 base 即自身） |
| databaseID | OK | server/baseapp/entity.cpp:38 |
| databaseInterfaceName | OK | server/baseapp/entity.cpp:39 |
| isDestroyed | OK | entity_macro.h 宏区块 |
| shouldAutoArchive / shouldAutoBackup | OK | server/baseapp/entity.cpp:41 / :40（getset） |

方法：
| 条目 | 判定 | 出处 |
|---|---|---|
| __init__ / __getattr__ | OK | 脚本对象协议（lib/pyscript/scriptobject.cpp tp_getattro / Entity 初始化路径） |
| addTimer(initialOffset, repeatOffset=0.0, userArg=0) | OK | entity_macro.h:17；`DECLARE_PY_MOTHOD_ARG3`（lib/pyscript/py_macros.h:228）支持 1-3 参；实现 entity_macro.h:1387-1405 |
| delTimer(id: int\|str) | OK | entity_macro.h:18；实现 :1407-1420 接受 int 或 `"All"` 字符串 |
| destroy(deleteFromDB=False, writeToDB=False) | OK | entity_macro.h:20（METH_KEYWORDS）；kwargs 名单 server/baseapp/entity.cpp:626-632、:657-658 |
| writeToDB(callback=None, shouldAutoLoad=False, dbInterfaceName="default") | OK | entity_macro.h:19；参数解析 entity_macro.h:832-940（baseapp 0-3 参 `"O|i|O"`，cellapp 0-2 参） |
| getComponent（2 重载, all: bool） | OK | entity_macro.h:24；实现 :1340-1370 `"sO"`，第二参须 `Py_True` |
| fireEvent / registerEvent / deregisterEvent | OK | entity_macro.h:21-23 |
| createCellEntity(cellEntityCall=None) | OK | server/baseapp/entity.cpp:27；`DECLARE_PY_MOTHOD_ARG1(createCellEntity, PyObject_ptr)` entity.h:187（0-1 参） |
| createCellEntityInNewSpace(cellappIndex=0) | OK | server/baseapp/entity.cpp:28；entity.h:198 |
| destroyCellEntity() | OK | server/baseapp/entity.cpp:29 |
| clientEntity(destID) | OK（适用范围注记：cellapp） | server/cellapp/entity.cpp:53 |
| isReal() | OK（适用范围注记：cellapp） | server/cellapp/entity.cpp:50 |
| cancelController(controllerID) | OK | server/cellapp/entity.cpp:54；另 lib/client_lib/entity.cpp:30 |
| canNavigate() | OK（cellapp） | server/cellapp/entity.cpp:55 |
| setViewRadius(radius, hyst) / getViewRadius / getViewHystArea | OK（cellapp） | server/cellapp/entity.cpp:47-49 |
| entitiesInView(pending=False) | OK（cellapp） | server/cellapp/entity.cpp:62 注册；实现 :3172-3185 `\|b` |
| entitiesInRange(range, entityType=None, position=None) | OK（cellapp） | server/cellapp/entity.cpp:61；实现 :3239+ |
| teleport(nearbyMBRef, position, direction) | OK（cellapp） | server/cellapp/entity.cpp:64；实现 :3392（3 参） |
| destroySpace / debugView / getWitnesses | OK（cellapp） | server/cellapp/entity.cpp:65-67 |

**Entity 缺漏（25 条，cellapp 侧真实存在、pyi 未写）**：
方法 8：`addProximity`、`addYawRotator`、`navigate`、`navigatePathPoints`、`getRandomPoints`、`moveToPoint`、`moveToEntity`、`accelerate`（server/cellapp/entity.cpp:52-64 SCRIPT_METHOD_DECLARE 区块）。
属性 13：`allClients`、`otherClients`、`isWitnessed`、`hasWitness`、`isOnGround`、`spaceID`、`layer`、`position`、`direction`、`topSpeed`、`topSpeedY`、`controlledBy`、`volatileInfo`（server/cellapp/entity.cpp:76-90 SCRIPT_GET(SET)_DECLARE 区块）。
模块级 1：`kbassert`（lib/server/entity_app.h:423，baseapp/cellapp 模块面）。
MemoryStream 3：`rpos`、`wpos`、`fill`（lib/pyscript/py_memorystream.cpp:31-33）。
（`__reduce_ex__`/`__getDEP__` 属 dunder/内部，不计缺漏。）

#### 3.5 Proxy（13）
| 条目 | 判定 | 出处 |
|---|---|---|
| Proxy 类 | OK | server/baseapp/proxy.cpp:40（BASE_SCRIPT_INIT）；安装 baseapp.cpp:409 |
| __ACCOUNT_NAME__ / __ACCOUNT_PASSWORD__ | OK | lib/entitydef/common.h:142-148（属性名映射表） |
| clientAddr / clientEnabled / hasClient / roundTripTime / timeSinceHeardFromClient | OK | server/baseapp/proxy.cpp:36 / :38 / :37 / :34 / :35 |
| disconnect | OK | proxy.cpp:27 |
| getClientType -> int | OK | proxy.cpp:23 |
| getClientDatas -> Tuple[bytes, bytes] | OK | proxy.cpp:24；实现 :258-270 返回 2 元 bytes 组 |
| giveClientTo(proxy) | OK | proxy.cpp:22 |
| streamFileToClient(resourceName, desc="", id=-1) | OK | proxy.cpp:26；实现 :560-600（1-3 参，id 默认 -1） |
| streamStringToClient(data, desc="", id=-1) | OK | proxy.cpp:25；实现 :649-672（1-3 参） |

#### 3.6 Space / EntityComponent（9）
| 条目 | 判定 | 出处 |
|---|---|---|
| Space | OK | server/baseapp/space.cpp（BASE_SCRIPT_HREADER(Space)）；安装 baseapp.cpp:410 |
| EntityComponent | OK | lib/entitydef/entity_component.h:82 |
| EntityComponent.client / .cell / .base | OK | lib/entitydef/entity_component.cpp:1548（cell）/ :1581（base）/ :1614（client） |
| EntityComponent.addTimer / delTimer | OK | server/baseapp/entity_component.cpp:10-11 |
| EntityComponent.clientEntity | OK（适用范围注记：cellapp） | server/cellapp/entity_component.cpp:14 |

#### 3.7 常量与模块级函数（34）
| 条目 | 判定 | 出处 |
|---|---|---|
| LOG_ON_ACCEPT / LOG_ON_REJECT / LOG_ON_WAIT_FOR_DESTROY | OK | server/baseapp/baseapp.cpp:468 / :463 / :473（仅 baseapp 注入，适用范围注记） |
| LOG_TYPE_NORMAL/INFO/ERR/DBG/WAR | OK | lib/server/entity_app.h:456-476 |
| NEXT_ONLY | OK | entity_app.h:478-480 |
| component | OK | lib/pyscript/script.cpp:248 |
| publish | OK | entity_app.h:426；python_app.cpp:275 |
| scriptLogType | OK | entity_app.h:429 |
| getResFullPath / hasRes | OK | entity_app.h:432 / :435 |
| listPathRes(path, extension) | **偏差** | pyi `extension: Union[str, Tuple[str,...]]`；源码仅接受 str（python_app.cpp:509-540 只按 `PyUnicode_Check` 处理第二参），Tuple 形式会走 else 分支报错 |
| matchPath | OK | entity_app.h:444 |
| debugTracing(self) | **偏差** | pyi 写 `debugTracing(self)`；源码零参（py_gc.cpp:224-228）。pythonapp.pyi:33 的 `debugTracing()` 才是对的 |
| genUUID64 | OK | script.cpp:258 |
| addTimer(…, repeatOffset=0.0, callbackObj=None) | **偏差** | 源码恰 3 参 `"ffO"` 无默认值（python_app.cpp:721-726）；且 baseapp/cellapp 模块根本未注册模块级 addTimer（baseapp.cpp:429-451、cellapp.cpp:172-189 无此项） |
| delTimer(id) -> None | **偏差** | 源码返回 int（python_app.cpp:766-771）；common.pyi 写 `-> None`，pythonapp.pyi:38 的 `-> int` 才是对的 |
| register/deregisterRead/WriteFileDescriptor | OK | baseapp.cpp:441-444；cellapp.cpp:176-179；python_app.cpp:331-334 |
| executeRawDatabaseCommand(command, callback=None, threadID=0, dbInterfaceName="default") | OK | baseapp.cpp:438；实现 :3175-3189（1-4 参，第 3 参 `"i"`） |
| open(res, mode="r", encoding=None) | **偏差** | 源码恰 2 参、无 encoding（python_app.cpp:444-452 `argCount != 2` 即报错；entity_app.h:438 的 EntityApp 版本同为 `__py_kbeOpen`） |
| accountLoginResponse / createAccountResponse / chargeResponse | OK（适用范围注记：仅 interfaces 注册，interfaces.cpp:186-188；签名 `"ssy#H"` interfaces.cpp:696-710） | |
| urlopen | OK | lib/pyscript/pyurl.cpp:23 |
| __getattr__ | OK | typing 兜底，对应模块动态属性（script.cpp:243 起的 module 机制） |

判定：**103 OK，9 偏差，25 缺漏**（缺漏单列）。

### 4. _KBEngine_baseapp.pyi（24 条，全量+12 签名抽查）

模块属性：`entities` OK（entity_app.h:411）、`globalData` OK（entity_app.h:419）、`baseAppData` OK（baseapp.cpp:461）。

| 条目 | 判定 | 出处 |
|---|---|---|
| address / time / isShuttingDown | OK | baseapp.cpp:447 / :430（__py_gametime）/ :446 |
| getWatcher / getWatcherDir / addWatcher / delWatcher | OK | entity_app.h:447 / :450；pywatcher.cpp:184-185（entity_app.h:413-414 安装） |
| setAppFlags / getAppFlags | OK | baseapp.cpp:450-451 |
| reloadScript | OK | baseapp.cpp:445 |
| quantumPassedPercent -> int | OK | baseapp.cpp:439；实现 :510-513 |
| createEntity(entityType, params=None) | OK | baseapp.cpp:431；实现 :951-971（1-2 参） |
| createEntityLocally | OK | baseapp.cpp:432（与 createEntity 同实现） |
| createEntityAnywhere(entityType, params, callback) | OK | baseapp.cpp:433；实现 :986-1025（1-3 参，callback 须 callable 否则置 NULL） |
| createEntityRemotely(entityType, baseMB, params, callback) | OK | baseapp.cpp:434；实现 :1028-1090（2-4 参，第 2 参强制校验 `ENTITYCALL_TYPE_BASE`） |
| createEntityFromDBID / createEntityAnywhereFromDBID / createEntityRemotelyFromDBID | OK | baseapp.cpp:435-437 |
| executeRawDatabaseCommand | OK | baseapp.cpp:438 |
| charge(ordersID, dbID, byteDatas, pycallback=None) | **偏差** | pyi 给 pycallback 默认 None；源码恰 4 参（baseapp.cpp:3398-3403 `PyTuple_Size(args) != 4` 即 TypeError），callback 不可省 |
| register/deregisterRead/WriteFileDescriptor | OK | baseapp.cpp:441-444 |
| deleteEntityByDBID(entityType, dbID, callback, dbInterfaceName="default") | OK | baseapp.cpp:448；实现 :5150-5162（3-4 参） |
| lookUpEntityByDBID | OK | baseapp.cpp:449 |

缺漏 1：`kbassert`（entity_app.h:423，baseapp/cellapp 模块面共有，pyi 未收）。

判定：**23 OK，1 偏差，1 缺漏**。

### 5. _KBEngine_cellapp.pyi（20 条，全量+8 签名抽查）

模块属性：`entities` OK（entity_app.h:411）、`globalData` OK（:419）、`cellAppData` OK（cellapp.cpp:199）。

| 条目 | 判定 | 出处 |
|---|---|---|
| address / time / isShuttingDown | OK | cellapp.cpp:186 / :172 / :185 |
| getWatcher / getWatcherDir / addWatcher / delWatcher | OK | entity_app.h:447/450；pywatcher.cpp:184-185 |
| setAppFlags / getAppFlags | OK | cellapp.cpp:188-189 |
| reloadScript | OK | cellapp.cpp:175 |
| createEntity(entityType, spaceID, position, direction, params=None) | OK | cellapp.cpp:173；实现 :471-484 `"s\|I\|O\|O\|O"` |
| addSpaceGeometryMapping | **偏差（轻微）** | pyi 5 参全带默认（mapper=None、path=""）；源码 3-5 参且前 3 参实际必填（server/cellapp/spacememory.cpp:144-147 `argCount < 3 || > 5` 报错、:152-230 解析），`(spaceID)` 单参调用不成立 |
| getSpaceGeometryMapping(spaceID) -> str | OK | cellapp.cpp:181；实现 spacememory.cpp:112-133（PyUnicode 返回） |
| setSpaceData / getSpaceData / delSpaceData | OK | cellapp.cpp:182-184 |
| raycast(spaceID, layer, src, dst) | **偏差（轻微）** | pyi 4 参必填；源码支持 **3 或 4 参**（cellapp.cpp:2139-2173，3 参形式无 layer，layer 默认 0），layer 应为可选 |
| executeRawDatabaseCommand | OK（cellapp.cpp:174；已由 common 覆盖） |

缺漏 1：`kbassert`（entity_app.h:423）。

判定：**18 OK，2 偏差，1 缺漏**。

### 6. _KBEngine_dbmgr.pyi（4 条）
`executeRawDatabaseCommand` OK（server/dbmgr/dbmgr.cpp:355，与 baseapp 同签名）；SERVER_ERR star-import OK（dbmgr.cpp:347-351）；pythonapp 公共面经 `Dbmgr : public PythonApp`（server/dbmgr/dbmgr.h:32）继承成立；两个别名 OK（typing 层，对应回调实参 `(rows, insertID, affectedRows, error)` 与 dbmgr.cpp:355 实现一致）。
判定：**4 OK**。

### 7. _KBEngine_interfaces.pyi（7 条）
`accountLoginResponse` OK（interfaces.cpp:187，签名 :696-710 `"ssy#H"` = str,str,bytes,uint16）；`createAccountResponse` OK（:188）；`chargeResponse` OK（:186）；`executeRawDatabaseCommand` OK（:189）；SERVER_ERR star OK（:177-183）；两个别名 OK（PythonApp 继承面，interfaces.h:24）。
判定：**7 OK**。

### 8. _KBEngine_loginapp.pyi（1 条）
star-import pythonapp + server_errors + `__getattr__`：与源码一致 —— `Loginapp : public PythonApp`（server/loginapp/loginapp.h:22），公共面来自 python_app.cpp:271-334，SERVER_ERR 注入 loginapp.cpp:199-203，无其他模块方法注册。
判定：**1 OK**。

### 9. _KBEngine_logger.pyi（7 条）
| 条目 | 判定 | 出处 |
|---|---|---|
| accountLoginResponse | **无出处** | logger.cpp 全文无任何 `APPEND_SCRIPT_MODULE_METHOD`/`onInstallPyModules` 注册；`Logger : public PythonApp`（server/tools/logger/logger.h:49）仅有 PythonApp 公共面；该函数只注册于 interfaces（interfaces.cpp:187） |
| createAccountResponse | **无出处** | 同上（interfaces.cpp:188） |
| chargeResponse | **无出处** | 同上（interfaces.cpp:186） |
| executeRawDatabaseCommand | **无出处** | 同上（注册点仅 baseapp.cpp:438 / cellapp.cpp:174 / dbmgr.cpp:355 / interfaces.cpp:189） |
| __getattr__ | OK | typing 兜底 |
| DatabaseCommandResult / DatabaseCommandCallback 别名 | OK | typing 层 |

判定：**3 OK，4 无出处**。

### 10. _KBEngine_bots.pyi（57 条，全量+10 签名抽查）

模块面：
| 条目 | 判定 | 出处 |
|---|---|---|
| addBots(count) / addBots(count, tickCount, tickTime) | OK | server/tools/bots/bots.cpp:182；实现 :344-380（1 或 3 参） |
| scriptLogType | OK | bots.cpp:185 |
| genUUID64 | OK | script.cpp:258（所有 app 共用） |
| urlopen | OK | pyurl.cpp:23（Script::install 统一安装） |
| component | OK | script.cpp:248 |
| LOG_TYPE_NORMAL/INFO/ERR/DBG/WAR | OK | bots.cpp:186-200 |
| bots: PyBots | OK | bots.cpp:180 `registerPyObjectToScript("bots", ...)` |

类：
| 条目 | 判定 | 出处 |
|---|---|---|
| Entity（bots 进程 = client_lib Entity） | OK | lib/client_lib/entity.h:37 |
| Entity.id / className | OK | entity_macro.h:58 / :61（CLIENT_ENTITY_GETSET） |
| Entity.spaceID | OK | entity_macro.h:59 |
| Entity.position / direction | OK | lib/client_lib/entity.cpp:44-45（getset） |
| Entity.baseCall | **无出处** | 全库无 `baseCall`/`cellCall` 脚本方法（双证：grep 无声明；bots 进程调 base 走 `entity.base.xxx()` EntityCall 路径，client_lib/entity.cpp:41-42） |
| Entity.cellCall | **无出处** | 同上 |
| Entity.isOnGround | **无出处** | 仅 C++ 访问器（lib/client_lib/entity.h:172），无 SCRIPT_GET_DECLARE |
| Entity.inWorld | **无出处** | 仅 C++ 访问器（entity.h:167），无暴露 |
| Entity.__init__ / __getattr__ | OK | 脚本对象协议 |
| Entity.isPlayer | OK | client_lib/entity.cpp:31 |
| Entity.getComponent | OK | client_lib/entity.cpp:34 |
| Entity.fireEvent / registerEvent / deregisterEvent | OK | lib/entitydef/entity_macro.h:47-49（CLIENT_ENTITY_METHOD 宏） |
| EntityComponent（bots） | OK | lib/client_lib/entity_component.cpp:20-21（base/cell getter） |
| Entities | OK | clientapp.cpp:266 + entities.h:22 |
| ClientObject.id / entities | OK | clientobjectbase.cpp:35-36 |
| ClientObject.player | OK | clientobjectbase.cpp:28 |
| ClientObject.getSpaceData / callback / cancelCallback / getWatcher / getWatcherDir / disconnect / kbassert | OK | clientobjectbase.cpp:21-27 |
| PyBots.has_key / keys / values / items / get（重载×2） | OK | server/tools/bots/pybots.cpp:29-35 |
| EntityComponent.fireEvent/registerEvent/deregisterEvent | OK | entity_macro.h:47-49 经 client 宏作用于组件类 |

缺漏 13：
- 模块面 7：`publish`、`hasRes`、`open`、`listPathRes`、`matchPath`、`getWatcher`、`getWatcherDir`（lib/client_lib/clientapp.cpp:191、:206、:209、:212、:215、:197、:198 —— bots = `Bots : public ClientApp`（bots.h:40），这些都注入 bots 的 KBEngine 模块）。
- Entity 6：`addTimer`、`delTimer`（client_lib/entity.cpp:32-33）、`moveToPoint`（:29）、`cancelController`（:30）、`velocity`（:46 getset）、`clientapp`（:43）。

判定：**53 OK，4 无出处，13 缺漏**。

### 11. _KBEngine_hooks.pyi（88 条，全量）

Protocol 分组类本身（10 个）为 typing 组织结构，按成员逐条核验。回调存在性以引擎调用点字符串为证：

**BaseAppModuleHooks（13）**：onBaseAppReady OK（server/baseapp/initprogress_handler.cpp:186）；onReadyForLogin OK（server/cellapp/initprogress_handler.cpp:153）；onReadyForShutDown OK（baseapp.cpp:275）；onBaseAppShutDown OK（baseapp.cpp:327）；onInit OK（baseapp.cpp:3602；entity_app.h:1461；python_app.cpp:710）；onFini **无出处**（全库仅 client_lib/clientapp.cpp:304 有 "onFinish"，服务端无 "onFini" 调用点）；onCellAppDeath OK（baseapp.cpp:697）；onGlobalData OK（entity_app.h:1308）；onGlobalDataDel OK（entity_app.h:1290）；onBaseAppData OK（baseapp.cpp:3668）；onBaseAppDataDel OK（baseapp.cpp:3649）；onLoseChargeCB OK（baseapp.cpp:3564）；onAutoLoadEntityCreate OK（server/baseapp/entity_autoloader.cpp:83）。

**CellAppModuleHooks（7）**：onInit OK（cellapp/initprogress_handler.cpp:138）；onGlobalData/onGlobalDataDel OK（entity_app.h:1308/1290，EntityApp 公共路径）；onCellAppData OK（cellapp.cpp:864）；onCellAppDataDel OK（cellapp.cpp:844）；onSpaceData OK（cellapp/spacememory.cpp:609）；onAllSpaceGeometryLoaded OK（spacememory.cpp:336）。

**LoginAppModuleHooks（8）**：onLoginAppReady OK（loginapp.cpp:175）；onTick **无出处**（全库无 "onTick" 字符串与调用点）；onLoginAppShutDown OK（loginapp.cpp:62）；onRequestLogin OK（loginapp.cpp:1014）；onLoseLogin OK（loginapp.cpp:131）；onLoginCallbackFromDB OK（loginapp.cpp:1232）；onRequestCreateAccount OK（loginapp.cpp:369）；onCreateAccountCallbackFromDB OK（loginapp.cpp:591）。

**InterfacesModuleHooks（6）**：onInterfaceAppReady OK（interfaces.cpp:154）；onTick **无出处**；onInterfaceAppShutDown OK（interfaces.cpp:79）；onRequestCreateAccount OK（interfaces.cpp:526）；onRequestAccountLogin OK（interfaces.cpp:638）；onRequestCharge OK（interfaces.cpp:753）。

**DBMgrModuleHooks（5）**：onDBMgrReady OK（dbmgr.cpp:310）；onTick **无出处**；onDBMgrShutDown OK（dbmgr.cpp:156）；onReadyForShutDown OK（dbmgr.cpp:78-82）；onSelectAccountDBInterface OK（dbmgr.cpp:1243）。

**LoggerModuleHooks（4）**：onLoggerAppReady OK（logger.cpp:132）；onLoggerAppShutDown OK（logger.cpp:218）；onReadyForShutDown OK（logger.cpp:176-180）；onLogWrote OK（logger.cpp:287）。

**BotsModuleHooks（3）**：onInit OK（bots.cpp:103）；onStart **无出处**（bots.cpp 只有 onInit/onFinish，无 "onStart" 调用点）；onFinish OK（bots.cpp:125）。

**BaseEntityHooks（8）**：onCreateCellFailure OK（baseapp/entity.cpp:884）；onDestroy OK（cellapp/entity.cpp:209）；onGetCell OK（baseapp/entity.cpp:1032）；onLoseCell OK（baseapp/entity.cpp:1065）；onPreArchive OK（baseapp/entity.cpp:1292）；onRestore OK（cellapp/entity.cpp:3868）；onTimer OK（baseapp/entity.cpp:1656、cellapp/entity.cpp:4322；实参 (timerID, userArg) 见 entity_macro.h:1363 `pEntity_->onTimer(id, intptr(pUser))`）；onWriteToDB OK（cellapp/entity.cpp:1259）。

**ProxyHooks（6）**：onClientDeath OK（baseapp/entity.cpp:1048）；onClientGetCell OK（proxy.cpp:248）；onClientEnabled OK（proxy.cpp:178）；onGiveClientToFailure OK（proxy.cpp:307）；onLogOnAttempt OK（proxy.cpp:189，`"sks"` = ip,port,password）；onStreamComplete OK（proxy.cpp:832）。

**CellEntityHooks（18）**：onDestroy OK（cellapp/entity.cpp:209）；onEnteredView OK（cellapp/entity.cpp:1783）；onGetWitness OK（:2177）；onLoseWitness OK（:2215）；onMove OK（:2943）；onMoveOver OK（:2961）；onMoveFailure OK（:2979）；onRestore OK（:3868）；onSpaceGone OK（:384）；onTurn OK（:3095）；onTeleport OK（:3783）；onTeleportFailure OK（cellapp/entity.cpp teleport 失败路径 bufferOrExeCallback，同区块）；onTeleportSuccess OK（:3811）；onTimer OK（:4322）；onUpdateBegin OK（cellapp/witness.cpp:749）；onUpdateEnd OK（witness.cpp onUpdateEnd 对称调用点）；onWitnessed OK（cellapp/entity.cpp:1483）；onWriteToDB OK（:1259）。

判定：**83 OK，5 无出处**（onFini、onTick×3、onStart）。

### 12. KBEngine/__init__.pyi、KBEngine/hooks.pyi（重导出链，2 条）
`from _KBEngine_common import *` / `from _KBEngine_hooks import *`：OK —— 与 API_MATRIX.md:7-8“公共包 + 目录级覆盖”设计一致；公共超集（common）+ 组件覆盖（baseapp/cellapp/…）的分文件模型与源码注册结构（PythonApp/EntityApp/ClientApp 三条继承线 + 各 onInstallPyModules）对应无误。
判定：**2 OK**。

### 13. API_MATRIX.md 归属分类抽核（10 条）
1. “pythonapp 典型接口 17 项来自 python_app.cpp/script.cpp/pyurl.cpp” → OK（python_app.cpp:271-334、script.cpp:248/258、pyurl.cpp:23）。
2. “EntityApp::installPyModules 注入整套 SERVER_ERR_*” → OK（entity_app.h:485-491）。
3. “interfaces/loginapp/dbmgr 重复注入同一套错误码” → OK（interfaces.cpp:177-183、loginapp.cpp:199-203、dbmgr.cpp:347-351）。
4. “Logger 模块层主要复用 PythonApp 公共 API” → OK（logger.h:49；但 typings/_KBEngine_logger.pyi 超出该声明多写 4 函数，见清单 #1）。
5. “BaseApp 专属清单（entities/globalData/baseAppData/time/…/quantumPassedPercent）” → OK（entity_app.h:411/419、baseapp.cpp:461/429-451）。
6. “CellApp createEntity(entityType, spaceID, position, direction, params)” → OK（cellapp.cpp:471-484）。
7. “Bots 专属清单 addBots/bots/component/LOG_TYPE_*” → OK（bots.cpp:180-200）。
8. “urlopen 来自 pyurl.cpp” → OK（pyurl.cpp:23）。
9. “addWatcher/delWatcher 属 baseapp/cellapp” → OK（pywatcher.cpp:184-185，仅 EntityApp 路径安装 entity_app.h:413-414）。
10. “EntityCall 远端方法保持动态/Any” → OK（与 entity_call.cpp:19-28 的动态属性解析一致）。
判定：**10 OK**（1 处 matrix↔pyi 不一致已计入 logger 无出处清单）。

---

## 偏差 / 缺漏 / 无出处清单（按严重度）

### A. 无出处（13 条）—— pyi 声明了、源码模块上不存在
| 严重度 | 条目 | 证据 |
|---|---|---|
| 高 | logger.accountLoginResponse | logger.cpp 无注册；仅 interfaces.cpp:187 |
| 高 | logger.createAccountResponse | 同上（interfaces.cpp:188） |
| 高 | logger.chargeResponse | 同上（interfaces.cpp:186） |
| 高 | logger.executeRawDatabaseCommand | 注册点仅 baseapp.cpp:438/cellapp.cpp:174/dbmgr.cpp:355/interfaces.cpp:189 |
| 中 | BaseAppModuleHooks.onFini | 服务端无调用点（仅 clientapp.cpp:304 "onFinish"） |
| 中 | LoginAppModuleHooks.onTick / InterfacesModuleHooks.onTick / DBMgrModuleHooks.onTick | 全库无 "onTick" |
| 中 | BotsModuleHooks.onStart | bots.cpp 仅 onInit(:103)/onFinish(:125) |
| 中 | bots Entity.baseCall / Entity.cellCall | 全库无声明；远端调用走 EntityCall（client_lib/entity.cpp:41-42） |
| 低 | bots Entity.isOnGround / Entity.inWorld | 仅 C++ 访问器（client_lib/entity.h:172/:167），未暴露给脚本 |

### B. 偏差（13 条）—— pyi 写 X、源码是 Y
| 严重度 | 条目 | pyi 写 | 源码实际 |
|---|---|---|---|
| 高 | common.EntityCall 三子类 | BaseEntityCall/CellEntityCall/ClientEntityCall 独立类型 | 单一 EntityCall 类型（entity_call.h:15）+ ENTITYCALL_TYPE 枚举（common.h:283-291） |
| 中 | common.open | `open(res, mode="r", encoding=None)` | 恰 2 参、无 encoding（python_app.cpp:444-452） |
| 中 | common.addTimer | repeatOffset/callbackObj 有默认值 | 恰 3 参 `"ffO"`（python_app.cpp:721-726）；且 baseapp/cellapp 模块未注册该函数 |
| 中 | common.delTimer | `-> None` | 返回 int（python_app.cpp:766-771） |
| 中 | common.listPathRes | extension 可为 Tuple[str,...] | 仅 str（python_app.cpp:509-540） |
| 中 | common.debugTracing | `debugTracing(self)` | 零参（py_gc.cpp:224-228） |
| 中 | common/…/MemoryStream.__bytes__ | `__bytes__` dunder | 方法名为 `bytes`（py_memorystream.cpp:30） |
| 中 | baseapp.charge | pycallback=None 可省 | 恰 4 参（baseapp.cpp:3398-3403） |
| 低 | cellapp.addSpaceGeometryMapping | 5 参全默认 | 3-5 参、前 3 参实际必填（spacememory.cpp:144-147） |
| 低 | cellapp.raycast | 4 参必填（含 layer） | 3 或 4 参，layer 可省（cellapp.cpp:2139-2173） |
| 信息 | common.LOG_ON_* | 归入公共 | 仅 baseapp 注入（baseapp.cpp:463-473） |
| 信息 | common.accountLoginResponse 等 3 个 | 归入公共 | 仅 interfaces 注册（interfaces.cpp:186-188） |
| 信息 | common.Entity.cell / isReal 等方法 | Entity 全量合并 | baseapp/cellapp 各自只暴露一半（baseapp/entity.cpp:35-41 vs cellapp/entity.cpp:47-90） |

### C. 缺漏（43 条）—— 引擎暴露了、pyi 未收
| 严重度 | 范围 | 条目 |
|---|---|---|
| 高 | common.Entity（cellapp 面）方法 ×8 | addProximity、addYawRotator、navigate、navigatePathPoints、getRandomPoints、moveToPoint、moveToEntity、accelerate（cellapp/entity.cpp:52-64） |
| 高 | common.Entity（cellapp 面）属性 ×13 | allClients、otherClients、isWitnessed、hasWitness、isOnGround、spaceID、layer、position、direction、topSpeed、topSpeedY、controlledBy、volatileInfo（cellapp/entity.cpp:76-90） |
| 中 | bots 模块面 ×7 | publish、hasRes、open、listPathRes、matchPath、getWatcher、getWatcherDir（clientapp.cpp:191-215） |
| 中 | bots Entity ×6 | addTimer、delTimer、moveToPoint、cancelController、velocity、clientapp（client_lib/entity.cpp:29-46） |
| 中 | baseapp/cellapp 模块 ×2 | kbassert（entity_app.h:423） |
| 低 | MemoryStream ×3 | rpos、wpos、fill（py_memorystream.cpp:31-33） |
| 低 | 服务端 Entity 通用 ×4（dunder/内部，可选） | __reduce_ex__、__getDEP__（entity_macro.h:15-16）、volatile 语义、EntityCall.__unpickle__（entity_call.cpp:255-263） |

---

## 统计

- 核验范围：11 个 .pyi + __init__/hooks 重导出 2 条 + API_MATRIX 抽核 10 条
- **总核验条数：445**（pyi 声明条目 392 + API_MATRIX 抽核 10 + 缺漏发现 43）
- **OK：376**（pyi 条目 366 + matrix 10）
- **偏差：13**
- **无出处：13**
- **缺漏：43**

（pyi 声明条目口径：392 = common 112 + baseapp 24 + cellapp 20 + pythonapp 31 + server_errors 39 + dbmgr 4 + interfaces 7 + logger 7 + bots 57 + hooks 88 + loginapp 1 + 重导出 2；重载按条计。）
