from __future__ import annotations

from typing import TYPE_CHECKING, assert_type

import KBEngine

if TYPE_CHECKING:
    from KBEngine.hooks import LoggerModuleHooks


def accepts_logger_hooks(hooks: LoggerModuleHooks) -> None:
    hooks.onLoggerAppReady()
    hooks.onReadyForShutDown()


def typecheck_only() -> None:
    def on_tick(timer_id: int) -> None:
        KBEngine.delTimer(timer_id)

    timer_id = KBEngine.addTimer(0.1, 0.0, on_tick)
    assert_type(timer_id, int)
    assert_type(KBEngine.component, str)
    assert_type(KBEngine.publish(), int)
    KBEngine.debugTracing()
