from collections.abc import Callable
from typing import Any, Dict, Tuple, Union

from _KBEngine_common import (
    Address,
    BaseEntityCall,
    Callback,
    Entities,
    Entity,
    GlobalDataClient,
    WatcherValue,
)

entities: Entities
globalData: GlobalDataClient
baseAppData: GlobalDataClient

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
    baseMB: BaseEntityCall,
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
    baseMB: BaseEntityCall,
    callback: Union[Callback, None] = None,
    dbInterfaceName: str = "default",
) -> None: ...
def charge(
    ordersID: str,
    dbID: int,
    byteDatas: bytes,
    pycallback: Union[Callback, None] = None,
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
