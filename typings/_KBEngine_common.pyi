from collections.abc import Callable, Sequence
from typing import (
    Any,
    BinaryIO,
    Dict,
    IO,
    List,
    TextIO,
    Tuple,
    Union,
    overload,
)

try:
    from typing import Literal
except ImportError:
    from typing_extensions import Literal

from _KBEngine_server_errors import *

# Type aliases
Vector3Like = Union[Tuple[float, float, float], Sequence[float]]
Address = Any
WatcherValue = Any
Callback = Callable[..., Any]

# 源码只有单一 EntityCall 脚本类型（lib/entitydef/entity_call.h:15）。
# base/cell/client 之别是内部枚举 ENTITYCALL_TYPE（lib/common/common.h:283-291：
# CELL=0, BASE=1, CLIENT=2, CELL_VIA_BASE=3, BASE_VIA_CELL=4,
# CLIENT_VIA_CELL=5, CLIENT_VIA_BASE=6），未注册为脚本可见常量，
# 故此不设 BaseEntityCall/CellEntityCall/ClientEntityCall 子类。
class EntityCall:
    def __getattr__(self, name: str) -> Any: ...
    @overload
    def getComponent(
        self, componentName: str, all: Literal[False] = False
    ) -> Union[EntityComponentCall, None]: ...
    @overload
    def getComponent(
        self, componentName: str, all: Literal[True]
    ) -> Tuple[EntityComponentCall, ...]: ...

class EntityComponentCall(EntityCall): ...

class MemoryStream:
    def __init__(self) -> None: ...
    def append(self, dataType: str, value: Any) -> None: ...
    def pop(self, dataType: str) -> Any: ...
    # 方法名即 bytes（非 __bytes__ dunder），lib/pyscript/py_memorystream.cpp:30
    def bytes(self) -> bytes: ...
    # 读/写位置：0 参取值 -> int，1 参设位置 -> None（py_memorystream.cpp:540/576）
    @overload
    def rpos(self) -> int: ...
    @overload
    def rpos(self, pos: int) -> None: ...
    @overload
    def wpos(self) -> int: ...
    @overload
    def wpos(self, pos: int) -> None: ...
    # val 须为 bytes 或 MemoryStream（py_memorystream.cpp:604-646）
    def fill(self, val: Union[bytes, "MemoryStream"]) -> None: ...
    def __reduce_ex__(self, protocol: int) -> Any: ...

class GlobalDataClient(Dict[str, Any]):
    pass

class Entity:
    id: int
    className: str
    client: Union[EntityCall, None]
    cell: Union[EntityCall, None]
    base: Union[EntityCall, None]
    databaseID: int
    databaseInterfaceName: str
    isDestroyed: bool
    shouldAutoArchive: Union[bool, int]
    shouldAutoBackup: Union[bool, int]
    # ---- cellapp 侧属性（server/cellapp/entity.cpp:76-90 SCRIPT_GET(SET)_DECLARE）----
    allClients: Any
    otherClients: Any
    isWitnessed: bool
    hasWitness: bool
    isOnGround: bool
    spaceID: int
    layer: int
    position: Vector3Like
    direction: Vector3Like
    topSpeed: float
    topSpeedY: float
    controlledBy: Any
    volatileInfo: Any

    def __init__(self) -> None: ...
    def __getattr__(self, name: str) -> Any: ...
    def __reduce_ex__(self, protocol: int) -> Any: ...
    def __getDEP__(self) -> Any: ...
    def addTimer(
        self, initialOffset: float, repeatOffset: float = 0.0, userArg: int = 0
    ) -> int: ...
    def delTimer(self, id: Union[int, str]) -> None: ...
    def destroy(
        self, deleteFromDB: bool = False, writeToDB: bool = False
    ) -> None: ...
    def writeToDB(
        self,
        callback: Union[Callback, None] = None,
        shouldAutoLoad: bool = False,
        dbInterfaceName: str = "default",
    ) -> None: ...
    @overload
    def getComponent(
        self, componentName: str, all: Literal[False] = False
    ) -> Union[EntityComponent, None]: ...
    @overload
    def getComponent(
        self, componentName: str, all: Literal[True]
    ) -> Tuple[EntityComponent, ...]: ...
    def fireEvent(self, eventName: str, *args: Any) -> None: ...
    def registerEvent(self, eventName: str, callback: Callback) -> None: ...
    def deregisterEvent(self, eventName: str, callback: Callback) -> None: ...
    def createCellEntity(
        self, cellEntityCall: Union[EntityCall, None] = None
    ) -> None: ...
    def createCellEntityInNewSpace(self, cellappIndex: int = 0) -> None: ...
    def destroyCellEntity(self) -> None: ...
    def clientEntity(self, destID: int) -> EntityCall: ...
    def isReal(self) -> bool: ...
    def cancelController(self, controllerID: Union[int, str]) -> None: ...
    def canNavigate(self) -> bool: ...
    def setViewRadius(self, radius: float, hyst: float = 5.0) -> None: ...
    def getViewRadius(self) -> float: ...
    def getViewHystArea(self) -> float: ...
    # ---- cellapp 侧移动/导航方法（server/cellapp/entity.cpp:52-64，
    #      签名照 DECLARE_PY_MOTHOD_ARGn 解析面，实参个数恒定）----
    def addProximity(self, range_xz: float, range_y: float, userArg: int) -> int: ...
    def addYawRotator(
        self, yaw: float, velocity: float, userData: Any
    ) -> int: ...
    def navigate(
        self,
        destination: Vector3Like,
        velocity: float,
        distance: float,
        maxMoveDistance: float,
        maxDistance: float,
        faceMovement: int,
        layer: int,
        userData: Any,
    ) -> int: ...
    def navigatePathPoints(
        self, destination: Vector3Like, maxSearchDistance: float, layer: int
    ) -> Tuple[Vector3Like, ...]: ...
    def getRandomPoints(
        self, centerPos: Vector3Like, maxRadius: float, maxPoints: int, layer: int
    ) -> Tuple[Vector3Like, ...]: ...
    def moveToPoint(
        self,
        destination: Vector3Like,
        velocity: float,
        distance: float,
        userData: Any,
        faceMovement: int,
        moveVertically: int,
    ) -> int: ...
    @overload
    def moveToEntity(
        self,
        targetID: int,
        velocity: float,
        distance: float,
        userData: Any,
        faceMovement: int,
        moveVertically: int,
    ) -> int: ...
    @overload
    def moveToEntity(
        self,
        targetID: int,
        velocity: float,
        distance: float,
        userData: Any,
        faceMovement: int,
        moveVertically: int,
        offset: Vector3Like,
    ) -> int: ...
    def accelerate(self, type: str, acceleration: float) -> float: ...
    def entitiesInView(self, pending: bool = False) -> Tuple[Entity, ...]: ...
    def entitiesInRange(
        self,
        range: float,
        entityType: Union[str, None] = None,
        position: Union[Vector3Like, None] = None,
    ) -> Tuple[Entity, ...]: ...
    def teleport(
        self,
        nearbyMBRef: Union[EntityCall, Entity, None],
        position: Vector3Like,
        direction: Vector3Like,
    ) -> None: ...
    def destroySpace(self) -> None: ...
    def debugView(self) -> None: ...
    def getWitnesses(self) -> Tuple[Entity, ...]: ...

class Proxy(Entity):
    __ACCOUNT_NAME__: str
    __ACCOUNT_PASSWORD__: str
    clientAddr: Address
    clientEnabled: bool
    hasClient: bool
    roundTripTime: float
    timeSinceHeardFromClient: float

    def disconnect(self) -> None: ...
    def getClientType(self) -> int: ...
    def getClientDatas(self) -> Tuple[bytes, bytes]: ...
    def giveClientTo(self, proxy: Proxy) -> None: ...
    def streamFileToClient(
        self, resourceName: str, desc: str = "", id: int = -1
    ) -> int: ...
    def streamStringToClient(
        self, data: Union[str, bytes], desc: str = "", id: int = -1
    ) -> int: ...

class Space(Entity):
    pass

class EntityComponent:
    client: Union[EntityCall, None]
    cell: Union[EntityCall, None]
    base: Union[EntityCall, None]

    def __init__(self) -> None: ...
    def __getattr__(self, name: str) -> Any: ...
    def addTimer(
        self, initialOffset: float, repeatOffset: float = 0.0, userArg: int = 0
    ) -> int: ...
    def delTimer(self, id: Union[int, str]) -> None: ...
    def clientEntity(self, destID: int) -> EntityCall: ...

class Entities(Dict[int, Entity]):
    pass

# 仅 baseapp 注入（server/baseapp/baseapp.cpp:463-473），其余组件模块上不存在
LOG_ON_ACCEPT: int
LOG_ON_REJECT: int
LOG_ON_WAIT_FOR_DESTROY: int
LOG_TYPE_NORMAL: int
LOG_TYPE_INFO: int
LOG_TYPE_ERR: int
LOG_TYPE_DBG: int
LOG_TYPE_WAR: int
NEXT_ONLY: int

component: str

def publish() -> int: ...
def scriptLogType(logType: int) -> None: ...
def getResFullPath(res: str) -> str: ...
def hasRes(res: str) -> bool: ...
def listPathRes(path: str, extension: str = "") -> Tuple[str, ...]: ...
def matchPath(res: str) -> str: ...
# 源码零参（lib/pyscript/py_gc.cpp:224-228）；pythonapp.pyi 的 debugTracing() 同
def debugTracing() -> None: ...
def genUUID64() -> int: ...
# baseapp/cellapp 模块未注册模块级 addTimer；仅 PythonApp 公共面
# （lib/server/python_app.cpp:721-726）恰 3 参 "ffO"，无默认值
def addTimer(
    initialOffset: float, repeatOffset: float, callbackObj: Callback
) -> int: ...
# 返回 timerID，失败 -1（python_app.cpp:766-771）
def delTimer(id: int) -> int: ...
def registerReadFileDescriptor(
    fileDescriptor: int, callback: Callback
) -> None: ...
def registerWriteFileDescriptor(
    fileDescriptor: int, callback: Callback
) -> None: ...
def deregisterReadFileDescriptor(fileDescriptor: int) -> None: ...
def deregisterWriteFileDescriptor(fileDescriptor: int) -> None: ...
def executeRawDatabaseCommand(
    command: str,
    callback: Union[Callback, None] = None,
    threadID: int = 0,
    dbInterfaceName: str = "default",
) -> None: ...
# 恰 2 参、无 encoding（lib/server/python_app.cpp:444-452）
def open(
    res: str, mode: str = "r"
) -> Union[IO[Any], TextIO, BinaryIO]: ...
# 仅 baseapp/cellapp 模块面（lib/server/entity_app.h:423）
def kbassert() -> None: ...
# 以下 3 个仅 interfaces 注册（server/tools/interfaces/interfaces.cpp:186-188），
# 其余组件模块上不存在（typing 层公共超集保留，调用方注意组件归属）
def accountLoginResponse(
    commitName: str, realAccountName: str, extraDatas: bytes, errorCode: int
) -> None: ...
def createAccountResponse(
    commitName: str, realAccountName: str, extraDatas: bytes, errorCode: int
) -> None: ...
def chargeResponse(
    orderID: str, extraDatas: bytes, errorCode: int
) -> None: ...
def urlopen(
    url: str,
    callback: Union[Callback, None] = None,
    postData: Union[bytes, None] = None,
    headers: Union[Dict[str, str], None] = None,
) -> None: ...
def __getattr__(name: str) -> Any: ...
