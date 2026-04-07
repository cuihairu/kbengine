from __future__ import annotations

from collections.abc import Callable
from typing import Any, TypeAlias

from _KBEngine_server_errors import *
from _KBEngine_pythonapp import *

DatabaseCommandResult: TypeAlias = list[list[str]] | None
DatabaseCommandCallback: TypeAlias = Callable[[DatabaseCommandResult, int | None, int | None, str | None], Any]


def executeRawDatabaseCommand(
    command: str,
    callback: DatabaseCommandCallback | None = None,
    threadID: int = 0,
    dbInterfaceName: str = "default"
) -> None: ...
def __getattr__(name: str) -> Any: ...
