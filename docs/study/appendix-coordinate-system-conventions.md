# 附录 F 坐标系约定：BigWorld 与 KBEngine

> **核心问题**：服务端的 X/Y/Z 各代表什么？方向怎么编码？坐标在网络传输中如何压缩？不同客户端引擎接入时需要注意什么？

本附录讲解的是**坐标系约定层**——轴方向、旋转表示、单位、压缩编码、客户端兼容性。十字链表的空间索引算法（CoordinateSystem / RangeList 的数据结构和插入/冒泡/删除操作）在 [Ch14](14-space-aoi-and-vision-system.md) 详解，不在此重复。

## F.1 轴约定

### KBEngine：Y-up

```
        Y (高度/上)
        │
        │
        │
        └────────── X (东/右)
       ╱
      ╱
     Z (南/前)
```

| 轴 | 含义 | 典型用途 |
|----|------|---------|
| X | 水平（左右） | 地平面东向 |
| Y | 垂直（上下） | 高度/海拔 |
| Z | 水平（前后） | 地平面前向 |

**证据**：

- `Position3D` 就是 `Vector3`（`kbe/src/lib/math/math.h:171`），x/y/z 即为坐标三分量
- AOI 十字链表默认只维护 X/Z 两条链表，Y 链表可选（`CoordinateSystem::hasY`，默认 `false`）
- 速度属性 `topSpeed` 表示"XZ 轴最大移动速度（米/秒）"，`topSpeedY` 表示"Y 轴最大移动速度（米/秒）"（`kbe/src/server/cellapp/entity.h:648-652`）
- 2D 距离计算忽略 Y 轴（`KBEVec3CalcVec2Length`，`math.h:144-149`）

```cpp
// kbe/src/lib/math/math.h:144
// 从2个3d向量忽略y计算出2d长度
inline float KBEVec3CalcVec2Length(const Vector3& v1, const Vector3& v2)
{
    float x = v1.x - v2.x;
    float z = v1.z - v2.z;
    return sqrt(x*x + z*z);
}
```

### BigWorld：同为 Y-up（仅 AOI 链表为 X/Z 两条）

BigWorld 的 `RangeListNode` 只维护 X/Z 两条链表指针（`pPrevX_/pNextX_/pPrevZ_/pNextZ_`），不管理 Y 轴。这与 KBEngine 的默认行为一致——大多数地牢/MMO 游戏中，实体在同一地平面上移动，Y 轴变化少，维护 Y 链表的代价不划算。

**注意**：BigWorld 的 `RangeListNode` 穿越回调同时传入 `oldOthX` 和 `oldOthZ`（但不传 Y），进一步说明 AOI 层只关心 XZ 平面：

```cpp
// BigWorld RangeListNode 穿越回调
virtual void crossedX(RangeListNode* node, bool positiveCrossing,
    float oldOthX, float oldOthZ) {}
virtual void crossedZ(RangeListNode* node, bool positiveCrossing,
    float oldOthX, float oldOthZ) {}
```

### 对照

| 维度 | KBEngine | BigWorld |
|------|----------|----------|
| 垂直轴 | Y | Y |
| 地平面 | XZ | XZ |
| AOI 链表 | X/Z（可选 Y） | X/Z（无 Y） |
| Y 轴管理 | `hasY` 开关，默认关 | 无 Y 轴链表 |
| 2D 距离 | 忽略 Y 的 `KBEVec3CalcVec2Length` | 类似 |

## F.2 Y 轴开关的设计取舍

KBEngine 提供了一个全局开关 `CoordinateSystem::hasY`，通过 `kbengine_defaults.xml` 配置：

```xml
<!-- kbengine_defaults.xml → cellapp → coordinate_system -->
<rangemgr_y> false </rangemgr_y>
```

### 为什么默认关闭

1. **大多数 MMO 地面平坦**：玩家和怪物在同一 Y 高度活动，AOI 事件中 Y 轴几乎不变
2. **减少链表操作**：关闭 Y 链表意味着每次实体移动只需冒泡 X 和 Z 两条链表，而非三条
3. **RangeTrigger 更简单**：进出判定只需检查 X 和 Z，跳过 Y 轴的 `isInYRange` 判断

### 什么时候打开

- **太空/飞行游戏**：实体在不同高度活动，Y 轴差异显著
- **小房间少实体**：Y 轴链表的性能开销在小规模场景中可忽略
- **需要 3D AOI**：精确的三维视野控制（如水下/空中分层）

### 开启后的代价

```cpp
// range_trigger_node.cpp:78
bool RangeTriggerNode::wasInYRange(CoordinateNode * pNode)
{
    if (!CoordinateSystem::hasY)
        return true;  // Y 关闭时，Y 范围判定永远返回 true
    ...
}
```

每对节点的进出判定从 2 轴检查变成 3 轴检查——X 通过了还要检查 Z，Z 通过了还要检查 Y。在实体密集的场景中（数千实体同一高度），这会额外增加进出事件数量。

## F.3 旋转表示：Direction3D

### 数据结构

```cpp
// kbe/src/lib/math/math.h:174-197
struct Direction3D
{
    Direction3D():dir(0.f, 0.f, 0.f) {};
    Direction3D(const Vector3 & v):dir(v){}
    Direction3D(float r, float p, float y):dir(r, p, y){}

    float roll()  const{ return dir.x; }
    float pitch() const{ return dir.y; }
    float yaw()   const{ return dir.z; }

    void roll(float v) { dir.x = v; }
    void pitch(float v){ dir.y = v; }
    void yaw(float v)  { dir.z = v; }

    Vector3 dir;  // 存储 (roll, pitch, yaw)
};
```

**关键设计决策**：

| 维度 | 决策 | 说明 |
|------|------|------|
| 编码 | Euler 角（roll, pitch, yaw） | 非四元数，非矩阵 |
| 单位 | **弧度（radians）** | 不是角度 |
| 存储布局 | `dir.x = roll, dir.y = pitch, dir.z = yaw` | 注意：不是 `(yaw, pitch, roll)` |
| 脚本接口 | `(roll, pitch, yaw)` 三元组 | Python 端 `self.direction = (r, p, y)` |

### Yaw 的含义

在 Y-up 坐标系中：

- **yaw**：绕 Y 轴旋转（水平转向）——最常用的旋转
- **pitch**：绕 X 轴旋转（抬头/低头）
- **roll**：绕 Z 轴旋转（侧翻）

大多数 MMO 只用 yaw（角色水平转向），pitch 和 roll 默认为 0。这也是为什么同步协议有 `UPDATE_FLAG_YAW` 单独的 flag。

### 网络传输的角度压缩

角度在传输时用 `angle2int8()` 压缩为 1 字节：

```cpp
// kbe/src/lib/math/math.h:156-169
inline KBEngine::int8 angle2int8(float v, bool half = false)
{
    KBEngine::int8 angle = 0;
    if(!half)
    {
        // 普通模式：角度 → int8，覆盖 -π ~ π
        angle = (KBEngine::int8)floorf((v * 128.f) / float(KBE_PI) + 0.5f);
    }
    else
    {
        // 半范围模式：角度 → int8，覆盖 -π/2 ~ π/2
        angle = (KBEngine::int8)KBEClamp(floorf((v * 254.f) / float(KBE_PI) + 0.5f), -128.f, 127.f);
    }
    return angle;
}
```

| 参数 | 压缩后大小 | 覆盖范围 | 精度 |
|------|-----------|---------|------|
| 普通 yaw/pitch/roll | 1 字节 (int8) | -π ~ π（-180° ~ 180°） | ~1.4° |
| 半范围模式 | 1 字节 (int8) | -π/2 ~ π/2（-90° ~ 90°） | ~0.7° |

**反向解码**：

```cpp
inline float int82angle(KBEngine::int8 angle, bool half = false)
{
    return float(angle) * float((KBE_PI / (half ? 254.f : 128.f)));
}
```

1.4° 的精度对 MMO 角色转向足够——客户端收到后做插值平滑即可。

## F.4 坐标压缩

位置同步是 MMO 带宽消耗的大头。KBEngine 提供三级压缩，从粗到细：

### PackXZ：地平面位移压缩（3 字节）

将 X/Z 两个 float（共 8 字节）压缩为 3 字节，节省 62.5%。

```cpp
// kbe/src/lib/common/memorystream.h:648-700
void appendPackXZ(float x, float z)
{
    // 利用 IEEE 754 浮点数结构：
    // 0-7位存放尾数, 8-10位存放指数, 11位存放标志
    // 舍弃第一位使范围达到 (-512~-2), (2~512)
    ...
}
```

| 参数 | 大小 | 覆盖范围 | 精度 |
|------|------|---------|------|
| X | 11.5 bit | -512 ~ +512 | ~0.25 单位 |
| Z | 11.5 bit | -512 ~ +512 | ~0.25 单位 |

**注意**：PackXZ 编码的是**相对坐标**（`otherEntity->position() - this->pEntity()->position()`），不是绝对坐标。相对距离通常在视野范围内（几十到几百米），512 单位的范围足够。

### PackY：高度位移压缩（2 字节）

将 Y 一个 float（4 字节）压缩为 2 字节，节省 50%。

```cpp
// kbe/src/lib/common/memorystream.h:703-713
void appendPackY(float y)
{
    PackFloatXType yPackData;
    yPackData.fv = y;
    yPackData.fv += yPackData.iv < 0 ? -2.f : 2.f;
    uint16 data = 0;
    data = (yPackData.uv >> 12) & 0x7fff;
    data |= ((yPackData.uv >> 16) & 0x8000);
    (*this) << data;
}
```

| 参数 | 大小 | 覆盖范围 | 精度 |
|------|------|---------|------|
| Y | 15 bit + 1 符号位 | -512 ~ +512 | 较低 |

### PackXYZ：三轴一体压缩（4 字节）

将 X/Y/Z 三个 float（12 字节）压缩为 4 字节，节省 66.7%。

```cpp
// kbe/src/lib/common/memorystream.h:633-646
void appendPackXYZ(float x, float y, float z, float minf = -256.f)
{
    x -= minf;
    y -= minf / 2.f;
    z -= minf;
    // X: 11 bit, Z: 11 bit, Y: 10 bit, 总共 32 bit
    uint32 packed = 0;
    packed |= ((int)(x / 0.25f) & 0x7FF);        // X: 11 bit
    packed |= ((int)(z / 0.25f) & 0x7FF) << 11;   // Z: 11 bit
    packed |= ((int)(y / 0.25f) & 0x3FF) << 22;   // Y: 10 bit
    *this << packed;
}
```

| 参数 | 分配 | 覆盖范围（minf = -256） | 精度 |
|------|------|------------------------|------|
| X | 11 bit | -256 ~ +256 | 0.25 单位 |
| Z | 11 bit | -256 ~ +256 | 0.25 单位 |
| Y | 10 bit | -128 ~ +128 | 0.25 单位 |

**为什么 Y 只有 10 bit**：高度变化范围通常比水平小，128 单位（米）足以覆盖大多数场景的高差。

### 压缩方案总结

| 编码方法 | 原始大小 | 压缩后 | 节省 | 使用场景 |
|---------|---------|--------|------|---------|
| PackXZ | 8B (2×float) | 3B | 62.5% | 仅水平移动（最常见） |
| PackXZ + PackY | 12B (3×float) | 5B | 58.3% | 水平+垂直移动 |
| PackXYZ | 12B (3×float) | 4B | 66.7% | 紧凑三轴（相对坐标） |
| angle2int8 | 4B (float) | 1B | 75.0% | 单个旋转角 |

## F.5 同步协议中的坐标传输

Witness 在每个 tick 收集可见实体的位置/方向变更，根据变更类型选择不同的同步消息。

### 更新标志位

```cpp
// kbe/src/server/cellapp/witness.cpp:20-30
#define UPDATE_FLAG_NULL              0x00000000
#define UPDATE_FLAG_XZ                0x00000001  // 仅 XZ 位移
#define UPDATE_FLAG_XYZ               0x00000002  // 三轴位移
#define UPDATE_FLAG_YAW               0x00000004  // 仅 yaw
#define UPDATE_FLAG_ROLL              0x00000008  // 仅 roll
#define UPDATE_FLAG_PITCH             0x00000010  // 仅 pitch
#define UPDATE_FLAG_YAW_PITCH_ROLL    0x00000020  // 三轴旋转
#define UPDATE_FLAG_YAW_PITCH         0x00000040  // yaw + pitch
#define UPDATE_FLAG_YAW_ROLL          0x00000080  // yaw + roll
#define UPDATE_FLAG_PITCH_ROLL        0x00000100  // pitch + roll
#define UPDATE_FLAG_ONGOUND           0x00000200  // 在地面上
```

### 组合示例

以最常见的"水平移动 + 转向"为例：

```cpp
// witness.cpp:1074 — UPDATE_FLAG_XZ | UPDATE_FLAG_YAW
case (UPDATE_FLAG_XZ | UPDATE_FLAG_YAW):
{
    Position3D relativePos = otherEntity->position() - this->pEntity()->position();
    // 发送：实体ID(可变长) + PackXZ(3B) + angle2int8(1B) = ~5-6 字节
    pForwardBundle->appendPackXZ(relativePos.x, relativePos.z);
    (*pForwardBundle) << angle2int8(dir.yaw());
}
```

**带宽计算**：一个 tick 内，一个观察者视野内有 30 个可见实体，其中 10 个在移动。每条 `XZ + YAW` 消息约 6 字节，10 × 6 = 60 字节/tick。按 10Hz tick rate 计算，每秒仅 600 字节的位移同步流量——非常高效。

### 优化标志 optimized

```cpp
// api/cellapp/Entity.md:1140
// 还有一个特殊的bool属性optimized，它的作用是控制服务器同步时是否进行优化，
// 目前主要的优化是Y轴。
// 如果为true，在一些行为(如：navigate)导致服务器能确定实体在地面时，
// 服务器不同步实体的Y轴坐标，当同步大量实体时能节省大量带宽，默认为true。
```

当 `optimized = true` 时，服务器在确定实体贴地时会跳过 Y 轴同步，仅发 `UPDATE_FLAG_XZ` 而非 `UPDATE_FLAG_XYZ`——每实体每 tick 省 2 字节。

## F.6 单位约定

| 量 | 单位 | 说明 |
|----|------|------|
| 位置 | 米（meter） | 1 单位 = 1 米。API 文档明确 `topSpeed` 单位为"米/秒" |
| 旋转 | 弧度（radian） | `direction` 属性内部存储弧度，范围 -π ~ π |
| 速度 | 米/秒（m/s） | `topSpeed`、`topSpeedY` 的单位 |
| 距离（AOI） | 米 | `ViewRadius`、`range_xz` 等参数均以米为单位 |
| 时间 | 秒（second） | 定时器参数、移动速度计算均基于秒 |

### 速度校验

服务端用 `topSpeed` / `topSpeedY` 校验客户端上报的移动合法性：

```cpp
// entity.h:648-652
float topSpeed_;   // entity x,z轴最高移动速度（米/秒）
float topSpeedY_;  // entity y轴最高移动速度（米/秒）

// entity.cpp — checkMoveForTopSpeed
// 如果移动距离超出速度限制，强制拉回上一个坐标位置
```

`topSpeed` 通常设置得比实际移动速度大一些，留出网络延迟的容差空间。

## F.7 数学库选择

KBEngine 的数学层有两个后端，编译期选择：

```cpp
// kbe/src/lib/math/math.h
#if KBE_PLATFORM == PLATFORM_WIN32 && defined(USE_D3DX)
    // DirectX 数学库（D3DX）
    typedef D3DXVECTOR3 Vector3;
    typedef D3DXQUATERNION Quaternion;
    #define KBEMatrixRotationYawPitchRoll D3DXMatrixRotationYawPitchRoll
#else
    // G3D 数学库（跨平台）
    typedef G3D::Vector3 Vector3;
    typedef G3D::Quat Quaternion;
#endif
```

| 后端 | 平台 | 坐标系 |
|------|------|--------|
| D3DX (DirectX) | Windows | 左手系 Y-up |
| G3D | 跨平台 | 右手系 Y-up |

**对服务端的影响**：服务端几乎不使用旋转矩阵（主要用 Euler 角做同步），左右手系差异在服务端不体现。差异仅在以下场景需要注意：

- 导航网格（NavMesh）生成时的坐标系假设
- 跨进程通信中的旋转矩阵序列化（目前不涉及）

**实际选择**：默认编译使用 G3D 后端（跨平台），`USE_D3DX` 通常不启用。

## F.8 客户端接入的坐标变换

KBEngine 是纯服务端引擎，客户端可以选择任意 3D 引擎。不同客户端引擎的坐标系不同，接入时需要做坐标变换。

### 常见客户端引擎坐标约定

| 客户端引擎 | 垂直轴 | 前向轴 | 手性 |
|-----------|--------|--------|------|
| Unity | Y-up | Z-forward | 左手系 |
| Unreal | Z-up | X-forward | 左手系 |
| Godot | Y-up | -Z-forward | 右手系 |
| OGRE | Y-up | -Z-forward | 右手系 |
| Cocos2d-x | Y-up | (2.5D) | 左手系 |

### 变换要点

**KBEngine → Unity**（最常见组合）：

```
KBEngine (Y-up)          Unity (Y-up)
  X  →  X                无需变换
  Y  →  Y                无需变换
  Z  →  Z                无需变换

方向（弧度 → 弧度）       无需变换
```

**KBEngine → Unreal**：

```
KBEngine (Y-up)          Unreal (Z-up)
  X  →  X
  Y  →  Z                Y 和 Z 互换
  Z  →  Y

yaw (绕 Y → 绕 Z)        需要重新映射
```

**KBEngine → Godot**：

```
KBEngine (Y-up)          Godot (Y-up, 右手系)
  X  →  X
  Y  →  Y
  Z  →  -Z               Z 轴取反

yaw 绕 Y                 注意旋转方向
```

### SDK 层处理

KBEngine 官方提供的客户端 SDK（Unity 插件等）已经在 SDK 层封装了坐标变换，开发者通常不需要手动处理。但如果自行实现客户端协议解析，需要注意：

1. **解包顺序**：`readPackXZ` 解出的是 `(x, z)`，对应 KBEngine 的 X 轴和 Z 轴
2. **角度解码**：`int82angle` 返回弧度值，客户端显示时可能需要转为角度
3. **相对坐标**：收到的坐标是相对观察者的位移，需加上观察者自身位置才能得到绝对坐标

## F.9 BigWorld 的坐标系差异

BigWorld 与 KBEngine 在轴约定上基本一致（Y-up，XZ 地平面），但存在实现差异：

| 维度 | KBEngine | BigWorld |
|------|----------|----------|
| AOI 链表维度 | X/Z（可选 Y） | X/Z（无 Y 选项） |
| Y 轴管理 | `hasY` 全局开关 | 无 |
| 穿越回调参数 | 单轴触发 | `crossedX/Z` 同时传入 `oldOthX` 和 `oldOthZ` |
| 排序稳定 | 隐式（节点标志位） | 显式 `RangeListOrder` 枚举 |

BigWorld 的 `crossedX` 回调同时提供 `oldOthX` 和 `oldOthZ` 是为了让触发器能在单次回调中做完整的 2D 判定（X 变了，Z 是否也在范围内），减少不必要的二次查找。

## F.10 关键源码入口

| 概念 | 文件 | 关键位置 |
|------|------|---------|
| Position3D / Direction3D | `kbe/src/lib/math/math.h` | L171-197 |
| Vector3（G3D 后端） | `kbe/src/lib/common/G3D/Vector3.h` | — |
| Y 轴开关 | `kbe/src/server/cellapp/coordinate_system.h` | `hasY` |
| Y 轴开关初始化 | `kbe/src/server/cellapp/cellapp.cpp` | L282-283 |
| Y 轴配置 | `kbe/res/server/kbengine_defaults.xml` | `rangemgr_y` |
| PackXZ 编解码 | `kbe/src/lib/common/memorystream.h` | L448-700 |
| PackY 编解码 | `kbe/src/lib/common/memorystream.h` | L490-501 |
| PackXYZ 编解码 | `kbe/src/lib/common/memorystream.h` | L444-455, L633-646 |
| angle2int8 / int82angle | `kbe/src/lib/math/math.h` | L151-169 |
| 更新标志位 | `kbe/src/server/cellapp/witness.cpp` | L20-30 |
| 同步消息组装 | `kbe/src/server/cellapp/witness.cpp` | L968-1076 |
| topSpeed / topSpeedY | `kbe/src/server/cellapp/entity.h` | L648-652 |
| optimized 标志 | `kbe/src/server/cellapp/entity.h` | `isOnGround_` |
| 速度校验 | `kbe/src/server/cellapp/entity.cpp` | `checkMoveForTopSpeed` |
| 数学库选择 | `kbe/src/lib/math/math.h` | L17-141 |

## F.11 小结

KBEngine 和 BigWorld 的坐标系约定简洁而统一：

1. **Y-up XZ 地平面**：服务端标准约定，与主流 3D 引擎兼容
2. **Euler 角弧度制**：`(roll, pitch, yaw)` 存储在 `Vector3` 中，yaw 最常用
3. **Y 轴可选**：AOI 默认只管 XZ，减少不必要的链表操作
4. **相对坐标压缩**：PackXZ/PackY 利用视野内位移有限的特点，将每 tick 的同步消息压缩到 5-6 字节
5. **角度单字节**：1.4° 精度对 MMO 角色转向足够
6. **客户端适配**：SDK 层封装了坐标变换，但自实现时需注意轴映射和手性差异

这套约定的核心思想是**按需精度**——不是所有坐标都需要 float32 的精度，也不是所有旋转都需要四元数。在服务端同步场景下，相对位移 + 压缩角度是最经济的组合。
