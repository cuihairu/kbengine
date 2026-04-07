from __future__ import annotations

from typing import TYPE_CHECKING, assert_type

import KBEngine

if TYPE_CHECKING:
    from KBEngine.hooks import InterfacesModuleHooks


def onInterfaceAppReady() -> None:
    def on_tick(timer_id: int) -> None:
        KBEngine.delTimer(timer_id)

    timer_id = KBEngine.addTimer(0.1, 0.0, on_tick)
    assert_type(timer_id, int)


def accepts_interfaces_hooks(hooks: InterfacesModuleHooks) -> None:
    hooks.onInterfaceAppReady()
    hooks.onRequestCharge("order-1", 1, b"ok")


def typecheck_only() -> None:
    assert_type(KBEngine.component, str)
    assert_type(KBEngine.publish(), int)
    assert_type(KBEngine.SERVER_ERR_SRV_OVERLOAD, int)
    payload = KBEngine.MemoryStream()
    payload.append("UINT32", 1)
    KBEngine.createAccountResponse("commit", "account", b"demo", KBEngine.SERVER_SUCCESS)
    KBEngine.accountLoginResponse("commit", "account", b"demo", KBEngine.SERVER_ERR_LOCAL_PROCESSING)
    KBEngine.chargeResponse("order-1", b"ok", KBEngine.SERVER_SUCCESS)
    KBEngine.executeRawDatabaseCommand("SELECT 1")
