from __future__ import annotations

from typing import TYPE_CHECKING, assert_type

import KBEngine

if TYPE_CHECKING:
    from KBEngine.hooks import DBMgrModuleHooks


def accepts_dbmgr_hooks(hooks: DBMgrModuleHooks) -> None:
    hooks.onDBMgrReady()
    hooks.onSelectAccountDBInterface("account")


def typecheck_only() -> None:
    def on_tick(timer_id: int) -> None:
        KBEngine.delTimer(timer_id)

    timer_id = KBEngine.addTimer(0.1, 0.0, on_tick)
    assert_type(timer_id, int)
    assert_type(KBEngine.component, str)
    assert_type(KBEngine.publish(), int)
    assert_type(KBEngine.SERVER_ERR_DB, int)
    KBEngine.executeRawDatabaseCommand("SELECT 1")
