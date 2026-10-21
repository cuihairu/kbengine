from collections.abc import Callable
from typing import Any, Dict, Tuple, Union

from _KBEngine_common import (
    Address,
    Callback,
    Entities,
    Entity,
    EntityCall,
    GlobalDataClient,
    WatcherValue,
)

entities: Entities
globalData: GlobalDataClient
baseAppData: GlobalDataClient

# baseapp/cellapp 模块面（lib/server/entity_app.h:423）
def kbassert() -> None: ...

def address() -> Address: ...
def time() -> int: ...
def isShuttingDown() -> bool: ...
def getWatcher(path: str) -> WatcherValue: ...
def getWatcherDir(path: str) -> Tuple[str, ...]: ...
def addWatcher(
    path: str,
    dataType: str,
    getFunction: Callable[[], WatcherValue],
) -> None: ...
def delWatcher(path: str) -> None: ...
def setAppFlags(flags: int) -> None: ...
def getAppFlags() -> int: ...
def reloadScript(fullReload: bool = False) -> None: ...
def quantumPassedPercent() -> int: ...
def createEntity(
    entityType: str,
    params: Union[Dict[str, Any], None] = None,
) -> Union[Entity, None]: ...
def createEntityLocally(
    entityType: str,
    params: Union[Dict[str, Any], None] = None,
) -> Union[Entity, None]: ...
def createEntityAnywhere(
    entityType: str,
    params: Union[Dict[str, Any], None] = None,
    callback: Union[Callback, None] = None,
) -> None: ...
def createEntityRemotely(
    entityType: str,
    baseMB: EntityCall,
    params: Union[Dict[str, Any], None] = None,
    callback: Union[Callback, None] = None,
) -> None: ...
def createEntityFromDBID(
    entityType: str,
    dbID: int,
    callback: Union[Callback, None] = None,
    dbInterfaceName: str = "default",
) -> None: ...
def createEntityAnywhereFromDBID(
    entityType: str,
    dbID: int,
    callback: Union[Callback, None] = None,
    dbInterfaceName: str = "default",
) -> None: ...
def createEntityRemotelyFromDBID(
    entityType: str,
    dbID: int,
    baseMB: EntityCall,
    callback: Union[Callback, None] = None,
    dbInterfaceName: str = "default",
) -> None: ...
# 恰 4 参，pycallback 不可省（server/baseapp/baseapp.cpp:3398-3403）
def charge(
    ordersID: str,
    dbID: int,
    byteDatas: bytes,
    pycallback: Callback,
) -> None: ...
def deleteEntityByDBID(
    entityType: str,
    dbID: int,
    callback: Union[Callback, None] = None,
    dbInterfaceName: str = "default",
) -> None: ...
def lookUpEntityByDBID(
    entityType: str,
    dbID: int,
    callback: Callback,
    dbInterfaceName: str = "default",
) -> None: ...
