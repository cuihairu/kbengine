# 12. cellData 与 Base-Cell 数据同步

> 本章回答：cellData 是什么？Base 和 Cell 之间如何同步数据？为什么同步不是实时的？延迟的来源是什么？

## 12.1 本章核心问题

- cellData 的本质是什么？
- backupCellData() 的完整调用链路是怎样的？
- SHA1 变化检测如何减少网络传输？
- 为什么同步不是实时的？
- onWriteToDB(cellData) 回调中应该做什么？
- 什么时候应该用 self.cell.xxx() 而不是 cellData？
- **为什么测试时会感觉到延迟？延迟的来源是什么？**

---

## 12.2 cellData 的本质

### 定义

cellData 是 **Base 侧的一个字典属性**，存储 Cell 属性的快照数据。

```python
# base/Account.py
class Account(KBEngine.Entity):
    def someMethod(self):
        # cellData 是一个字典
        hp = self.cellData.get("hp")
        position = self.cellData.get("position")
```

### 文档说明

根据 API 文档（`docs/api/baseapp/Entity.md`）：

> cellData 是一个字典属性。每当 base 实体没有创建它的 cell 实体时，cell 实体的属性会保存在这里。
> 如果 cell 实体被创建，这些用到的值和 cellData 属性将被删除。
>
> 除了 cell 实体在实体定义文件里指定的属性外，它还包含 position、direction 和 spaceID。

### 核心定位

```
┌─────────────────────────────────────────────────────────────────┐
│                    cellData 的本质                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  cellData 不是用来"实时查询 Cell 状态"的                        │
│                                                                 │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │ 它是 Base 和 Cell 数据之间的"持久化桥梁"                   │  │
│  │                                                           │  │
│  │ 1. Cell 数据平时在 CellApp                                │  │
│  │ 2. 存档/销毁时，汇总到 Base 的 cellData                   │  │
│  │ 3. Base 将 cellData + Base 属性一起写入数据库             │  │
│  └───────────────────────────────────────────────────────────┘  │
│                                                                 │
│  类比：                                                         │
│  cellData 就像是一个"暂存箱"，存档时把所有数据               │
│  （Base + Cell）整理好一起打包入库                             │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 12.3 同步机制

### 12.3.1 完整调用链路

```
┌─────────────────────────────────────────────────────────────────────────┐
│                        CellApp → BaseApp 同步流程                        │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  CellApp (发送端)                                                        │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │ Entity::backupCellData()  // cellapp/entity.cpp:1129           │   │
│  │   │                                                             │   │
│  │   ├── addCellDataToStream()  // 序列化 Cell 属性到流           │   │
│  │   │                                                             │   │
│  │   ├── 计算数据 SHA1 哈希                                       │   │
│  │   │                                                             │   │
│  │   ├── dataDirty = memcmp(digest, persistentDigest_)             │   │
│  │   │   // ⚠️ 关键：只有数据变化时才发送                          │   │
│  │   │                                                             │   │
│  │   └── bundle.newMessage(onBackupEntityCellData)                │   │
│  │       bundle << id_                                            │   │
│  │       bundle << dataDirty                                      │   │
│  │       if (dataDirty) bundle.append(cellData)                   │   │
│  │       baseEntityCall_->sendCall(bundle)  // 发送到 BaseApp      │   │
│  └─────────────────────────────────────────────────────────────────┘   │
│                               │                                          │
│                               ▼                                          │
│  BaseApp (接收端)                                                         │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │ Baseapp::onBackupEntityCellData()  // baseapp/baseapp.cpp:4674│   │
│  │   │                                                             │   │
│  │   ├── stream >> entityID                                       │   │
│  │   ├── pEntity = findEntity(entityID)                           │   │
│  │   │                                                             │   │
│  │   └── pEntity->onBackupCellData(channel, stream)               │   │
│  └─────────────────────────────────────────────────────────────────┘   │
│                               │                                          │
│                               ▼                                          │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │ Entity::onBackupCellData()  // baseapp/entity.cpp:1109        │   │
│  │   │                                                             │   │
│  │   ├── stream >> isDirty                                        │   │
│  │   │                                                             │   │
│  │   ├── if (isDirty)                                              │   │
│  │   │   ├── cellData = createCellDataFromStream(&stream)         │   │
│  │   │   ├── installCellDataAttr(cellData)  // 更新 self.cellData  │   │
│  │   │   └── setDirty()  // 标记需要存档                           │   │
│  └─────────────────────────────────────────────────────────────────┘   │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

---

### 12.3.2 SHA1 变化检测机制（核心）

#### 数据结构定义

**源码位置**：`cellapp/entity.h:683`

```cpp
// 需要持久化的数据是否变脏（内存sha1），如果没有变脏不需要持久化
uint32 persistentDigest_[5];
```

> SHA1 哈希输出 160 位 = 20 字节 = 5 个 uint32

#### backupCellData() 完整实现

**源码位置**：`cellapp/entity.cpp:1129-1189`

```cpp
void Entity::backupCellData()
{
    AUTO_SCOPED_PROFILE("backup");

    // 1. 必须有 Base 实体才能同步
    if(baseEntityCall_ != NULL)
    {
        // 2. 创建内存流用于序列化
        MemoryStream* s = MemoryStream::createPoolObject(OBJECTPOOL_POINT);

        try
        {
            // 3. 将 Cell 属性序列化到流
            addCellDataToStream(BASEAPP_TYPE, ENTITY_CELL_DATA_FLAGS, s);
        }
        catch (MemoryStreamWriteOverflow & err)
        {
            ERROR_MSG(fmt::format("{}::backupCellData({}): {}\n",
                scriptName(), id(), err.what()));
            MemoryStream::reclaimPoolObject(s);
            return;
        }

        // 4. 计算 SHA1 哈希
        KBE_SHA1 sha;
        uint32 digest[5];
        sha.Input(s->data(), s->length());
        sha.Result(digest);

        // 5. ⚠️ 关键：与上次哈希比较，判断是否变化
        bool dataDirty = memcmp((void*)&persistentDigest_[0],
                                (void*)&digest[0],
                                sizeof(persistentDigest_)) != 0;

        // 检查数据是否有变化，有变化则将数据备份并且记录数据hash
        if (!dataDirty)
        {
            // 6a. 数据没变化：回收流，不发送数据
            MemoryStream::reclaimPoolObject(s);
        }
        else
        {
            // 6b. 数据有变化：更新哈希记录
            setDirty((uint32*)&digest[0]);
        }

        // 7. 构建网络消息
        Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
        (*pBundle).newMessage(BaseappInterface::onBackupEntityCellData);
        (*pBundle) << id_;           // 实体 ID
        (*pBundle) << dataDirty;     // 是否有数据变化

        // 8. 只有数据变化时才附加实际数据
        if (dataDirty)
        {
            (*pBundle).append(s);    // 附加 Cell 数据流
            MemoryStream::reclaimPoolObject(s);
        }

        // 9. ⚠️ 无论数据是否变化，都会发送消息（但数据内容不同）
        baseEntityCall_->sendCall(pBundle);
    }
}
```

#### setDirty() 实现

**源码位置**：`cellapp/entity.inl:263-277`

```cpp
INLINE void Entity::setDirty(uint32* digest)
{
    if (digest)
    {
        // 更新哈希记录
        memcpy((void*)&persistentDigest_[0], (void*)digest, sizeof(persistentDigest_));
    }
    else
    {
        // 清空哈希（标记为脏）
        persistentDigest_[0] = 0;
        persistentDigest_[1] = 0;
        persistentDigest_[2] = 0;
        persistentDigest_[3] = 0;
        persistentDigest_[4] = 0;
    }
}
```

#### isDirty() 实现

**源码位置**：`cellapp/entity.inl:280-287`

```cpp
INLINE bool Entity::isDirty() const
{
    // 全零表示初始状态（脏）
    return persistentDigest_[0] == 0 &&
           persistentDigest_[1] == 0 &&
           persistentDigest_[2] == 0 &&
           persistentDigest_[3] == 0 &&
           persistentDigest_[4] == 0;
}
```

---

### 12.3.3 同步触发时机

| 触发时机 | 源码位置 | 说明 |
|----------|----------|------|
| **Cell 销毁时** | `cellapp/entity.cpp:215` | `destroy() → onDestroy() → backupCellData()` |
| **存档时** | `cellapp/entity.cpp:1236` | `writeToDB() → backupCellData()` |
| **Base 请求时** | `cellapp/cellapp.cpp:771` | `reqBackupEntityCellData → backupCellData()` |

**重要**：Python 脚本无法直接调用 `backupCellData()`，这是 C++ 内部方法。

---

### 12.3.4 为什么同步不是实时的

```
┌─────────────────────────────────────────────────────────────────────┐
│                      cellData 同步特点                              │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  1. ❌ 不是实时同步                                                 │
│     - Cell 属性修改后，不会立即通知 Base                             │
│     - 只有在特定时机（销毁/存档/请求）才会触发                       │
│                                                                     │
│  2. ✅ 有变化检测机制                                               │
│     - 通过 SHA1 哈希对比，只有数据变化才发送实际数据                 │
│     - 减少网络传输和序列化开销                                      │
│                                                                     │
│  3. 📦 只在特定时机同步                                             │
│     - Cell 销毁时（onDestroy）                                      │
│     - 存档时（writeToDB）                                           │
│     - Base 请求时（通过 reqBackupEntityCellData）                    │
│                                                                     │
│  4. 🔒 Python 无法直接控制同步                                      │
│     - backupCellData() 是 C++ 内部方法                              │
│     - 没有 Python 接口可以主动触发                                   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 12.4 延迟来源分析

### 12.4.1 延迟的完整链路

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         延迟来源分解                                    │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  ┌──────────────────────────────────────────────────────────────────┐  │
│  │ 阶段1: 触发延迟                                                  │  │
│  │ ─────────────────────────────────────────────────────────────── │  │
│  │ 从 Cell 数据变化 → 到 backupCellData() 被调用                   │  │
│  │                                                                  │  │
│  │ 延迟：0秒 ~ 无限大（取决于触发时机）                              │  │
│  │                                                                  │  │
│  │ 场景：                                                           │  │
│  │ - 修改 HP 后，如果不触发存档/销毁，永远不会同步                  │  │
│  │ - 只有调用 writeToDB() 或 destroy() 时才会触发                  │  │
│  └──────────────────────────────────────────────────────────────────┘  │
│                               │                                          │
│                               ▼                                          │
│  ┌──────────────────────────────────────────────────────────────────┐  │
│  │ 阶段2: 变化检测延迟                                              │  │
│  │ ─────────────────────────────────────────────────────────────── │  │
│  │ backupCellData() 执行 SHA1 计算和比较                            │  │
│  │                                                                  │  │
│  │ 延迟：约 1-5 毫秒（取决于数据大小）                              │  │
│  │                                                                  │  │
│  │ 操作：                                                           │  │
│  │ 1. 序列化 Cell 属性到 MemoryStream                               │  │
│  │ 2. 计算 SHA1 哈希 (O(n) 复杂度)                                  │  │
│  │ 3. 与 persistentDigest_ 比较                                    │  │
│  │ 4. 如果没变化，直接返回（不发送）                                │  │
│  └──────────────────────────────────────────────────────────────────┘  │
│                               │                                          │
│                               ▼                                          │
│  ┌──────────────────────────────────────────────────────────────────┐  │
│  │ 阶段3: 网络传输延迟                                              │  │
│  │ ─────────────────────────────────────────────────────────────── │  │
│  │ CellApp → BaseApp 的网络通信                                    │  │
│  │                                                                  │  │
│  │ 延迟：5-50 毫秒（本地网络）或更高（远程）                        │  │
│  │                                                                  │  │
│  │ 操作：                                                           │  │
│  │ 1. 构建网络消息 Bundle                                           │  │
│  │ 2. 通过 baseEntityCall_ 发送                                     │  │
│  │ 3. TCP 传输                                                     │  │
│  │ 4. BaseApp 接收并解析                                            │  │
│  └──────────────────────────────────────────────────────────────────┘  │
│                               │                                          │
│                               ▼                                          │
│  ┌──────────────────────────────────────────────────────────────────┐  │
│  │ 阶段4: BaseApp 处理延迟                                          │  │
│  │ ─────────────────────────────────────────────────────────────── │  │
│  │ BaseApp 更新 cellData                                           │  │
│  │                                                                  │  │
│  │ 延迟：约 1-3 毫秒                                                │  │
│  │                                                                  │  │
│  │ 操作：                                                           │  │
│  │ 1. onBackupEntityCellData() 网络消息处理                         │  │
│  │ 2. onBackupCellData() 更新 self.cellData                        │  │
│  │ 3. 调用 installCellDataAttr()                                    │  │
│  └──────────────────────────────────────────────────────────────────┘  │
│                                                                          │
│  总延迟 ≈ 触发延迟 (不确定) + 变化检测 (~2ms) + 网络传输 (~10-50ms) + 处理 (~2ms) │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

### 12.4.2 延迟的关键因素

| 因素 | 延迟量级 | 说明 |
|------|----------|------|
| **触发延迟** | 最大（0 ~ ∞） | 取决于何时调用 writeToDB() |
| **SHA1 计算** | 1-5ms | 数据越大，计算越慢 |
| **网络往返** | 10-50ms（本地） | 主要延迟来源 |
| **序列化/反序列化** | 1-3ms | 属性越多，越慢 |
| **Base 处理** | 1-3ms | 更新 cellData 字典 |

### 12.4.3 优化设计的权衡

```
┌─────────────────────────────────────────────────────────────────────┐
│                  KBEngine 设计的权衡                                │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ⚠️  为什么不实时同步？                                             │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │ 实时同步的代价                                              │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │ 1. 每次属性修改都要网络传输                                 │   │
│  │ 2. 高频属性（如 position）会产生海量网络包                  │   │
│  │ 3. CellApp 和 BaseApp 都会因序列化而卡顿                    │   │
│  │ 4. 数据库写入频率无法控制                                    │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
│  ✅  当前设计的优势                                                 │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │ 批量 + 按需同步                                              │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │ 1. 只在需要时同步（存档/销毁）                               │   │
│  │ 2. SHA1 变化检测，避免无效传输                               │   │
│  │ 3. 支持自动存档间隔控制                                     │   │
│  │ 4. 网络开销可控                                             │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
│  结论：KBEngine 牺牲了实时性，换取了可扩展性和性能                   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

### 12.4.4 实际测试中的延迟表现

```python
# 测试场景：在 Cell 修改 HP，然后在 Base 查看 cellData

# Cell 端（cell/Avatar.py）
class Avatar(KBEngine.Entity):
    def modifyHP(self):
        self.hp = 50  # 修改 HP
        # 此时 Base 的 cellData 还没有更新！

        KBEngine.writeToDB()  # 触发存档，开始同步
        # 从这里开始，大约 10-50ms 后 Base 才能看到新值

# Base 端（base/Avatar.py）
class Avatar(KBEngine.Entity):
    def checkHP(self):
        # 可能读到旧值，因为同步还没完成
        hp = self.cellData.get("hp")
        print(f"Current HP: {hp}")
```

**时间线**：

```
t=0ms:    Cell 修改 self.hp = 50
t=1ms:    调用 writeToDB()
t=2ms:    backupCellData() 开始执行
t=5ms:    SHA1 计算完成，检测到变化
t=6ms:    构建网络消息
t=20ms:   BaseApp 收到消息
t=22ms:   Base 更新 self.cellData
t=23ms:   Python 可以读到新值

总延迟：约 20-25ms（本地）
```

---

## 12.5 使用场景

### 12.5.1 正确使用：onWriteToDB(cellData) 回调

**源码位置**：`baseapp/entity.cpp:1299`

```cpp
// 存档流程中调用 onWriteToDB
void Entity::onCellWriteToDBCompleted(...)
{
    onPreArchive();
    onWriteToDB();  // ← 这里会调用 Python 的 onWriteToDB(cellData)
    addPersistentsDataToStream(ED_FLAG_ALL, s);
    // ... 发送到 DBMgr
}
```

**实际例子**：

```python
# base/Account.py
class Account(KBEngine.Entity):
    def onWriteToDB(self, cellData):
        # ✅ 这就是 cellData 的主要用途！

        # 场景1: 存档前检查 Cell 数据
        if cellData.get("hp", 100) <= 0:
            # 玩家已死亡，不存档或特殊处理
            return

        # 场景2: 记录额外信息
        self.lastSaveTime = time.time()
        self.lastSaveHp = cellData.get("hp")
        self.lastSavePosition = cellData.get("position")

        # 场景3: 数据验证
        assert cellData.get("hp") >= 0, "HP 不能为负"
```

### 12.5.2 错误使用：试图实时获取 Cell 数据

```python
# ❌ 错误示例
class Account(KBEngine.Entity):
    def someMethod(self):
        # 这获取的可能是旧数据！
        hp = self.cellData.get("hp")
        if hp < 10:
            self.heal()  # 可能已经治疗了，但 cellData 还是旧值

# ✅ 正确做法：使用远程方法调用
class Account(KBEngine.Entity):
    def someMethod(self):
        # 调用 Cell 方法获取实时数据
        self.cell.checkHpAndHeal()
```

```xml
<!-- entity_defs/Account.def -->
<CellMethods>
    <checkHpAndHeal>
        <Exposed> 1 </Exposed>
    </checkHpAndHeal>
</CellMethods>
```

### 12.5.3 什么时候用 cellData，什么时候用 self.cell.xxx()

| 场景 | 推荐方式 | 原因 |
|------|----------|------|
| 存档前检查/修改 Cell 数据 | `onWriteToDB(cellData)` | cellData 在存档时已同步 |
| 获取 Cell 实时状态 | `self.cell.xxx()` | 远程方法调用，获取最新值 |
| Cell 销毁后保留数据 | `onLoseCell()` 中访问 cellData | 销毁时会自动同步 |
| 判断 Cell 是否存在 | `if self.cell:` | 检查 EntityCall 是否为空 |

---

## 12.6 源码走读

### 12.6.1 CellApp 发送端

**文件**：`kbe/src/server/cellapp/entity.cpp:1129-1189`

```cpp
void Entity::backupCellData()
{
    if(baseEntityCall_ != NULL)
    {
        // 1. 序列化属性
        addCellDataToStream(BASEAPP_TYPE, ENTITY_CELL_DATA_FLAGS, s);

        // 2. 计算哈希并检查变化
        sha.Input(s->data(), s->length());
        sha.Result(digest);
        bool dataDirty = memcmp(&persistentDigest_[0], &digest[0], ...) != 0;

        // 3. 没变化则不发送数据（但仍发送消息通知）
        if (!dataDirty) {
            MemoryStream::reclaimPoolObject(s);
        } else {
            setDirty((uint32*)&digest[0]);
        }

        // 4. 发送到 BaseApp
        (*pBundle).newMessage(BaseappInterface::onBackupEntityCellData);
        (*pBundle) << id_;
        (*pBundle) << dataDirty;
        if (dataDirty) (*pBundle).append(s);
        baseEntityCall_->sendCall(pBundle);
    }
}
```

### 12.6.2 BaseApp 接收端

**文件**：`kbe/src/server/baseapp/baseapp.cpp:4674-4696`

```cpp
void Baseapp::onBackupEntityCellData(Network::Channel* pChannel, MemoryStream& s)
{
    if(pChannel->isExternal()) return;

    ENTITY_ID entityID = 0;
    s >> entityID;

    Entity* pEntity = this->findEntity(entityID);
    if(pEntity) {
        pEntity->onBackupCellData(pChannel, s);
    }
}
```

**文件**：`kbe/src/server/baseapp/entity.cpp:1109-1126`

```cpp
void Entity::onBackupCellData(Network::Channel* pChannel, MemoryStream& s)
{
    if(pChannel->isExternal()) return;

    isGetingCellData_ = false;

    bool isDirty = false;
    s >> isDirty;

    if(isDirty) {
        // 从流中解析并更新 cellData
        PyObject* cellData = createCellDataFromStream(&s);
        installCellDataAttr(cellData);  // ← 更新 self.cellData
        Py_DECREF(cellData);
        setDirty();
    }
}
```

### 12.6.3 存档时使用 cellData

**文件**：`kbe/src/server/baseapp/entity.cpp:398-428`

```cpp
void Entity::addPersistentsDataToStream(uint32 flags, MemoryStream* s)
{
    // 1. 从 Base 实体的 __dict__ 读取 Base 属性
    PyObject* pydict = PyObject_GetAttrString(this, "__dict__");

    // 2. 从 cellData 读取 Cell 属性（关键！）
    ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs =
        pScriptModule_->getPersistentPropertyDescriptions();

    for(; iter != propertyDescrs.end(); ++iter) {
        PropertyDescription* propertyDescription = iter->second;

        // 从 cellDataDict_ 中查找 Cell 属性
        if (propertyDescription->hasCell()) {
            // 从 cellData 获取值并序列化到流中
            PyDict_GetItemString(cellDataDict_, attrname);
            // ... 序列化到流
        }
    }
}
```

---

## 12.7 关键总结

| 要点 | 说明 |
|------|------|
| **cellData 本质** | Base 侧的字典，存储 Cell 属性快照 |
| **同步方向** | Cell → Base（单向） |
| **同步时机** | Cell 销毁、存档、Base 请求 |
| **变化检测** | SHA1 哈希，只在变化时发送数据 |
| **延迟来源** | 触发时机（最大） + 网络 + 处理 |
| **主要用途** | 存档时在 Base 端收集所有数据 |
| **Python 控制** | 无法主动触发，只能通过 writeToDB() 等间接触发 |

### 核心原则

1. **cellData 用于持久化场景，不是实时查询**
2. **需要 Cell 实时数据时，用 `self.cell.xxx()` 远程调用**
3. **存档前的数据处理，在 `onWriteToDB(cellData)` 中进行**
4. **延迟主要来自于同步不是自动触发的，需要主动调用 writeToDB()**

### 如果需要更实时的同步

如果业务场景确实需要更实时的 Base-Cell 数据同步，可以考虑：

```python
# 方案1：定期同步（不推荐，网络开销大）
class Avatar(KBEngine.Entity):
    def __init__(self):
        self.addTimer(1.0, 1.0, 0)  # 每秒触发一次

    def onTimer(self, timerID, userArg):
        # 定期请求 Base 备份数据
        # 这需要 Cell 主动通知 Base
        pass

# 方案2：使用远程方法获取实时值（推荐）
class Avatar(KBEngine.Entity):
    def needsHP(self):
        # 直接调用 Cell 方法获取
        self.cell.getHP()
```

---

## 12.8 与其他专题的关系

- **实体创建**：[实体系统](/architecture/source-analysis/entity-system.md)
- **持久化流程**：[数据库与持久化](/architecture/source-analysis/persistence.md)
- **EntityCall 通信**：[RPC 与 EntityCall](/study/11-rpc-entitycall-and-communication-patterns.md)
- **EntityDef 定义**：[EntityDef 实体定义](/study/05-entitydef-and-entity-definition.md)
