from __future__ import annotations

from typing import TYPE_CHECKING, assert_type

import KBEngine

if TYPE_CHECKING:
    from KBEngine.hooks import BotsModuleHooks


class Account(KBEngine.Entity):
    def onDestroy(self) -> None:
        pass


def accepts_bots_hooks(hooks: BotsModuleHooks) -> None:
    hooks.onInit(False)
    hooks.onFinish()


def typecheck_only() -> None:
    assert_type(KBEngine.component, str)
    assert_type(KBEngine.bots, KBEngine.PyBots)
    assert_type(KBEngine.genUUID64(), int)
    KBEngine.addBots(10, 2, 0.5)

    bot = KBEngine.bots.get(1)
    if bot is not None:
        assert_type(bot.id, int)
        assert_type(bot.entities, KBEngine.Entities)
