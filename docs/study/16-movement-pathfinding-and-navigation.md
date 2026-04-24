# 16. 移动、寻路与导航

> 这一章回答：实体怎么从 A 点走到 B 点？为什么 MMO 选 NavMesh 而不是 Grid / Waypoint？移动控制器怎么把"目标点"变成"逐帧移动"？

## 16.1 本章核心问题

- 为什么用 NavMesh 而不用 Grid / Waypoint？
- 导航网格是怎么生成的？（Recast 流水线）
- A* 在导航多边形上的变体怎么工作？
- 移动控制器的架构是怎样的？（Controller 模式）
- BigWorld 的 10+ 种控制器各自解决什么问题？
- KBEngine 的 NavigateHandler 怎么实现逐帧移动？
- 移动和 AOI 怎么交互？
- 射线检测在服务器端做什么？

## 16.2 为什么用 NavMesh 而不用 Grid / Waypoint

### 三种寻路方案对比

| 方案 | 空间表示 | 精度 | 内存 | 适用场景 |
|------|---------|------|------|---------|
| **Grid（网格）** | 固定大小的方格 | 低（格子粒度） | 高（n×m 数组） | 2D 游戏、简单地图 |
| **Waypoint（路点）** | 离散的路径点 | 低（只能走路点） | 低 | 固定路线的 NPC |
| **NavMesh（导航网格）** | 凸多边形覆盖可行走区域 | 高（连续空间） | 中 | 3D 游戏、复杂地形 |

```
Grid:
  ┌──┬──┬──┬──┐
  │  │██│  │  │   ██ = 不可行走
  ├──┼──┼──┼──┤   每个格子要么可行走要么不可行走
  │  │  │  │██│   分辨率受格子大小限制
  └──┴──┴──┴──┘

Waypoint:
  A ──→ B ──→ C ──→ D
           │
           └──→ E    只能走路点，不能自由移动

NavMesh:
  ╱‾‾‾‾‾‾╲   ╱‾‾‾‾╲
 │  poly1  │ │poly2 │   凸多边形覆盖所有可行走区域
 │    ●    │ │  ●   │   实体可以在多边形内自由移动
  ╲______╱   ╲____╱
```

**NavMesh 的优势**：

1. **连续空间**：实体不限制在格子或路点上，可以在多边形内自由移动
2. **内存高效**：用少量多边形覆盖大面积可行走区域，比 Grid 的逐格标记省内存
3. **精度自适应**：开阔区域用大多边形，复杂地形用小多边形
4. **支持 3D**：多边形可以跟随地形起伏（楼梯、斜坡、多层建筑）

## 16.3 NavMesh 基本原理：Recast 流水线

Recast 是导航网格生成的工业标准库（MIT 协议），Detour 是配套的寻路库。

### 生成流程

```
原始几何数据（三角形网格）
  │
  ├── 1. Voxelization（体素化）
  │     将三角形网格转换为高度场（Heightfield）
  │     每个网格单元记录最高可行走面
  │
  ├── 2. Region Partitioning（区域划分）
  │     将高度场中的可行走体素分成连通区域
  │     使用 Watershed / Monotone / Layer 分区算法
  │
  ├── 3. Contour Simplification（轮廓简化）
  │     将区域的体素边界简化为多边形轮廓
  │     Douglas-Peucker 算法去除冗余顶点
  │
  ├── 4. Polygon Mesh Generation（多边形网格生成）
  │     将轮廓三角剖分，再合并为凸多边形
  │     生成 CompactHeightfield → PolyMesh
  │
  └── 5. Detail Mesh Generation（细节网格生成）
        为每个多边形添加高度细节
        生成 PolyMeshDetail（可选，用于精确查询）
```

### BigWorld 的 Recast 集成

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/lib/navigation_recast/recast_generator.hpp（简化）
class RecastGenerator
{
    struct RecastConfig
    {
        BoundingBox chunkBB_;                 // 分块边界
        float maxWalkableSlopeInAngle_;       // 最大可走坡度
        float maxAgentClimb_;                 // 最大攀爬高度
        float agentHeight_;                   // 代理高度
        float agentRadius_;                   // 代理半径
        bool scaleNavigation_;                // 导航缩放
        float gridSize_;                      // 网格大小
    };

    bool generate(const CollisionSceneProviders& provider,
        const RecastConfig& config, int gridX, int gridZ, bool useMono);

    // Recast 流水线的中间产物
    AutoPtr<rcHeightfield> heightField_;
    AutoPtr<rcCompactHeightfield> compactHeightField_;
    AutoPtr<rcPolyMesh> polyMesh_;
};
```

### KBEngine 的 NavMesh 加载

```cpp
// 文件：kbe/src/server/cellapp/loadnavmesh_threadtasks.h（简化）
class LoadNavmeshTask : public thread::TPTask
{
public:
    LoadNavmeshTask(const std::string& resPath, SPACE_ID spaceID,
        const std::map<int, std::string>& params);

    virtual bool process();              // 在线程池中加载 NavMesh
    virtual TPTaskState presentMainThread();  // 回到主线程通知

private:
    std::string resPath_;
    SPACE_ID spaceID_;
    std::map<int, std::string> params_;
};
```

KBEngine 当前源码主线是加载现成的导航资源：`LoadNavmeshTask::process()` 在线程池里调用 `Navigation::loadNavigation()`，主线程再在 `presentMainThread()` 里把结果挂回 `SpaceMemory`。也就是说，这一侧重点是**异步加载和查询**，不是在 CellApp 内现场生成 NavMesh。

### A* 在导航多边形上的变体

传统 A* 在格子上搜索，NavMesh 版本在多边形上搜索：

```
传统 Grid A*：
  格子 → 8 个邻居 → 选择 f 值最小的

NavMesh A*（Detour 实现）：
  多边形 → 共享边的邻居多边形 → 选择 f 值最小的

  f = g + h
  g = 从起点沿多边形边走的实际距离
  h = 到终点的直线距离（启发式）

  路径后处理：string pulling（绳拉法）
    多边形序列 → 拉直为最短路径 → 只保留转折点
```

## 16.4 KBEngine 的导航系统

### NavigationHandle：统一导航接口

```cpp
// 文件：kbe/src/lib/navigation/navigation_handle.h（简化）
class NavigationHandle : public RefCountable
{
public:
    enum NAV_TYPE { NAV_UNKNOWN = 0, NAV_MESH = 1, NAV_TILE = 2 };
    enum NAV_OBJECT_STATE { NAV_OBJECT_STATE_MOVING = 1, NAV_OBJECT_STATE_MOVEOVER = 2 };

    virtual NAV_TYPE type() const { return NAV_UNKNOWN; }

    // 核心导航接口
    virtual int findStraightPath(int layer, const Position3D& start,
        const Position3D& end, std::vector<Position3D>& paths) = 0;

    virtual int findRandomPointAroundCircle(int layer, const Position3D& centerPos,
        std::vector<Position3D>& points, uint32 max_points, float maxRadius) = 0;

    virtual int raycast(int layer, const Position3D& start,
        const Position3D& end, std::vector<Position3D>& hitPointVec) = 0;

    std::string resPath;
};
```

**三种导航能力**：

1. **findStraightPath**：寻路——从 start 到 end 的最短路径
2. **findRandomPointAroundCircle**：在指定半径内随机选点（用于 NPC 漫游）
3. **raycast**：射线检测——从 start 到 end 的碰撞检测

### NavMeshHandle：基于 Detour 的 3D 导航

```cpp
// 文件：kbe/src/lib/navigation/navigation_mesh_handle.h（简化）
class NavMeshHandle : public NavigationHandle
{
public:
    static const int MAX_POLYS = 256;
    static const int INVALID_NAVMESH_POLYREF = 0;

    struct NavmeshLayer
    {
        dtNavMesh* pNavmesh;           // Detour 导航网格
        dtNavMeshQuery* pNavmeshQuery; // Detour 查询对象
    };

    virtual int findStraightPath(int layer, const Position3D& start,
        const Position3D& end, std::vector<Position3D>& paths);
    virtual int raycast(int layer, const Position3D& start,
        const Position3D& end, std::vector<Position3D>& hitPointVec);
    virtual NAV_TYPE type() const { return NAV_MESH; }

    std::map<int, NavmeshLayer> navmeshLayer;   // 支持多层导航网格
};
```

**多层导航网格**：`navmeshLayer` 是 `map<int, NavmeshLayer>`，支持按 layer 查询不同的 NavMesh。典型用例：地面层和空中层使用不同的导航网格。

### NavTileHandle：基于格子的 2D 导航

```cpp
// 文件：kbe/src/lib/navigation/navigation_tile_handle.h（简化）
class NavTileHandle : public NavigationHandle
{
public:
    enum TILE_STATE
    {
        TILE_STATE_OPENED_COST0 = 0,   // 可通行，代价 0
        TILE_STATE_OPENED_COST1 = 1,   // 可通行，代价 1
        // ... 代价 2~5
        TILE_STATE_CLOSED = 9          // 不可通行
    };

    // A* 搜索节点
    class MapSearchNode
    {
    public:
        int x, y;
        float GoalDistanceEstimate(MapSearchNode& nodeGoal);
        bool IsGoal(MapSearchNode& nodeGoal);
        bool GetSuccessors(AStarSearch<MapSearchNode>* astarsearch,
            MapSearchNode* parent_node);
        float GetCost(MapSearchNode& successor);
        bool IsSameState(MapSearchNode& rhs);
    };

    virtual NAV_TYPE type() const { return NAV_TILE; }

    Tmx::Map* pTilemap;           // TMX 格式地图（Tiled Map Editor 导出）
    bool direction8_;             // 8 方向 vs 4 方向
};
```

NavTileHandle 使用 TMX 格式的地图数据，支持 4 方向和 8 方向移动，适用于 2D 游戏。

### Navigation：全局导航管理器

```cpp
// 文件：kbe/src/lib/navigation/navigation.h（简化）
class Navigation : public Singleton<Navigation>
{
public:
    NavigationHandlePtr loadNavigation(std::string resPath,
        const std::map<int, std::string>& params);
    bool hasNavigation(std::string resPath);
    bool removeNavigation(std::string resPath);
    NavigationHandlePtr findNavigation(std::string resPath);

private:
    KBEUnordered_map<std::string, NavigationHandlePtr> navhandles_;
    KBEngine::thread::ThreadMutex mutex_;
};
```

所有 NavMesh 共享缓存——同一个资源路径只加载一次。

## 16.5 移动控制器架构

### KBEngine Controller 基类

```cpp
// 文件：kbe/src/server/cellapp/controller.h（简化）
class Controller
{
public:
    enum ControllerType
    {
        CONTROLLER_TYPE_NORMAL = 0,        // 常规类型
        CONTROLLER_TYPE_PROXIMITY = 1,     // 范围触发器类型
        CONTROLLER_TYPE_MOVE = 2,          // 移动控制器类型
        CONTROLLER_TYPE_ROTATE = 3,        // 旋转控制器类型
    };

    Controller(ControllerType type, Entity* pEntity, int32 userarg, uint32 id = 0);
    virtual ~Controller();

    virtual void destroy();
    virtual void addToStream(KBEngine::MemoryStream& s);
    virtual void createFromStream(KBEngine::MemoryStream& s);

protected:
    uint32 id_;
    Entity* pEntity_;
    int32 userarg_;
    Controllers* pControllers_;
    ControllerType type_;
};
```

### MoveController：移动控制器

```cpp
// 文件：kbe/src/server/cellapp/move_controller.h（简化）
class MoveController : public Controller
{
public:
    MoveController(Entity* pEntity, MoveToPointHandler* pHandler = NULL,
        uint32 id = 0);

    float velocity() const { return pMoveToPointHandler_->velocity(); }
    void velocity(float v) { pMoveToPointHandler_->velocity(v); }

    virtual void destroy();

private:
    MoveToPointHandler* pMoveToPointHandler_;
};
```

### MoveToPointHandler：移动处理器继承链

```
MoveToPointHandler（基类：直线移动到点）
  │
  ├── NavigateHandler（沿路径移动——寻路结果）
  │     持有路径点列表 paths_
  │     到达当前路径点后切换到下一个
  │
  └── MoveToEntityHandler（追踪移动——跟随另一个实体）
        每帧更新目标位置
```

```cpp
// 文件：kbe/src/server/cellapp/navigate_handler.h（简化）
class NavigateHandler : public MoveToPointHandler
{
public:
    NavigateHandler(KBEShared_ptr<Controller>& pController,
        const Position3D& destPos, float velocity, float distance,
        bool faceMovement, float maxMoveDistance,
        VECTOR_POS3D_PTR paths_ptr, PyObject* userarg);

    virtual bool requestMoveOver(const Position3D& oldPos);
    virtual bool isOnGround() { return true; }
    virtual MoveType type() const { return MOVE_TYPE_NAV; }

protected:
    int destPosIdx_;                 // 当前路径点索引
    VECTOR_POS3D_PTR paths_;         // 完整路径点列表
    float maxMoveDistance_;           // 最大移动距离（安全限制）
};
```

### 移动的完整生命周期

```
脚本调用: entity.navigateTo((10, 0, 20))
  │
  ├── 1. 寻路
  │     NavigationHandle::findStraightPath(start, end, paths)
  │     → Detour A* → 路径点列表 paths = [(5,0,5), (8,0,12), (10,0,20)]
  │
  ├── 2. 创建控制器
  │     new NavigateHandler(controller, dest, velocity, ..., paths)
  │     new MoveController(entity, navigateHandler)
  │
  ├── 3. 注册到实体
  │     entity.pControllers()->add(pMoveController)
  │
  ├── 4. 逐帧移动（每 tick）
  │     NavigateHandler::requestMoveOver()
  │       计算本帧移动距离 = velocity * tickTime
  │       向当前路径点移动
  │       到达当前路径点 → destPosIdx_++ → 向下一个路径点移动
  │       更新实体位置 → 触发 CoordinateSystem::update()
  │
  ├── 5. 到达终点
  │     所有路径点走完 → 触发脚本回调 onNavigateComplete()
  │     销毁控制器
  │
  └── 6. 异常情况
        移动距离超过 maxMoveDistance → 中断
        实体被销毁 → 清理控制器
```

## 16.6 BigWorld 的控制器架构

BigWorld 有 10+ 种控制器，覆盖移动、朝向、感知等场景。

### Controller 基类

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/controller.hpp（简化）
class Controller : public ReferenceCount
{
public:
    virtual ControllerType type() const = 0;
    virtual ControllerDomain domain() const = 0;
    virtual ControllerID exclusiveID() const = 0;

    // 生命周期
    virtual void startReal(bool isInitialStart);
    virtual void stopReal(bool isFinalStop);
    virtual void startGhost();
    virtual void stopGhost();

    // 序列化（跨 Cell 迁移时保存/恢复控制器状态）
    virtual void writeRealToStream(BinaryOStream& stream);
    virtual bool readRealFromStream(BinaryIStream& stream);
};
```

**Ghost 支持**：控制器的 `startGhost()` / `stopGhost()` 确保实体跨 Cell 迁移时控制器状态正确恢复。

### 移动控制器家族

```cpp
// 抽象移动基类
class MoveController : public Controller, public Updatable
{
    virtual void setEntityPosition(const Position3D& position,
        const Direction3D& direction);
    bool move(const Position3D& destination, float* pRemainingTicks = NULL);
};

// 移动到指定点
class MoveToPointController : public MoveController;

// 导航步骤（逐路径点移动）
class NavigateStepController : public MoveController;

// 追踪实体
class MoveToEntityController : public MoveController;
```

### NavigationController：寻路控制器

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/navigation_controller.hpp（简化）
class NavigationController : public Controller, public Updatable
{
    enum NavigationStatus
    {
        NAVIGATION_CANCELLED = -2,
        NAVIGATION_FAILED = -1,
        NAVIGATION_IN_PROGRESS = 0,
        NAVIGATION_COMPLETE = 1
    };

    NavigationStatus move();
    void generateTraversalPath(const NavLoc& srcLoc);

private:
    Vector3Path path_;          // 路径点数组
    int currentNode_;           // 当前路径节点索引
    Position3D destination_;    // 目标位置
    float girth_;               // 实体宽度（影响寻路避障）
    float closeEnough_;         // "足够接近"阈值
};
```

### 转向控制器

```cpp
// 按固定角速度转向
class YawRotatorController : public Controller, public Updatable
{
    float radiansPerTick_;      // 每帧转向角度
    Angle targetYaw_;           // 目标偏航角
    bool turn();                // 执行转向
};

// 面向实体（持续跟踪）
class FaceEntityController : public Controller, public Updatable
{
    EntityPtr pTargetEntity_;   // 目标实体
    float radiansPerTick_;      // 转向速度
    int ticksToNextUpdate_;     // 更新间隔
    int period_;                // 更新周期
};
```

### 加速控制器系统

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/base_acceleration_controller.hpp（简化）
class BaseAccelerationController : public Controller, public Updatable
{
    enum Facing
    {
        FACING_NONE = 0,            // 不改变朝向
        FACING_VELOCITY = 1,        // 面向速度方向
        FACING_ACCELERATION = 2     // 面向加速度方向
    };

    static Vector3 calculateDesiredVelocity(
        const Position3D& currentPosition,
        const Position3D& desiredPosition,
        float acceleration, float maxSpeed, bool stopAtDestination);

    static Vector3 calculateAccelerationVector(
        const Vector3& currentVelocity,
        const Vector3& desiredVelocity);

    float acceleration_;    // 加速度
    float maxSpeed_;        // 最大速度
    Facing facing_;         // 朝向模式
};

// 具体实现
class AccelerateToPointController : public BaseAccelerationController;
class AccelerateAlongPathController : public BaseAccelerationController;
class AccelerateToEntityController : public BaseAccelerationController;
```

加速控制器支持**平滑加减速**，比恒速移动更自然。`calculateDesiredVelocity` 根据距离动态调整速度——远处加速，接近时减速。

### 感知控制器

```cpp
// 视觉控制器（视锥范围内的实体感知）
class VisionController : public Controller, public Updatable
{
    float visionAngle_;          // 视野角度
    float visionRange_;          // 视野距离
    float seeingHeight_;         // 观测高度
    int updatePeriod_;           // 更新周期（每 N tick 检查一次）
    VisionRangeTrigger* pVisionTrigger_;
};

// 临近控制器（球形范围内的实体感知）
class ProximityController : public Controller
{
    float range_;
    ProximityRangeTrigger* pProximityTrigger_;
    BW::vector<EntityID>* pOnloadedSet_;
};
```

### 定时控制器

```cpp
class TimerController : public Controller
{
    class Handler : public TimerHandler
    {
        virtual void handleTimeout(TimerHandle handle, void* pUser);
        virtual void onRelease(TimerHandle handle, void* pUser);
    };

    Handler* pHandler_;
    GameTime start_;             // 开始时间
    GameTime interval_;          // 间隔时间
    TimerHandle timerHandle_;
};
```

### BigWorld 控制器全览

| 控制器 | 用途 | 核心参数 |
|--------|------|---------|
| MoveToPointController | 直线移动到点 | destination, velocity |
| NavigateStepController | 逐路径点移动 | path, velocity |
| MoveToEntityController | 追踪实体 | targetEntity, velocity |
| NavigationController | 寻路移动 | destination, girth, closeEnough |
| YawRotatorController | 按角速度转向 | targetYaw, radiansPerTick |
| FaceEntityController | 面向实体 | targetEntity, radiansPerTick, period |
| AccelerateToPointController | 加速移动到点 | acceleration, maxSpeed, facing |
| AccelerateAlongPathController | 沿路径加速 | path, acceleration, maxSpeed |
| AccelerateToEntityController | 加速追踪实体 | target, acceleration, maxSpeed |
| VisionController | 视锥感知 | visionAngle, visionRange, period |
| ProximityController | 球形范围感知 | range |
| TimerController | 定时回调 | start, interval |

## 16.7 寻路与 AOI 的关系

移动和 AOI 是紧耦合的：

```
实体移动（由 Controller 驱动）
  │
  ├── 每帧更新实体位置
  │     Entity::position(newPos)
  │
  ├── 触发 CoordinateSystem 更新
  │     EntityCoordinateNode::update()
  │     → 冒泡排序式位置调整
  │     → onNodePassX/Z 回调
  │
  ├── 触发 RangeTrigger 检测
  │     正/负边界节点和实体节点互相越过
  │     → ViewTrigger::onEnter / onLeave
  │     → Witness::onEnterView / onLeaveView
  │
  ├── tick 末 Witness::update()
  │     收集所有脏属性（包括位置）
  │     构造 Bundle 发送给客户端
  │     → 客户端看到实体移动
  │
  └── 寻路影响同步频率
        如果实体沿路径高速移动
        → 位置每帧都变（Volatile 属性）
        → 可能超过 Volatile 阈值才同步
        → 远处实体可能只同步方向不同步精确位置
```

**关键交互**：寻路产生路径点序列，移动控制器每帧驱动实体前进，每帧的位置变化都通过 CoordinateSystem 触发 AOI 事件。这意味着**寻路路径的质量直接影响 AOI 事件的频率**——如果路径经过视野边界，会产生进出事件。

## 16.8 射线检测

### KBEngine raycast

```cpp
// 文件：kbe/src/lib/navigation/navigation_handle.h（简化）
// NavMeshHandle::raycast
virtual int raycast(int layer, const Position3D& start,
    const Position3D& end, std::vector<Position3D>& hitPointVec);

// 文件：kbe/src/server/cellapp/cellapp.h（简化）
// Python 绑定
int raycast(SPACE_ID spaceID, int layer,
    const Position3D& start, const Position3D& end,
    std::vector<Position3D>& hitPos);
static PyObject* __py_raycast(PyObject* self, PyObject* args);
```

**基于 Detour 的射线检测**：在 NavMesh 上投射一条射线，返回所有碰撞点。典型用途：

1. **视线检测**：实体 A 能不能"看到"实体 B？
2. **攻击范围验证**：弹道是否被障碍物遮挡？
3. **导航验证**：两个点之间是否有直线路径？

### BigWorld 碰撞检测

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/lib/chunk/terrain_ray_collision_callback.hpp（简化）
class TerrainRayCollisionCallback : public Terrain::TerrainCollisionCallback
{
    virtual bool collide(const WorldTriangle& triangle, float dist);

    bool finishedColliding() const;

private:
    Matrix transform_;
    Vector3 start_;       // 射线起点
    Vector3 end_;         // 射线终点
    Vector3 dir_;         // 射线方向
    float dist_;          // 碰撞距离
};
```

BigWorld 的射线检测基于 Chunk 空间的三角形网格，不只检测 NavMesh 还检测实际的地形和碰撞体。精度更高但开销也更大。

### 服务器端 vs 客户端的射线检测

| 维度 | 服务器端 | 客户端 |
|------|---------|--------|
| 用途 | 视线验证、攻击合法性、导航验证 | 鼠标拾取、UI 交互、视觉效果 |
| 精度 | NavMesh 级别（简化碰撞体） | 完整碰撞体（高精度） |
| 频率 | 按需（脚本调用时） | 每帧（鼠标射线） |
| 数据源 | NavMesh / 简化碰撞体 | 完整场景几何 |

## 16.9 地图数据格式对比

### KBEngine

| 格式 | 来源 | 文件 | 用途 |
|------|------|------|------|
| NavMesh | Recast 导出 / Unity NavMesh 导出 | `.navmesh` 二进制 | 3D 寻路 |
| TMX | Tiled Map Editor 导出 | `.tmx` XML | 2D 格子寻路 |

### BigWorld

| 格式 | 来源 | 文件 | 用途 |
|------|------|------|------|
| Chunk | BigWorld Editor 导出 | `.chunk` / `.cdata` | 空间分割 + 碰撞 + 导航 |
| NavMesh | Recast 内建生成 | Chunk 内嵌 | 3D 寻路 |

BigWorld 的 Chunk 格式更统一——空间分割、碰撞体、导航网格都包含在同一个 Chunk 文件中。KBEngine 需要单独的 NavMesh 文件。

## 16.10 两套项目的移动/寻路系统对比

| 维度 | KBEngine | BigWorld |
|------|----------|----------|
| 导航类型 | NavMesh + NavTile（两种） | NavMesh + Chunk 内建 |
| NavMesh 引擎 | Detour（外部库） | Detour（外部库）+ 内建 |
| 2D 支持 | NavTile（TMX 格式） | 无 |
| 移动控制器 | MoveController（1 种） | 10+ 种控制器 |
| 寻路控制器 | NavigateHandler | NavigationController |
| 加速移动 | 无 | Accelerate* 系列（3 种） |
| 转向控制 | 无 | YawRotator + FaceEntity |
| 视觉感知 | 无 | VisionController |
| 临近感知 | 无（用 ViewTrigger 替代） | ProximityController |
| 定时器 | 无控制器（Python 层实现） | TimerController |
| Ghost 支持 | 无 | startGhost/stopGhost |
| 射线检测 | Detour NavMesh raycast | Chunk 三角形碰撞 |
| 多层导航 | 支持（navmeshLayer map） | 支持 |
| 异步加载 | LoadNavmeshTask（线程池） | Chunk 异步加载 |

**核心差异**：BigWorld 的控制器系统是**可组合的组件库**——开发者可以选择不同控制器组合出各种移动行为。KBEngine 的控制器更简单——只有移动和旋转两种，其他行为在 Python 脚本层实现。

## 16.11 关键源码入口

### KBEngine

| 概念 | 文件 |
|------|------|
| Navigation（管理器） | `kbe/src/lib/navigation/navigation.h` |
| NavigationHandle | `kbe/src/lib/navigation/navigation_handle.h` |
| NavMeshHandle | `kbe/src/lib/navigation/navigation_mesh_handle.h` |
| NavTileHandle | `kbe/src/lib/navigation/navigation_tile_handle.h` |
| NavigateHandler | `kbe/src/server/cellapp/navigate_handler.h` |
| MoveController | `kbe/src/server/cellapp/move_controller.h` |
| Controller 基类 | `kbe/src/server/cellapp/controller.h` |
| LoadNavmeshTask | `kbe/src/server/cellapp/loadnavmesh_threadtasks.h` |
| raycast | `kbe/src/server/cellapp/cellapp.h` |
| SpaceMemory（导航句柄） | `kbe/src/server/cellapp/spacememory.h` |

### BigWorld

| 概念 | 文件 |
|------|------|
| Controller 基类 | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/controller.hpp` |
| MoveController | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/move_controller.hpp` |
| NavigationController | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/navigation_controller.hpp` |
| YawRotatorController | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/turn_controller.hpp` |
| FaceEntityController | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/face_entity_controller.hpp` |
| AccelerateToPoint | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/base_acceleration_controller.hpp` |
| VisionController | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/vision_controller.hpp` |
| ProximityController | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/proximity_controller.hpp` |
| TimerController | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/timer_controller.hpp` |
| RecastGenerator | `BigWorld-Engine-14.4.1/programming/bigworld/lib/chunk/chunk/navmesh_generator.hpp` |
| TerrainRayCollision | `BigWorld-Engine-14.4.1/programming/bigworld/lib/chunk/terrain_ray_collision_callback.hpp` |

## 16.12 源码走读路径

### 路径一：理解 NavMesh 寻路的完整流程

1. `kbe/src/lib/navigation/navigation_mesh_handle.h` — NavMeshHandle::findStraightPath() 使用 Detour API
2. `kbe/src/server/cellapp/navigate_handler.h` — NavigateHandler 持有路径点列表
3. `kbe/src/server/cellapp/move_controller.h` — MoveController 驱动逐帧移动

### 路径二：对比 BigWorld 的控制器系统

1. `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/controller.hpp` — 基类，Ghost/Real 生命周期
2. `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/move_controller.hpp` — 移动控制器家族
3. `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/base_acceleration_controller.hpp` — 加速控制器
4. `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/vision_controller.hpp` — 感知控制器

### 路径三：理解射线检测

1. `kbe/src/lib/navigation/navigation_handle.h` — raycast() 接口
2. `kbe/src/server/cellapp/cellapp.h` — Python 绑定 __py_raycast()
3. `BigWorld-Engine-14.4.1/programming/bigworld/lib/chunk/terrain_ray_collision_callback.hpp` — 三角形碰撞检测

## 16.13 小结

- **NavMesh 是 MMO 寻路的标准方案**：连续空间、精度自适应、支持 3D 地形
- **Recast 是 NavMesh 生成的工业标准库**：体素化→区域划分→轮廓简化→多边形网格→细节网格
- **KBEngine 有两种导航后端**：NavMeshHandle（Detour 3D 导航）和 NavTileHandle（A* 2D 格子导航）
- **NavigateHandler 持有路径点列表，逐帧驱动移动**：到达当前路径点后切换到下一个
- **BigWorld 有 10+ 种控制器**：移动、导航、转向、加速、面向实体、视觉感知、临近感知、定时器
- **加速控制器支持平滑加减速**：calculateDesiredVelocity 根据距离动态调整速度
- **移动每帧触发 AOI 更新**：CoordinateSystem 冒泡→RangeTrigger 检测→Witness 广播
- **射线检测用于视线验证和攻击合法性**：服务器端基于 NavMesh（高效），客户端基于完整几何（精确）
- **BigWorld 的控制器系统是可组合的组件库**：选择不同控制器组合出各种移动行为
