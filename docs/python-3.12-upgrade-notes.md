# Python 3.12 Upgrade Notes

## Current baseline

- KBEngine vendors CPython directly under `kbe/src/lib/python`.
- The current vendored version is `3.7.3`.
- Unix build copies a generated static archive into `kbe/src/libs/libpython.a`.
- Windows projects link directly against `python37.lib` or `python37_d.lib`.

## Why target 3.12

- Python `3.12` is in the bugfix phase on the Python developer guide support table.
- Python `3.14` is still in the feature phase, so it is less conservative.
- Python `3.13` is viable, but `3.12` is the safer first landing point for a large embedded-runtime upgrade.

Official reference: <https://devguide.python.org/versions/>

## Confirmed upgrade blockers

- `kbe/src/lib/Makefile` assumed the static archive name was `libpython3.7m.a`.
- `kbe/src/lib/pyscript/script.cpp` still calls `PyEval_InitThreads()`, which is removed in modern CPython.
- Windows project files hardcode `python37.lib` and `python37_d.lib`.
- The vendored CPython tree contains Windows import-forwarding files under `kbe/src/lib/python/PC/` that also encode the `python37` DLL name.

## First-pass migration plan

1. Replace the vendored `kbe/src/lib/python` tree with CPython `3.12.x`.
2. Re-apply the KBEngine-specific modifications called out in `kbe/src/lib/python/Makefile`.
3. Update Unix build glue to consume the new CPython archive and generated extension layout.
4. Update all Windows `vcxproj` references from `python37` to the `3.12` library names.
5. Audit KBEngine C and C++ embedding code for removed or behavior-changed Python C API calls.
6. Rebuild server targets on Linux first, then fix Windows project integration.

## Known files to revisit next

- `kbe/src/lib/pyscript/script.cpp`
- `kbe/src/lib/python/PC/pyconfig.h`
- `kbe/src/lib/python/PC/python3.def`
- `kbe/src/server/baseapp/baseapp.vcxproj`
- `kbe/src/server/cellapp/cellapp.vcxproj`
- `kbe/src/server/dbmgr/dbmgr.vcxproj`
- `kbe/src/server/loginapp/loginapp.vcxproj`
- `kbe/src/server/tools/bots/bots.vcxproj`
- `kbe/src/server/tools/guiconsole/guiconsole.vcxproj`
- `kbe/src/server/tools/interfaces/interfaces.vcxproj`
- `kbe/src/server/tools/kbcmd/kbcmd.vcxproj`
- `kbe/src/server/tools/logger/logger.vcxproj`
