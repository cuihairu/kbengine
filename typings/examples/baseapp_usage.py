from __future__ import annotations

from typing import TYPE_CHECKING, assert_type

import KBEngine

if TYPE_CHECKING:
    from KBEngine.hooks import BaseAppModuleHooks


class Account(KBEngine.Proxy):
    def onClientEnabled(self) -> None:
        timer_id = self.addTimer(1.0, 0.0, 7)
        self.delTimer(timer_id)
        self.streamStringToClient("server-ready", "status")

    def onLogOnAttempt(self, ip: str, port: int, password: str) -> int:
        return KBEngine.SERVER_SUCCESS


def onInit(isReload: bool) -> None:
    timer_id = KBEngine.addTimer(0.1, 0.0)
    KBEngine.delTimer(timer_id)
    KBEngine.getResFullPath("scripts/base/Account.py")


def accepts_baseapp_hooks(hooks: BaseAppModuleHooks) -> None:
    hooks.onInit(False)
    hooks.onReadyForShutDown()


def typecheck_only() -> None:
    entity = KBEngine.createEntityLocally("Account", {"nickname": "demo"})
    assert_type(KBEngine.baseAppData, KBEngine.GlobalDataClient)
    assert_type(KBEngine.globalData, KBEngine.GlobalDataClient)
    assert_type(KBEngine.address(), KBEngine.Address)
    if entity is not None:
        entity.addTimer(1.0, 0.0, 1)
        entity.delTimer(1)
