from __future__ import annotations

from typing import TYPE_CHECKING, assert_type

import KBEngine

if TYPE_CHECKING:
    from KBEngine.hooks import CellAppModuleHooks


def onInit(isReload: bool) -> None:
    pass


def accepts_cellapp_hooks(hooks: CellAppModuleHooks) -> None:
    hooks.onInit(False)
    hooks.onCellAppData("weather", "sunny")


def typecheck_only() -> None:
    entity = KBEngine.createEntity(
        "Avatar",
        1,
        (0.0, 0.0, 0.0),
        (0.0, 0.0, 0.0),
        {"nickname": "demo"},
    )
    assert_type(entity, KBEngine.Entity | None)
    assert_type(KBEngine.cellAppData, KBEngine.GlobalDataClient)
    assert_type(KBEngine.globalData, KBEngine.GlobalDataClient)
    assert_type(KBEngine.address(), KBEngine.Address)

    KBEngine.setSpaceData(1, "weather", "sunny")
    assert_type(KBEngine.getSpaceData(1, "weather"), str)
    assert_type(KBEngine.getSpaceGeometryMapping(1), str)
