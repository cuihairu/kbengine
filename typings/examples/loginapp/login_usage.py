from __future__ import annotations

from typing import TYPE_CHECKING, assert_type

import KBEngine

if TYPE_CHECKING:
    from KBEngine.hooks import LoginAppModuleHooks


def accepts_login_hooks(hooks: LoginAppModuleHooks) -> None:
    hooks.onLoginAppReady()
    hooks.onRequestLogin("account", "password", 6, b"demo")


def typecheck_only() -> None:
    def on_tick(timer_id: int) -> None:
        KBEngine.delTimer(timer_id)

    timer_id = KBEngine.addTimer(0.1, 0.0, on_tick)
    assert_type(timer_id, int)
    assert_type(KBEngine.component, str)
    assert_type(KBEngine.publish(), int)
    assert_type(KBEngine.SERVER_SUCCESS, int)
    assert_type(KBEngine.SERVER_ERR_SRV_NO_READY, int)
    assert_type(KBEngine.SERVER_ERR_NAME, int)
    assert_type(KBEngine.SERVER_ERR_PASSWORD, int)
    assert_type(KBEngine.SERVER_ERR_ACCOUNT_LOGIN_ANOTHER_SERVER, int)
