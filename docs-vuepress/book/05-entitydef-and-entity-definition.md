# 5. EntityDef 与实体定义系统

> EntityDef 不是配置文件解析器。它是脚本描述 + 网络协议描述 + 持久化描述的**三合一运行时骨架**。改一个 .def 文件，同时影响 Python 类型、网络消息格式和数据库表结构。

## 5.1 本章核心问题

- `.def` 文件到底是什么？它怎样同时驱动脚本层、网络层、持久化层？
- 一个属性有哪些"身份"？为什么理解这些身份是读懂源码的前提？
- `ScriptDefModule` / `EntityDescription` 怎样从 XML 变成运行时对象？
- EntityDef 的 alias 机制如何让属性同步包变小？
- 组件系统（Component）怎样让实体定义变成对象树？

## 5.2 .def 文件：不是配置，而是运行时骨架

### KBEngine .def 示例

```xml
<!-- 文件：scripts/entity_defs/Account.def -->
<root>
    <Properties>
        <playerName>
            <Type>      STRING          </Type>
            <Flags>     ALL_CLIENTS     </Flags>
            <Persistent>true            </Persistent>
        </playerName>
        <level>
            <Type>      INT32           </Type>
            <Flags>     BASE_AND_CLIENT </Flags>
            <Default>   1               </Default>
        </level>
    </Properties>
    <ClientMethods>
        <chatMessage>
            <Arg>   STRING  </Arg>
        </chatMessage>
    </ClientMethods>
    <CellMethods>  </CellMethods>
    <BaseMethods>  </BaseMethods>
</root>
```

### BigWorld .def 示例

```xml
<!-- 文件：scripts/entity_defs/ClientAvatar.def -->
<root>
    <Volatile>
        <position/>
        <yaw/>
        <pitch>    20    </pitch>
    </Volatile>
    <Properties>
        <playerName>
            <Type>            STRING                </Type>
            <Flags>           ALL_CLIENTS           </Flags>
            <Persistent>      true                  </Persistent>
            <Editable>        true                  </Editable>
            <Identifier>      true                  </Identifier>
        </playerName>
        <prop1>
            <Type>            INT32                 </Type>
            <Flags>           BASE_AND_CLIENT       </Flags>
            <Default>         1                     </Default>
        </prop1>
    </Properties>
    <ClientMethods>
        <chatMessage>
            <Arg>    STRING    </Arg>
        </chatMessage>
    </ClientMethods>
    <CellMethods>  </CellMethods>
    <BaseMethods>
        <logOff>
            <Exposed/>
        </logOff>
    </BaseMethods>
</root>
```

两者格式几乎相同——都源自 BigWorld 的设计。但 BigWorld 多了 `<Volatile>` 块（位置/朝向的实时属性）和 `<Exposed>` 标记（客户端可调用方法的安全边界）。

## 5.3 三合一：一个 .def 文件驱动三层

```
.def 文件
  │
  ├──→ 脚本层：生成 Python 类的属性描述和方法签名
  │     Python 脚本可以 getattr/setattr 这些属性
  │     Python 脚本可以调用这些方法
  │
  ├──→ 网络层：生成协议字段 ID（utype）和消息编解码规则
  │     属性同步时按 utype 序列化
  │     RPC 调用时按 MethodDescription 编码参数
  │
  └──→ 持久化层：标记哪些属性需要写库（Persistent=true）
        决定数据库表结构（哪些列）
        决定写库/恢复流中属性的排列顺序
```

**改一个 .def 文件的影响链**：

- 加一个属性 → Python 类型多一个字段 → 网络协议多一个字段 ID → 如果 `Persistent=true`，持久化映射也要跟着变化
- 改一个属性的类型 → 网络编解码方式变 → 数据库存储类型和恢复逻辑可能都要调整 → 已有数据可能不兼容
- 加一个方法 → 网络消息多一个 ID → RPC 调用链多一条路径

这就是为什么 EntityDef 是整个系统的骨架——它不是"配置"，而是**运行时的元模型**。

## 5.4 属性的四层身份

理解属性不能只看名字。一个属性在系统里有四层身份：

### 身份一：Python 名

脚本里直接 `entity.playerName` 访问。这是开发者视角。

### 身份二：所属侧（Flags）

Flags 决定这个属性在哪一侧存在、谁能看到：

| Flag | 含义 |
|------|------|
| `BASE` | 只在 BaseApp 存在 |
| `CELL_PRIVATE` | 只在 CellApp 本实体内部 |
| `CELL_PUBLIC` | CellApp 内，可同步到 Ghost |
| `OWN_CLIENT` | 只同步给本实体的客户端 |
| `BASE_AND_CLIENT` | Base 有，自己的客户端也看到 |
| `ALL_CLIENTS` | 所有能看到这个实体的客户端都看到 |
| `OTHER_CLIENTS` | 其他客户端看到，自己的不看到 |

**一个属性可以同时属于多个侧**。比如 `ALL_CLIENTS` 意味着 Cell 侧存储 + 所有客户端可见。

### 身份三：协议字段 ID（utype / aliasID）

网络传输时不用属性名，用数字 ID：

```cpp
// 文件：kbe/src/lib/entitydef/property.h（简化）
class PropertyDescription
{
    uint16 utype_;       // 定义系统分配的数值 ID
    uint8  aliasID_;     // 局部别名 ID（优化用）
    // ...
};
```

KBEngine 的 alias 优化不是简单的“属性数 < 255 就行”。它还受 `entitydefAliasID` 开关、保留 alias 区间以及客户端可见属性/方法数量的约束。满足条件时才会用 1 字节 `aliasID` 代替 `utype`。这在 Ch12 属性同步里会详细讲。

### 身份四：是否持久化（Persistent）

```cpp
// 文件：kbe/src/lib/entitydef/property.h
bool isPersistent_;   // 是否写库
```

`Persistent=true` 的属性：
- 写库时包含在 `addPersistentsDataToStream()` 的输出里
- 恢复时从 `createDictDataFromPersistentStream()` 重建
- 会进入数据库映射；如果表已存在，通常还需要显式迁移而不是只改 `.def`

`Persistent=false` 的属性：
- 纯运行时状态（如当前血量百分比、临时 buff 列表）
- 不写库，重启后丢失

## 5.5 ScriptDefModule / EntityDescription：从 XML 到运行时对象

### KBEngine ScriptDefModule

```cpp
// 文件：kbe/src/lib/entitydef/scriptdef_module.h（简化）
class ScriptDefModule : public RefCountable
{
    // 属性描述：按域分
    PROPERTYDESCRIPTION_MAP cellPropertyDescr_;      // Cell 侧属性
    PROPERTYDESCRIPTION_MAP basePropertyDescr_;      // Base 侧属性
    PROPERTYDESCRIPTION_MAP clientPropertyDescr_;    // Client 侧属性
    PROPERTYDESCRIPTION_MAP persistentPropertyDescr_; // 持久化属性

    // 方法描述：按域分
    METHODDESCRIPTION_MAP methodCellDescr_;          // Cell 方法
    METHODDESCRIPTION_MAP methodBaseDescr_;          // Base 方法
    METHODDESCRIPTION_MAP methodClientDescr_;        // Client 方法

    // 组件系统
    COMPONENTDESCRIPTION_MAP componentDescr_;        // 组件描述

    // 标志
    bool hasCell_, hasBase_, hasClient_;

    // 核心方法
    PyObject* createObject();       // 创建 Python 对象
    PyObject* getInitDict();        // 获取默认值字典
};
```

### BigWorld EntityDescription

```cpp
// 文件：programming/bigworld/lib/entitydef/entity_description.hpp（简化）
class EntityDescription : public BaseUserDataObjectDescription
{
    // 三组方法描述
    EntityMethodDescriptions cell_;    // Cell 方法
    EntityMethodDescriptions base_;    // Base 方法
    EntityMethodDescriptions client_;  // Client 方法

    // 属性（通过基类管理）
    // DataDescription 列表，按 DataFlags 过滤

    // 标志
    bool canBeOnCell_, canBeOnBase_, canBeOnClient_;
    bool hasComponents_, isPersistent_;

    // LOD 级别
    DataLoDLevels lodLevels_;

    // Volatile 信息
    VolatileInfo volatileInfo_;

    // 压缩
    internalNetworkCompressionType_, externalNetworkCompressionType_;

    // 流内容类型（不同场景用不同的属性子集）
    StreamContentType  // BACKUP / UNLOAD / CREATE / LOD 等
};
```

### 加载流程

KBEngine 的 `EntityDef::initialize()` 做的事：

```cpp
// 文件：kbe/src/lib/entitydef/entitydef.cpp（简化流程）
EntityDef::initialize()
{
    loadAllEntityScriptModules();   // 解析 entities.xml → 创建 ScriptDefModule 列表
    loadAllDefDescriptions();       // 解析 .def XML → 填充属性/方法描述
    {
        loadDefPropertys();         // Properties → PropertyDescription
        loadDefCellMethods();       // CellMethods → MethodDescription
        loadDefBaseMethods();       // BaseMethods → MethodDescription
        loadDefClientMethods();     // ClientMethods → MethodDescription
    }
    // 为每个属性分配 utype 和 aliasID
    // 校验持久化属性
    // 构建数据库表结构
}
```

BigWorld 的加载由 `EntityDescriptionMap` 完成，流程类似但多了 `EntityDistribution` 标签计算（自动推导哪些属性属于哪个域）。

## 5.6 DataType 系统：属性的"类型"

### KBEngine

```cpp
// 文件：kbe/src/lib/entitydef/datatype.h（简化）
class DataType : public RefCountable
{
    virtual bool isSameType(PyObject* pyValue) = 0;
    virtual void addToStream(MemoryStream& mstream, PyObject* pyValue) = 0;
    virtual PyObject* createFromStream(MemoryStream& mstream) = 0;
    virtual PyObject* parseDefaultStr(const char* str) = 0;
};
```

内置类型：

| 类型 | 类 | 用途 |
|------|-----|------|
| INT8/16/32/64, UINT* | `IntType<T>` | 整数 |
| FLOAT, DOUBLE | `FloatType` | 浮点 |
| STRING, UNICODE | `StringType` / `UnicodeType` | 字符串 |
| VECTOR2/3/4 | `Vector*Type` | 向量（位置/方向） |
| PYTHON, PY_DICT/TUPLE/LIST | `PythonType` 等 | 任意 Python 对象 |
| FIXED_ARRAY, FIXED_DICT | `FixedArrayType` / `FixedDictType` | 固定结构 |
| ENTITYCALL | `EntityCallType` | 实体引用（可序列化到网络） |

### BigWorld

```cpp
// 文件：programming/bigworld/lib/entitydef/data_type.hpp（简化）
class DataType : public ReferenceCount
{
    virtual bool isSameType(PyObject* pyValue) = 0;
    virtual void addToStream(BinaryOStream& stream, PyObject* pyValue) = 0;
    virtual PyObject* createFromStream(BinaryIStream& stream) = 0;
    // BigWorld 独有：
    virtual void addToSection(DataSection* pSection, PyObject* pyValue) = 0;
    virtual PyObject* createFromSection(DataSection* pSection) = 0;
    virtual StreamElement* getStreamElement(size_t index) = 0;
};
```

BigWorld 多了 `DataSection` 接口（XML 格式读写）和 `StreamElement` 迭代器（流式遍历）。此外 BigWorld 用 `MetaDataType` 工厂模式注册类型：

```cpp
// 文件：programming/bigworld/lib/entitydef/meta_data_type.hpp（简化）
class MetaDataType
{
    static void addMetaType(MetaDataType* pMetaType, const char* name);
    static DataType* find(const char* name);
    virtual DataType* getType(DataTypeList& args) = 0;
};
```

## 5.7 MethodDescription：方法的元数据

### KBEngine

```cpp
// 文件：kbe/src/lib/entitydef/method.h（简化）
class MethodDescription
{
    uint16 utype_;               // 网络传输 ID
    uint8 aliasID_;              // 别名 ID
    int methodDomain_;           // cell/base/client
    std::vector<DataType*> argTypes_; // 参数类型列表

    // 暴露类型
    enum EXPOSED_TYPE
    {
        NO_EXPOSED,                 // 内部调用
        EXPOSED,                    // 客户端可调用
        EXPOSED_AND_CALLER_CHECK    // 客户端可调用 + 校验调用者
    };
    EXPOSED_TYPE exposedType_;

    // 核心方法
    void addToStream(MemoryStream& mstream, PyObject* args);
    PyObject* createFromStream(MemoryStream& mstream);
};
```

### BigWorld

```cpp
// 文件：programming/bigworld/lib/entitydef/method_description.hpp（简化）
class MethodDescription : public MemberDescription
{
    Component component_;        // CLIENT/CELL/BASE
    MethodArgs args_;            // 参数列表
    MethodArgs returnValues_;    // 返回值列表（KBEngine 没有！）
    bool hasReturnValues_;
    int exposedSubMsgID_;        // 扩展消息 ID
    int priority_;               // 消息优先级
    ReplayExposureLevel replayExposureLevel_; // 录制暴露级别

    // 暴露标志
    enum
    {
        IS_EXPOSED_TO_ALL_CLIENTS,
        IS_EXPOSED_TO_OWN_CLIENT
    };
};
```

**关键差异**：BigWorld 方法支持返回值（`returnValues_`），这是 TwoWay RPC 的基础。KBEngine 的方法只有参数，没有返回值——因为 EntityCall 是纯单向的。

## 5.8 EntityApp::createEntity：定义世界 → 运行世界

`ScriptDefModule` / `EntityDescription` 是“定义”，`Entity` 是“运行”。真正跨越这个边界的核心入口，是 `EntityApp::createEntity(...)` 及其内部调用的 `onCreateEntity(...)` / `initializeEntity(...)`。

```cpp
// 文件：kbe/src/lib/server/entity_app.h（简化）
template<class E>
class EntityApp : public ServerApp
{
    // 创建实体
    virtual Entity* onCreateEntity(PyObject* pyEntity, ScriptDefModule* sm, ENTITY_ID eid);

    // 脚本接口
    static PyObject* __py_createEntity(PyObject* self, PyObject* args);
};
```

创建流程：

```
脚本调用 `KBEngine.createEntity("Avatar", params)`
  │
  ├── `EntityDef::findScriptModule("Avatar")`
  │     找到对应 `ScriptDefModule`
  │
  ├── ScriptDefModule::createObject()     ← 分配 Python 对象
  │     构造 PyObject，设置属性默认值
  │
  ├── 分配 ENTITY_ID                      ← 从 idClient 获取
  │
  ├── 构造 C++ Entity 对象                 ← 绑定 Python 对象
  │     Entity(id, pScriptModule, pyType)
  │
  ├── initializeEntity()                  ← 定义驱动的数据装配
  │     从 ScriptDefModule 读取属性描述
  │     从 initDict 填充默认值
  │
  ├── 加入实体容器                         ← entities_.add(pEntity)
  │
  └── 视创建路径决定是否立即做脚本初始化
        常见路径会回调脚本 `onInit(isReload=false)`
```

**实体不是"先有脚本对象再外挂 C++ 句柄"，而是统一构造**——Python 对象和 C++ Entity 在同一个构造流程里绑定。

## 5.9 组件系统：实体定义是对象树，不是平面表

### KBEngine Component

```cpp
// 文件：kbe/src/lib/entitydef/scriptdef_module.h（简化）
class ScriptDefModule : public RefCountable
{
    // 组件描述 map
    COMPONENTDESCRIPTION_MAP componentDescr_;
    COMPONENTDESCRIPTION_UID_MAP componentDescr_uidmap_;

    // 组件描述
    class ComponentDescription
    {
        ScriptDefModule* pScriptModule_;  // 组件自身的定义模块
        // 组件的属性映射到宿主实体的属性空间
    };
};
```

一个实体的 .def 可以引用其他 ScriptDefModule 作为组件：

```xml
<root>
    <Components>
        <CombatComponent>
            <Type> Combat </Type>
            <Persistent> true </Persistent>
        </CombatComponent>
    </Components>
    <!-- Combat 组件的属性自动合并到宿主实体的属性空间 -->
</root>
```

组件系统的效果：**实体定义是对象树，不是平面表**。一个 Avatar 实体可能包含 Combat、Inventory、Quest 等组件，每个组件有自己的属性和方法。

## 5.10 持久化与定义系统的衔接

`Persistent=true` 的属性从定义系统流向持久化系统：

```
ScriptDefModule::persistentPropertyDescr_    ← 定义系统标记
        │
        ▼
EntityTable::addPersistentsDataToStream()   ← 序列化到写库流
        │
        ▼
DBMgr → DBTaskWriteEntity → MySQL/Redis     ← 实际写库
        │
        ▼
EntityTable::createDictDataFromPersistentStream()  ← 从库恢复
```

这就是为什么“改 `.def` 文件 → 影响持久化映射、写库流、恢复流、客户端同步”。如果系统已经在线运行，还要额外考虑表迁移和历史数据兼容。**定义系统是三合一的**。

## 5.11 关键源码入口

### KBEngine

| 概念 | 文件 | 关键类/方法 |
|------|------|------------|
| 全局管理 | `kbe/src/lib/entitydef/entitydef.h` | `EntityDef::initialize()` |
| 实体描述 | `kbe/src/lib/entitydef/scriptdef_module.h` | `ScriptDefModule` |
| 属性描述 | `kbe/src/lib/entitydef/property.h` | `PropertyDescription` |
| 方法描述 | `kbe/src/lib/entitydef/method.h` | `MethodDescription` |
| 类型系统 | `kbe/src/lib/entitydef/datatype.h` | `DataType` |
| 类型注册 | `kbe/src/lib/entitydef/datatypes.h` | `DataTypes` |
| 创建实体 | `kbe/src/lib/server/entity_app.h` | `createEntity()` / `__py_createEntity` |
| 组件 | `kbe/src/lib/entitydef/entity_component.h` | `EntityComponent` |

### BigWorld

| 概念 | 文件 | 关键类/方法 |
|------|------|------------|
| 全局管理 | `lib/entitydef/entity_description_map.hpp` | `EntityDescriptionMap` |
| 实体描述 | `lib/entitydef/entity_description.hpp` | `EntityDescription` |
| 属性描述 | `lib/entitydef/data_description.hpp` | `DataDescription` |
| 方法描述 | `lib/entitydef/method_description.hpp` | `MethodDescription` |
| 类型系统 | `lib/entitydef/data_type.hpp` | `DataType` / `MetaDataType` |
| 成员基类 | `lib/entitydef/member_description.hpp` | `MemberDescription` |

## 5.12 源码走读路径

### 路径一：跟踪 .def 加载全链

1. KBEngine: `kbe/src/lib/entitydef/entitydef.cpp` — `initialize()` → `loadAllDefDescriptions()`
2. KBEngine: `kbe/src/lib/entitydef/scriptdef_module.h` — 看属性/方法描述的数据结构
3. BigWorld: `lib/entitydef/entity_description_map.hpp` — 对比加载方式

### 路径二：理解属性的四层身份

1. `kbe/src/lib/entitydef/property.h` — 看 `utype_`, `aliasID_`, `flags_`, `isPersistent_`
2. `kbe/res/sdk_templates/.../Account.def` — 对照 XML 中的 `<Flags>` 和 `<Persistent>`
3. BigWorld: `lib/entitydef/data_description.hpp` — 对比 `dataFlags_` 和 `DATA_GHOSTED`

### 路径三：理解方法与 Exposed

1. `kbe/src/lib/entitydef/method.h` — 看 `EXPOSED_TYPE` 枚举
2. BigWorld: `lib/entitydef/method_description.hpp` — 看 `IS_EXPOSED_TO_ALL_CLIENTS` + `returnValues_`
3. 对比：KBEngine 无返回值 vs BigWorld 有返回值

### 路径四：理解实体创建

1. `kbe/src/lib/server/entity_app.h` — `__py_createEntity`
2. `kbe/src/lib/entitydef/scriptdef_module.h` — `createObject()` + `getInitDict()`

## 5.13 小结

- `.def` 不是配置文件，而是**脚本描述 + 网络协议 + 持久化描述的三合一运行时骨架**
- 属性有四层身份：Python 名、所属侧（Flags）、协议字段 ID（utype/aliasID）、是否持久化
- `ScriptDefModule` / `EntityDescription` 把 XML 变成运行时元模型
- `createEntity` 是从"定义世界"跨越到"运行世界"的关键函数
- 组件系统让实体定义变成对象树
- BigWorld 的方法描述比 KBEngine 更丰富：支持返回值（TwoWay RPC 基础）、优先级、Ghost 同步标志
- 改一个 .def 文件同时影响 Python 类型、网络协议、数据库表结构
