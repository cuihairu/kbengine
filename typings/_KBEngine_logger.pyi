from collections.abc import Callable
from typing import Any, List, Tuple, Union

from _KBEngine_server_errors import *
from _KBEngine_pythonapp import *

# Type aliases
DatabaseCommandResult = Union[List[List[str]], None]
DatabaseCommandCallback = Callable[[DatabaseCommandResult, Union[int, None], Union[int, None], Union[str, None]], Any]


def accountLoginResponse(commitName: str, realAccountName: str, extraDatas: bytes, errorCode: int) -> None: ...
def createAccountResponse(commitName: str, realAccountName: str, extraDatas: bytes, errorCode: int) -> None: ...
def chargeResponse(orderID: str, extraDatas: bytes, errorCode: int) -> None: ...
def executeRawDatabaseCommand(
    command: str,
    callback: Union[DatabaseCommandCallback, None] = None,
    threadID: int = 0,
    dbInterfaceName: str = "default"
) -> None: ...
def __getattr__(name: str) -> Any: ...
