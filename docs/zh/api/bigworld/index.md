# BigWorld API

> This directory organizes the scripting API of `BigWorld-Engine-14.4.1`. The repository has no BigWorld API package equivalent to the KBEngine CHM, so this is a "source-extracted API" entry that will be filled in from source comments and Python binding points over time.

## Source boundary

The priority sources for the BigWorld API are:

- `/*~ module ... */`, `/*~ class ... */`, `/*~ function ... */`, and `/*~ attribute ... */` comments under `BigWorld-Engine-14.4.1/programming/bigworld/server/**`
- Python binding macros such as `PY_AUTO_MODULE_FUNCTION`, `PY_METHOD`, `PY_ATTRIBUTE`, and `PY_RO_ATTRIBUTE_DECLARE`
- The server whitepaper, installation guide, and build guide under `BigWorld-Engine-14.4.1/docs/pdf/**` are background material only, not per-item API source text

## Module entries

- [BigWorld module](bigworld-module.md
- [Base entity](base.md
- [Cell Entity](entity.md
- [BWPersonality callbacks](bwpersonality.md

## How to read it

BigWorld and KBEngine share many concepts, such as Base/Cell, Mailbox/EntityCall, timers, SpaceData, and `reloadScript`. Do not carry KBEngine component names and semantics over directly while reading the BigWorld API; check the following at the same time:

- [BigWorld / KBEngine comparison](study/23-bigworld-and-kbengine-comparison.md
- [Hotfix](study/21-hotupdate-fault-tolerance-and-ops.md
- [Scripting source analysis](architecture/source-analysis/scripting.md