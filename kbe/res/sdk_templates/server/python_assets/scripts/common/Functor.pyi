from __future__ import annotations

from typing import Any, Callable


class Functor:
    func: Callable[..., Any]
    args: tuple[Any, ...]

    def __init__(self, func: Callable[..., Any], *args: Any) -> None: ...
    def __call__(self, *args: Any) -> Any: ...
