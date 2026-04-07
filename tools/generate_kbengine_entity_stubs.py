#!/usr/bin/env python3
from __future__ import annotations

import argparse
import sys
import xml.etree.ElementTree as ET
from collections import OrderedDict
from dataclasses import dataclass, field
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]

BASE_VISIBLE_FLAGS = {"BASE", "BASE_AND_CLIENT"}
CELL_VISIBLE_FLAGS = {
    "CELL_PUBLIC",
    "CELL_PRIVATE",
    "ALL_CLIENTS",
    "CELL_PUBLIC_AND_OWN",
    "OWN_CLIENT",
    "OTHER_CLIENTS",
}
CLIENT_VISIBLE_FLAGS = {
    "BASE_AND_CLIENT",
    "ALL_CLIENTS",
    "CELL_PUBLIC_AND_OWN",
    "OWN_CLIENT",
    "OTHER_CLIENTS",
}

BUILTIN_TYPE_MAP = {
    "UINT8": "int",
    "UINT16": "int",
    "UINT32": "int",
    "UINT64": "int",
    "INT8": "int",
    "INT16": "int",
    "INT32": "int",
    "INT64": "int",
    "FLOAT": "float",
    "DOUBLE": "float",
    "STRING": "str",
    "UNICODE": "str",
    "BLOB": "bytes",
    "PYTHON": "Any",
    "VECTOR2": "tuple[float, float]",
    "VECTOR3": "tuple[float, float, float]",
    "VECTOR4": "tuple[float, float, float, float]",
    "ENTITYCALL": "KBEngine.EntityCall",
}

BASE_ENTITY_HOOKS = OrderedDict(
    [
        ("__init__", "def __init__(self) -> None: ..."),
        ("onCreateCellFailure", "def onCreateCellFailure(self) -> None: ..."),
        ("onDestroy", "def onDestroy(self) -> None: ..."),
        ("onGetCell", "def onGetCell(self) -> None: ..."),
        ("onLoseCell", "def onLoseCell(self) -> None: ..."),
        ("onPreArchive", "def onPreArchive(self) -> None: ..."),
        ("onRestore", "def onRestore(self) -> None: ..."),
        ("onTimer", "def onTimer(self, timerHandle: int, userData: int) -> None: ..."),
        ("onWriteToDB", "def onWriteToDB(self, cellData: Any) -> None: ..."),
    ]
)

PROXY_HOOKS = OrderedDict(
    [
        ("onClientDeath", "def onClientDeath(self) -> None: ..."),
        ("onClientGetCell", "def onClientGetCell(self) -> None: ..."),
        ("onClientEnabled", "def onClientEnabled(self) -> None: ..."),
        ("onGiveClientToFailure", "def onGiveClientToFailure(self) -> None: ..."),
        ("onLogOnAttempt", "def onLogOnAttempt(self, ip: str, port: int, password: str) -> int: ..."),
        ("onStreamComplete", "def onStreamComplete(self, id: int, success: bool) -> None: ..."),
    ]
)

CELL_ENTITY_HOOKS = OrderedDict(
    [
        ("__init__", "def __init__(self) -> None: ..."),
        ("onDestroy", "def onDestroy(self) -> None: ..."),
        ("onEnteredView", "def onEnteredView(self, entity: KBEngine.Entity) -> None: ..."),
        ("onGetWitness", "def onGetWitness(self) -> None: ..."),
        ("onLoseWitness", "def onLoseWitness(self) -> None: ..."),
        ("onMove", "def onMove(self, controllerID: int, userData: int) -> None: ..."),
        ("onMoveFailure", "def onMoveFailure(self, controllerID: int, userData: int) -> None: ..."),
        ("onMoveOver", "def onMoveOver(self, controllerID: int, userData: int) -> None: ..."),
        ("onRestore", "def onRestore(self) -> None: ..."),
        ("onSpaceGone", "def onSpaceGone(self) -> None: ..."),
        ("onTeleport", "def onTeleport(self) -> None: ..."),
        ("onTeleportFailure", "def onTeleportFailure(self) -> None: ..."),
        ("onTeleportSuccess", "def onTeleportSuccess(self, nearbyEntity: KBEngine.Entity) -> None: ..."),
        ("onTimer", "def onTimer(self, timerHandle: int, userData: int) -> None: ..."),
        ("onTurn", "def onTurn(self, controllerID: int, userData: int) -> None: ..."),
        ("onUpdateBegin", "def onUpdateBegin(self) -> None: ..."),
        ("onUpdateEnd", "def onUpdateEnd(self) -> None: ..."),
        ("onWitnessed", "def onWitnessed(self, isWitnessed: bool) -> None: ..."),
        ("onWriteToDB", "def onWriteToDB(self) -> None: ..."),
    ]
)

COMPONENT_HOOKS = OrderedDict(
    [
        ("__init__", "def __init__(self) -> None: ..."),
    ]
)


@dataclass
class PropertyModel:
    name: str
    source_type: str
    type_expr: str
    flags: str


@dataclass
class MethodModel:
    name: str
    arg_source_types: list[str] = field(default_factory=list)
    arg_type_exprs: list[str] = field(default_factory=list)
    exposed: bool = False


@dataclass
class ComponentRef:
    name: str
    type_name: str


@dataclass
class ResolvedComponentRef:
    name: str
    type_name: str
    has_base: bool
    has_cell: bool
    has_client: bool


@dataclass
class DefinitionModel:
    properties: OrderedDict[str, PropertyModel] = field(default_factory=OrderedDict)
    base_methods: OrderedDict[str, MethodModel] = field(default_factory=OrderedDict)
    cell_methods: OrderedDict[str, MethodModel] = field(default_factory=OrderedDict)
    client_methods: OrderedDict[str, MethodModel] = field(default_factory=OrderedDict)
    components: OrderedDict[str, ComponentRef] = field(default_factory=OrderedDict)

    def merge_from(self, other: DefinitionModel) -> None:
        merge_ordered_dict(self.properties, other.properties)
        merge_ordered_dict(self.base_methods, other.base_methods)
        merge_ordered_dict(self.cell_methods, other.cell_methods)
        merge_ordered_dict(self.client_methods, other.client_methods)
        merge_ordered_dict(self.components, other.components)


@dataclass
class EntityEntry:
    name: str
    has_base: bool
    has_cell: bool
    has_client: bool


@dataclass
class TypeDescriptor:
    kind: str
    target: str | None = None
    item_type: str | None = None
    implemented_by: str | None = None
    fields: OrderedDict[str, str] = field(default_factory=OrderedDict)


def merge_ordered_dict(target: OrderedDict[str, object], source: OrderedDict[str, object]) -> None:
    for key, value in source.items():
        if key not in target:
            target[key] = value


class TypeRegistry:
    def __init__(self, types_path: Path):
        self.types_path = types_path
        self._descriptors: dict[str, TypeDescriptor] = {}

    def load(self) -> None:
        if not self.types_path.exists():
            return

        root = ET.parse(self.types_path).getroot()
        for node in root:
            alias_name = node.tag.strip()
            value = leading_token(node).upper()

            if value == "ARRAY" or (not value and node.find("of") is not None):
                self._descriptors[alias_name] = TypeDescriptor(
                    kind="array",
                    item_type=node_value(node.find("of")),
                )
                continue

            if value == "FIXED_DICT" or node.find("Properties") is not None:
                self._descriptors[alias_name] = TypeDescriptor(
                    kind="fixed_dict",
                    implemented_by=node_value(node.find("implementedBy")),
                    fields=self._parse_fixed_dict_fields(node.find("Properties")),
                )
                continue

            self._descriptors[alias_name] = TypeDescriptor(kind="alias", target=leading_token(node))

    def _parse_fixed_dict_fields(self, node: ET.Element | None) -> OrderedDict[str, str]:
        fields: OrderedDict[str, str] = OrderedDict()
        if node is None:
            return fields

        for field_node in node:
            field_name = field_node.tag.strip()
            field_type = node_value(field_node.find("Type"))
            if not field_name or not field_type:
                continue
            fields[field_name] = field_type

        return fields

    def render(self, type_name: str) -> str:
        return self._render(type_name.strip(), set())

    def _render(self, type_name: str, seen: set[str]) -> str:
        upper = type_name.upper()
        if upper in BUILTIN_TYPE_MAP:
            return BUILTIN_TYPE_MAP[upper]

        if type_name in seen:
            return "Any"

        descriptor = self._descriptors.get(type_name)
        if descriptor is None:
            return "Any"

        seen.add(type_name)

        if descriptor.kind == "alias" and descriptor.target:
            rendered = self._render(descriptor.target, seen)
        elif descriptor.kind == "array" and descriptor.item_type:
            rendered = f"list[{self._render(descriptor.item_type, seen)}]"
        elif descriptor.kind == "fixed_dict":
            rendered = type_name
        else:
            rendered = "Any"

        seen.remove(type_name)
        return rendered

    def collect_fixed_dict_descriptors(self, type_names: list[str]) -> OrderedDict[str, TypeDescriptor]:
        descriptors: OrderedDict[str, TypeDescriptor] = OrderedDict()
        for type_name in type_names:
            self._collect_fixed_dict_descriptors(type_name.strip(), descriptors, set())
        return descriptors

    def _collect_fixed_dict_descriptors(
        self,
        type_name: str,
        descriptors: OrderedDict[str, TypeDescriptor],
        seen: set[str],
    ) -> None:
        if not type_name:
            return

        upper = type_name.upper()
        if upper in BUILTIN_TYPE_MAP:
            return

        if type_name in seen:
            return

        descriptor = self._descriptors.get(type_name)
        if descriptor is None:
            return

        seen.add(type_name)

        if descriptor.kind == "alias" and descriptor.target:
            self._collect_fixed_dict_descriptors(descriptor.target, descriptors, seen)
        elif descriptor.kind == "array" and descriptor.item_type:
            self._collect_fixed_dict_descriptors(descriptor.item_type, descriptors, seen)
        elif descriptor.kind == "fixed_dict":
            for field_type in descriptor.fields.values():
                self._collect_fixed_dict_descriptors(field_type, descriptors, seen)
            if type_name not in descriptors:
                descriptors[type_name] = descriptor

        seen.remove(type_name)


class EntityDefParser:
    def __init__(self, scripts_dir: Path, type_registry: TypeRegistry):
        self.scripts_dir = scripts_dir
        self.type_registry = type_registry
        self.entity_defs_dir = scripts_dir / "entity_defs"
        self.interfaces_dir = self.entity_defs_dir / "interfaces"
        self.components_dir = self.entity_defs_dir / "components"
        self._cache: dict[tuple[str, str, bool], DefinitionModel] = {}

    def parse_entities(self) -> list[EntityEntry]:
        entities_path = self.scripts_dir / "entities.xml"
        if not entities_path.exists():
            return self._derive_entities_from_scripts()

        root = ET.parse(entities_path).getroot()
        entries: list[EntityEntry] = []

        for node in root:
            name = node.tag.strip()
            if not name:
                continue

            base_script = self.scripts_dir / "base" / f"{name}.py"
            cell_script = self.scripts_dir / "cell" / f"{name}.py"

            has_base = parse_assertion(node.get("hasBase"), base_script.exists())
            has_cell = parse_assertion(node.get("hasCell"), cell_script.exists())
            has_client = parse_assertion(node.get("hasClient"), False)

            entries.append(
                EntityEntry(
                    name=name,
                    has_base=has_base,
                    has_cell=has_cell,
                    has_client=has_client,
                )
            )

        return entries

    def _derive_entities_from_scripts(self) -> list[EntityEntry]:
        names: set[str] = set()
        base_dir = self.scripts_dir / "base"
        cell_dir = self.scripts_dir / "cell"

        for script_dir in (base_dir, cell_dir):
            if not script_dir.exists():
                continue

            for path in script_dir.glob("*.py"):
                if path.stem == "kbemain":
                    continue
                names.add(path.stem)

        entries: list[EntityEntry] = []
        for name in sorted(names):
            entries.append(
                EntityEntry(
                    name=name,
                    has_base=(base_dir / f"{name}.py").exists(),
                    has_cell=(cell_dir / f"{name}.py").exists(),
                    has_client=False,
                )
            )
        return entries

    def load_entity_model(self, entity_name: str) -> DefinitionModel:
        return self._load_definition(entity_name, category="entity", allow_components=True)

    def load_component_model(self, component_name: str) -> DefinitionModel:
        return self._load_definition(component_name, category="component", allow_components=False)

    def _load_definition(self, name: str, category: str, allow_components: bool) -> DefinitionModel:
        cache_key = (category, name, allow_components)
        cached = self._cache.get(cache_key)
        if cached is not None:
            return clone_definition(cached)

        path = self._definition_path(name, category)
        if not path.exists():
            raise FileNotFoundError(f"未找到定义文件: {path}")

        root = ET.parse(path).getroot()
        model = DefinitionModel()

        self._parse_properties(root.find("Properties"), model)
        self._parse_methods(root.find("BaseMethods"), model.base_methods)
        self._parse_methods(root.find("CellMethods"), model.cell_methods)
        self._parse_methods(root.find("ClientMethods"), model.client_methods)

        interface_allow_components = allow_components and category != "component"
        for interface_name in self._parse_interface_names(root.find("Interfaces")):
            interface_model = self._load_definition(
                interface_name,
                category="interface",
                allow_components=interface_allow_components,
            )
            model.merge_from(interface_model)

        if allow_components:
            for component in self._parse_components(root.find("Components")):
                if component.name not in model.components:
                    model.components[component.name] = component

        parent_name = node_value(root.find("Parent"))
        if parent_name:
            parent_category = "component" if category == "component" else "entity"
            parent_model = self._load_definition(
                parent_name,
                category=parent_category,
                allow_components=allow_components,
            )
            model.merge_from(parent_model)

        self._cache[cache_key] = clone_definition(model)
        return model

    def _definition_path(self, name: str, category: str) -> Path:
        if category == "entity":
            return self.entity_defs_dir / f"{name}.def"
        if category == "component":
            return self.components_dir / f"{name}.def"
        if category == "interface":
            return self.interfaces_dir / f"{name}.def"
        raise ValueError(f"未知定义类型: {category}")

    def _parse_properties(self, node: ET.Element | None, model: DefinitionModel) -> None:
        if node is None:
            return

        for prop_node in node:
            prop_name = prop_node.tag.strip()
            if not prop_name:
                continue

            type_name = node_value(prop_node.find("Type"))
            if not type_name:
                continue

            flags = node_value(prop_node.find("Flags")).upper()
            model.properties.setdefault(
                prop_name,
                PropertyModel(
                    name=prop_name,
                    source_type=type_name,
                    type_expr=self.type_registry.render(type_name),
                    flags=flags,
                ),
            )

    def _parse_methods(self, node: ET.Element | None, target: OrderedDict[str, MethodModel]) -> None:
        if node is None:
            return

        for method_node in node:
            method_name = method_node.tag.strip()
            if not method_name:
                continue

            arg_source_types: list[str] = []
            arg_type_exprs: list[str] = []
            exposed = False
            for child in method_node:
                child_tag = child.tag.strip()
                if child_tag == "Exposed":
                    exposed = True
                    continue

                if child_tag != "Arg":
                    continue

                arg_type = node_value(child)
                arg_source_types.append(arg_type)
                arg_type_exprs.append(self.type_registry.render(arg_type) if arg_type else "Any")

            target.setdefault(
                method_name,
                MethodModel(
                    name=method_name,
                    arg_source_types=arg_source_types,
                    arg_type_exprs=arg_type_exprs,
                    exposed=exposed,
                ),
            )

    def _parse_interface_names(self, node: ET.Element | None) -> list[str]:
        if node is None:
            return []

        names: list[str] = []
        for child in node:
            tag = child.tag.strip().lower()
            if tag in {"interface", "type"}:
                name = node_value(child)
            else:
                name = child.tag.strip()

            if name:
                names.append(name)

        return names

    def _parse_components(self, node: ET.Element | None) -> list[ComponentRef]:
        if node is None:
            return []

        refs: list[ComponentRef] = []
        for child in node:
            component_name = child.tag.strip()
            component_type = node_value(child.find("Type"))
            if not component_name or not component_type:
                continue

            refs.append(ComponentRef(name=component_name, type_name=component_type))

        return refs


def clone_definition(model: DefinitionModel) -> DefinitionModel:
    return DefinitionModel(
        properties=OrderedDict(model.properties),
        base_methods=OrderedDict(model.base_methods),
        cell_methods=OrderedDict(model.cell_methods),
        client_methods=OrderedDict(model.client_methods),
        components=OrderedDict(model.components),
    )


def leading_token(node: ET.Element | None) -> str:
    if node is None:
        return ""

    text = (node.text or "").strip()
    if text:
        return text

    children = list(node)
    if children:
        return children[0].tag.strip()

    return ""


def node_value(node: ET.Element | None) -> str:
    if node is None:
        return ""

    text = (node.text or "").strip()
    if text:
        return text

    children = list(node)
    if children:
        return children[0].tag.strip()

    return ""


def parse_assertion(value: str | None, default: bool) -> bool:
    if value is None:
        return default
    return value.strip().lower() == "true"


def method_signature(method: MethodModel) -> str:
    params = ["self"]
    for index, arg_type in enumerate(method.arg_type_exprs, start=1):
        params.append(f"arg{index}: {arg_type}")
    return f"def {method.name}({', '.join(params)}) -> None: ..."


def render_entity_stub(
    entity_name: str,
    model: DefinitionModel,
    *,
    has_base: bool,
    has_cell: bool,
    has_client: bool,
    is_proxy: bool,
    side: str,
    type_registry: TypeRegistry,
    component_refs: OrderedDict[str, ResolvedComponentRef],
) -> str:
    base_class = "KBEngine.Proxy" if is_proxy and side == "base" else "KBEngine.Entity"
    hook_methods = OrderedDict(BASE_ENTITY_HOOKS if side == "base" else CELL_ENTITY_HOOKS)
    if is_proxy and side == "base":
        hook_methods.update(PROXY_HOOKS)

    selected_properties = filter_properties(model.properties, side=side)
    selected_methods = model.base_methods if side == "base" else model.cell_methods
    typed_dict_blocks = render_typed_dict_blocks(
        type_registry,
        collect_used_source_types(
            selected_properties,
            [
                selected_methods,
                model.base_methods,
                model.cell_methods,
                model.client_methods,
            ],
        ),
    )
    typing_names = ["Any"]
    if typed_dict_blocks:
        typing_names.append("TypedDict")
    local_component_types = collect_component_types(component_refs, side=side)
    if component_refs:
        typing_names.extend(["Literal", "overload"])
    typing_names = list(dict.fromkeys(typing_names))
    remote_call_blocks = render_remote_call_blocks(
        entity_name=entity_name,
        model=model,
        has_base=has_base,
        has_cell=has_cell,
        has_client=has_client,
        component_refs=component_refs,
    )

    import_lines = [
        "# 此文件由 tools/generate_kbengine_entity_stubs.py 自动生成。",
        "from __future__ import annotations",
        "",
        f"from typing import {', '.join(typing_names)}",
        "",
        "import KBEngine",
    ]

    component_imports = build_component_imports(
        component_refs=component_refs,
        include_local_types=local_component_types,
        include_base_call_types=has_base,
        include_cell_call_types=has_cell,
        include_client_call_types=has_client,
    )
    if component_imports:
        import_lines.append("")
        import_lines.extend(component_imports)

    lines = [*import_lines]

    if typed_dict_blocks:
        lines.extend(["", "", *typed_dict_blocks])

    if remote_call_blocks:
        lines.extend(["", "", *remote_call_blocks])

    lines.extend(["", "", f"class {entity_name}({base_class}):"])

    class_lines = render_class_body(
        hook_methods=hook_methods,
        properties=selected_properties,
        methods=selected_methods,
        component_refs=filter_component_refs(component_refs, side=side),
        remote_attrs=build_remote_attrs(
            type_name=entity_name,
            has_base=has_base,
            has_cell=has_cell,
            has_client=has_client,
            local_side=side,
        ),
        extra_methods=build_entity_extra_methods(
            entity_name=entity_name,
            has_client=has_client,
        ),
        side=side,
        decorated_method_blocks=render_get_component_overload_blocks(
            component_refs=component_refs,
            result_mode="local",
            side=side,
        ),
    )
    lines.extend(class_lines)
    lines.append("")
    return "\n".join(lines)


def render_component_stub(
    component_name: str,
    model: DefinitionModel,
    *,
    side: str,
    type_registry: TypeRegistry,
    has_base: bool,
    has_cell: bool,
    has_client: bool,
) -> str:
    selected_properties = filter_properties(model.properties, side=side)
    selected_methods = model.base_methods if side == "base" else model.cell_methods
    typed_dict_blocks = render_typed_dict_blocks(
        type_registry,
        collect_used_source_types(
            selected_properties,
            [
                selected_methods,
                model.base_methods,
                model.cell_methods,
                model.client_methods,
            ],
        ),
    )
    typing_names = ["Any"]
    if typed_dict_blocks:
        typing_names.append("TypedDict")
    remote_call_blocks = render_component_remote_call_blocks(
        component_name=component_name,
        model=model,
        has_base=has_base,
        has_cell=has_cell,
        has_client=has_client,
    )

    lines = [
        "# 此文件由 tools/generate_kbengine_entity_stubs.py 自动生成。",
        "from __future__ import annotations",
        "",
        f"from typing import {', '.join(typing_names)}",
        "",
        "import KBEngine",
    ]

    if typed_dict_blocks:
        lines.extend(["", "", *typed_dict_blocks])

    if remote_call_blocks:
        lines.extend(["", "", *remote_call_blocks])

    if is_component_available_on_side(side=side, has_base=has_base, has_cell=has_cell):
        lines.extend(["", "", f"class {component_name}(KBEngine.EntityComponent):"])

        class_lines = render_class_body(
            hook_methods=OrderedDict(COMPONENT_HOOKS),
            properties=selected_properties,
            methods=selected_methods,
            component_refs=OrderedDict(),
            remote_attrs=build_remote_attrs(
                type_name=component_name,
                has_base=has_base,
                has_cell=has_cell,
                has_client=has_client,
                local_side=side,
            ),
            extra_methods=build_component_extra_methods(
                component_name=component_name,
                has_client=has_client,
            ),
            side=side,
        )
        lines.extend(class_lines)
    lines.append("")
    return "\n".join(lines)


def build_component_imports(
    *,
    component_refs: OrderedDict[str, ResolvedComponentRef],
    include_local_types: set[str],
    include_base_call_types: bool,
    include_cell_call_types: bool,
    include_client_call_types: bool,
) -> list[str]:
    imports: list[str] = []
    seen: dict[str, list[str]] = {}
    for component in component_refs.values():
        names = seen.setdefault(component.type_name, [])
        if component.type_name in include_local_types:
            names.append(component.type_name)
        if include_base_call_types and component.has_base:
            names.append(f"{component.type_name}BaseCall")
        if include_cell_call_types and component.has_cell:
            names.append(f"{component.type_name}CellCall")
        if include_client_call_types and component.has_client:
            names.append(f"{component.type_name}ClientCall")

    for type_name, names in seen.items():
        if not names:
            continue
        unique_names = list(dict.fromkeys(names))
        imports.append(f"from components.{type_name} import {', '.join(unique_names)}")
    return imports


def filter_properties(
    properties: OrderedDict[str, PropertyModel],
    *,
    side: str,
) -> OrderedDict[str, PropertyModel]:
    visible = BASE_VISIBLE_FLAGS if side == "base" else CELL_VISIBLE_FLAGS
    selected: OrderedDict[str, PropertyModel] = OrderedDict()
    for name, prop in properties.items():
        if prop.flags in visible:
            selected[name] = prop
    return selected


def collect_used_source_types(
    properties: OrderedDict[str, PropertyModel],
    method_groups: list[OrderedDict[str, MethodModel]],
) -> list[str]:
    used: list[str] = []
    for property_model in properties.values():
        used.append(property_model.source_type)
    for methods in method_groups:
        for method in methods.values():
            used.extend(method.arg_source_types)
    return used


def render_typed_dict_blocks(type_registry: TypeRegistry, type_names: list[str]) -> list[str]:
    descriptors = type_registry.collect_fixed_dict_descriptors(type_names)
    if not descriptors:
        return []

    lines: list[str] = []
    for alias_name, descriptor in descriptors.items():
        lines.append(f"class {alias_name}(TypedDict):")
        if descriptor.fields:
            for field_name, field_type in descriptor.fields.items():
                lines.append(f"    {field_name}: {type_registry.render(field_type)}")
        else:
            lines.append("    pass")
        lines.append("")

    if lines[-1] == "":
        lines.pop()

    return lines


def render_class_body(
    *,
    hook_methods: OrderedDict[str, str],
    properties: OrderedDict[str, PropertyModel],
    methods: OrderedDict[str, MethodModel],
    component_refs: OrderedDict[str, ResolvedComponentRef],
    remote_attrs: OrderedDict[str, str],
    extra_methods: OrderedDict[str, str],
    side: str,
    decorated_method_blocks: list[list[str]] | None = None,
) -> list[str]:
    lines: list[str] = []
    emitted_names: set[str] = set()

    for property_model in properties.values():
        lines.append(f"    {property_model.name}: {property_model.type_expr}")
        emitted_names.add(property_model.name)

    for component in component_refs.values():
        lines.append(f"    {component.name}: {component.type_name}")
        emitted_names.add(component.name)

    for attr_name, attr_type in remote_attrs.items():
        lines.append(f"    {attr_name}: {attr_type}")
        emitted_names.add(attr_name)

    if properties or component_refs or remote_attrs:
        lines.append("")

    for name, signature in hook_methods.items():
        lines.append(f"    {signature}")
        emitted_names.add(name)

    if decorated_method_blocks:
        for block in decorated_method_blocks:
            if lines and lines[-1] != "":
                lines.append("")
            for block_line in block:
                lines.append(f"    {block_line}")
        emitted_names.add("getComponent")

    for name, signature in extra_methods.items():
        if name in emitted_names:
            continue
        lines.append(f"    {signature}")
        emitted_names.add(name)

    for method in methods.values():
        if method.name in emitted_names:
            continue
        lines.append(f"    {method_signature(method)}")
        emitted_names.add(method.name)

    if not lines:
        lines.append("    ...")

    return lines


def build_remote_attrs(
    *,
    type_name: str,
    has_base: bool,
    has_cell: bool,
    has_client: bool,
    local_side: str,
) -> OrderedDict[str, str]:
    attrs: OrderedDict[str, str] = OrderedDict()
    if has_client:
        attrs["client"] = f"{type_name}ClientCall | None"
    if local_side == "base" and has_cell:
        attrs["cell"] = f"{type_name}CellCall | None"
    if local_side == "cell" and has_base:
        attrs["base"] = f"{type_name}BaseCall | None"
    return attrs


def build_entity_extra_methods(
    *,
    entity_name: str,
    has_client: bool,
) -> OrderedDict[str, str]:
    methods: OrderedDict[str, str] = OrderedDict()
    if has_client:
        methods["clientEntity"] = f"def clientEntity(self, destID: int) -> {entity_name}ClientCall: ..."
    return methods


def build_component_extra_methods(
    *,
    component_name: str,
    has_client: bool,
) -> OrderedDict[str, str]:
    methods: OrderedDict[str, str] = OrderedDict()
    if has_client:
        methods["clientEntity"] = f"def clientEntity(self, destID: int) -> {component_name}ClientCall: ..."
    return methods


def render_remote_call_blocks(
    *,
    entity_name: str,
    model: DefinitionModel,
    has_base: bool,
    has_cell: bool,
    has_client: bool,
    component_refs: OrderedDict[str, ResolvedComponentRef],
) -> list[str]:
    blocks: list[str] = []
    specs: list[tuple[bool, str, str, OrderedDict[str, MethodModel], str]] = [
        (has_base, f"{entity_name}BaseCall", "KBEngine.BaseEntityCall", model.base_methods, "base"),
        (has_cell, f"{entity_name}CellCall", "KBEngine.CellEntityCall", model.cell_methods, "cell"),
        (has_client, f"{entity_name}ClientCall", "KBEngine.ClientEntityCall", model.client_methods, "client"),
    ]

    for enabled, class_name, base_class, methods, remote_side in specs:
        if not enabled:
            continue

        blocks.append(f"class {class_name}({base_class}):")
        class_lines = render_remote_call_class_body(
            methods=methods,
            component_refs=filter_component_refs(component_refs, side=remote_side),
            type_suffix=remote_side,
        )
        if class_lines:
            blocks.extend(class_lines)
        else:
            blocks.append("    ...")
        blocks.append("")

    if blocks and blocks[-1] == "":
        blocks.pop()

    return blocks


def render_component_remote_call_blocks(
    *,
    component_name: str,
    model: DefinitionModel,
    has_base: bool,
    has_cell: bool,
    has_client: bool,
) -> list[str]:
    blocks: list[str] = []
    specs: list[tuple[bool, str, str, OrderedDict[str, MethodModel]]] = [
        (has_base, f"{component_name}BaseCall", "KBEngine.EntityComponentCall", model.base_methods),
        (has_cell, f"{component_name}CellCall", "KBEngine.EntityComponentCall", model.cell_methods),
        (has_client, f"{component_name}ClientCall", "KBEngine.EntityComponentCall", model.client_methods),
    ]

    for enabled, class_name, base_class, methods in specs:
        if not enabled:
            continue

        blocks.append(f"class {class_name}({base_class}):")
        if methods:
            for method in methods.values():
                blocks.append(f"    {method_signature(method)}")
        else:
            blocks.append("    ...")
        blocks.append("")

    if blocks and blocks[-1] == "":
        blocks.pop()

    return blocks


def render_remote_call_class_body(
    *,
    methods: OrderedDict[str, MethodModel],
    component_refs: OrderedDict[str, ResolvedComponentRef],
    type_suffix: str,
) -> list[str]:
    lines: list[str] = []

    for component in component_refs.values():
        lines.append(f"    {component.name}: {component.type_name}{type_suffix.capitalize()}Call")

    overload_blocks = render_get_component_overload_blocks(
        component_refs=component_refs,
        result_mode="remote",
        side=type_suffix,
    )
    if lines and overload_blocks:
        lines.append("")

    for block in overload_blocks:
        for block_line in block:
            lines.append(f"    {block_line}")

    if (lines and methods) or (overload_blocks and methods):
        lines.append("")

    for method in methods.values():
        lines.append(f"    {method_signature(method)}")

    return lines


def render_get_component_overload_blocks(
    *,
    component_refs: OrderedDict[str, ResolvedComponentRef],
    result_mode: str,
    side: str,
) -> list[list[str]]:
    component_types = sorted(collect_component_types(component_refs, side=side))
    if not component_types:
        return []

    fallback_single = "KBEngine.EntityComponent | None"
    fallback_multi = "tuple[KBEngine.EntityComponent, ...]"
    if result_mode == "remote":
        fallback_single = "KBEngine.EntityComponentCall | None"
        fallback_multi = "tuple[KBEngine.EntityComponentCall, ...]"

    blocks: list[list[str]] = []
    for type_name in component_types:
        return_type = type_name if result_mode == "local" else f"{type_name}{side.capitalize()}Call"
        blocks.append(
            [
                "@overload",
                f'def getComponent(self, componentName: Literal["{type_name}"], all: Literal[False] = False) -> {return_type} | None: ...',
                "@overload",
                f'def getComponent(self, componentName: Literal["{type_name}"], all: Literal[True]) -> tuple[{return_type}, ...]: ...',
            ]
        )

    blocks.append(
        [
            "@overload",
            f"def getComponent(self, componentName: str, all: Literal[False] = False) -> {fallback_single}: ...",
            "@overload",
            f"def getComponent(self, componentName: str, all: Literal[True]) -> {fallback_multi}: ...",
        ]
    )
    return blocks


def collect_component_types(
    component_refs: OrderedDict[str, ResolvedComponentRef],
    *,
    side: str,
) -> set[str]:
    return {component.type_name for component in filter_component_refs(component_refs, side=side).values()}


def filter_component_refs(
    component_refs: OrderedDict[str, ResolvedComponentRef],
    *,
    side: str,
) -> OrderedDict[str, ResolvedComponentRef]:
    selected: OrderedDict[str, ResolvedComponentRef] = OrderedDict()
    for name, component in component_refs.items():
        available = False
        if side == "base":
            available = component.has_base
        elif side == "cell":
            available = component.has_cell
        elif side == "client":
            available = component.has_client

        if available:
            selected[name] = component
    return selected


def is_component_available_on_side(*, side: str, has_base: bool, has_cell: bool) -> bool:
    if side == "base":
        return has_base
    if side == "cell":
        return has_cell
    raise ValueError(f"未知组件本地侧: {side}")


def infer_component_presence(scripts_dir: Path, component_name: str, model: DefinitionModel) -> tuple[bool, bool, bool]:
    has_base = (scripts_dir / "base" / "components" / f"{component_name}.py").exists()
    has_cell = (scripts_dir / "cell" / "components" / f"{component_name}.py").exists()

    if not has_base:
        has_base = bool(filter_properties(model.properties, side="base")) or bool(model.base_methods)
    if not has_cell:
        has_cell = bool(filter_properties(model.properties, side="cell")) or bool(model.cell_methods)

    has_client_property = any(prop.flags in CLIENT_VISIBLE_FLAGS for prop in model.properties.values())
    has_client_method = bool(model.client_methods)
    has_exposed_server_method = (
        (has_base and any(method.exposed for method in model.base_methods.values()))
        or (has_cell and any(method.exposed for method in model.cell_methods.values()))
    )
    has_client = has_client_property or has_client_method or has_exposed_server_method
    return has_base, has_cell, has_client


def generate_outputs(scripts_dir: Path) -> dict[Path, str]:
    type_registry = TypeRegistry(scripts_dir / "entity_defs" / "types.xml")
    type_registry.load()

    parser = EntityDefParser(scripts_dir, type_registry)
    outputs: dict[Path, str] = {}
    component_outputs: dict[tuple[str, str], str] = {}
    component_cache: dict[str, tuple[DefinitionModel, tuple[bool, bool, bool]]] = {}

    def resolve_component(component_type: str) -> tuple[DefinitionModel, tuple[bool, bool, bool]]:
        cached = component_cache.get(component_type)
        if cached is not None:
            return cached

        component_model = parser.load_component_model(component_type)
        presence = infer_component_presence(scripts_dir, component_type, component_model)
        component_cache[component_type] = (component_model, presence)
        return component_model, presence

    for entity in parser.parse_entities():
        model = parser.load_entity_model(entity.name)
        resolved_components: OrderedDict[str, ResolvedComponentRef] = OrderedDict()

        for component in model.components.values():
            component_model, presence = resolve_component(component.type_name)
            has_base_component, has_cell_component, has_client_component = presence
            resolved_components[component.name] = ResolvedComponentRef(
                name=component.name,
                type_name=component.type_name,
                has_base=has_base_component,
                has_cell=has_cell_component,
                has_client=has_client_component,
            )

            if entity.has_base:
                key = ("base", component.type_name)
                if key not in component_outputs:
                    component_outputs[key] = render_component_stub(
                        component.type_name,
                        component_model,
                        side="base",
                        type_registry=type_registry,
                        has_base=has_base_component,
                        has_cell=has_cell_component,
                        has_client=has_client_component,
                    )

            if entity.has_cell:
                key = ("cell", component.type_name)
                if key not in component_outputs:
                    component_outputs[key] = render_component_stub(
                        component.type_name,
                        component_model,
                        side="cell",
                        type_registry=type_registry,
                        has_base=has_base_component,
                        has_cell=has_cell_component,
                        has_client=has_client_component,
                    )

        if entity.has_base and (scripts_dir / "base" / f"{entity.name}.py").exists():
            outputs[scripts_dir / "base" / f"{entity.name}.pyi"] = render_entity_stub(
                entity.name,
                model,
                has_base=entity.has_base,
                has_cell=entity.has_cell,
                has_client=entity.has_client,
                is_proxy=entity.has_client,
                side="base",
                type_registry=type_registry,
                component_refs=resolved_components,
            )

        if entity.has_cell and (scripts_dir / "cell" / f"{entity.name}.py").exists():
            outputs[scripts_dir / "cell" / f"{entity.name}.pyi"] = render_entity_stub(
                entity.name,
                model,
                has_base=entity.has_base,
                has_cell=entity.has_cell,
                has_client=entity.has_client,
                is_proxy=False,
                side="cell",
                type_registry=type_registry,
                component_refs=resolved_components,
            )

    for (side, component_name), content in component_outputs.items():
        outputs[scripts_dir / side / "components" / f"{component_name}.pyi"] = content

    return outputs


def write_outputs(outputs: dict[Path, str], *, check: bool) -> int:
    changed_paths: list[Path] = []

    for path, content in sorted(outputs.items()):
        existing = path.read_text(encoding="utf-8") if path.exists() else None
        if existing == content:
            continue

        changed_paths.append(path)
        if not check:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(content, encoding="utf-8", newline="\n")

    if not changed_paths:
        print("没有需要更新的实体类型桩。")
        return 0

    action = "将更新" if check else "已更新"
    for path in changed_paths:
        print(f"{action}: {path}")

    return 1 if check else 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="根据 KBEngine 的 entities.xml / entity_defs 生成旁路 .pyi 类型桩。"
    )
    parser.add_argument(
        "--scripts-dir",
        type=Path,
        default=ROOT / "kbe" / "res" / "sdk_templates" / "server" / "python_assets" / "scripts",
        help="scripts 目录路径。",
    )
    parser.add_argument(
        "--mode",
        choices=["adjacent"],
        default="adjacent",
        help="输出模式。第一版只支持 adjacent。",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="只检查是否有未生成或过时的 .pyi，不写文件。",
    )
    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()

    scripts_dir = args.scripts_dir.resolve()
    if not scripts_dir.exists():
        print(f"未找到 scripts 目录: {scripts_dir}", file=sys.stderr)
        return 2

    try:
        outputs = generate_outputs(scripts_dir)
    except Exception as exc:
        print(f"生成实体类型桩失败: {exc}", file=sys.stderr)
        return 1

    return write_outputs(outputs, check=args.check)


if __name__ == "__main__":
    raise SystemExit(main())
