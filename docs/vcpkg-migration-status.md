# vcpkg Migration Status

This document tracks third-party dependencies in `kbe/src/lib/dependencies` and whether they have been replaced by package-managed dependencies in the CMake build.

## Already package-managed

These dependencies are now resolved through `vcpkg.json` and `cmake/KBEngineDependencies.cmake` in vcpkg mode:

- `apr`
- `apr-util`
- `curl`
- `fmt`
- `hiredis`
- `jemalloc`
- `libmariadb`
- `log4cxx`
- `openssl`
- `python3`
- `tinyxml2`
- `utf8cpp` via the `utfcpp` package
- `zlib`

`jwsmtp` has been replaced at the application layer by libcurl's SMTP support, so it is no longer part of the active dependency graph.

Notes:

- `tinyxml` is no longer the active XML dependency. The codebase builds against `tinyxml2`.
- `log4cxx` requires `EXPAT` to be discoverable before `find_package(log4cxx CONFIG REQUIRED)` in vcpkg mode.

## Remaining vendored dependencies that still participate in the build

These are still built from source inside the repository as of this branch:

### `g3dlite`

- Build integration:
  - `kbe/src/lib/CMakeLists.txt` builds `kbe_math` from vendored `g3dlite` sources.
- Reason still vendored:
  - No matching port was found under the local vcpkg `ports/` tree.
- Upstream/source:
  - G3D project: <https://casual-effects.com/g3d>
  - GitHub mirror/project page commonly referenced for G3D: <https://github.com/RomkoSI/G3D>

### `tmxparser`

- Build integration:
  - `cmake/KBEngineDependencies.cmake` builds `kbe_dependency_tmxparser` from vendored sources.
  - `kbe/src/lib/navigation/navigation_tile_handle.h` includes `tmxparser/Tmx.h`.
- Reason still vendored:
  - No matching port was found under the local vcpkg `ports/` tree.
- Upstream/source:
  - GitHub: <https://github.com/sainteos/tmxparser>
  - Docs: <https://sainteos.github.io/tmxparser-docs/>

## Vendored directories currently not driving the active CMake build

These directories still exist in-tree, but they are not the next migration priority because the current CMake build is not using them directly:

- `apr`
- `apr-util`
- `curl`
- `expat`
- `jemalloc`
- `jwsmtp`
- `log4cxx`
- `tinyxml`
- `utf8cpp`
- `vsopenssl`
- `zlib`

These can be cleaned up later after the branch is stabilized and any platform-specific fallback logic is intentionally removed.

## Additional legacy directories to evaluate separately

- `sigar`
  - Upstream/source: <https://github.com/hyperic/sigar>
  - No matching local vcpkg port was found.
  - The current lib/server CMake flow does not appear to include it directly in the active build graph.

- `vld`
  - Windows-only leak detection helper; not active in the current macOS build.

- `openssl`
  - There is also an untracked vendored checkout under `kbe/src/lib/dependencies/openssl/`.
  - The active build is already using package-provided OpenSSL targets.

## Recommended next step

If the goal is to keep pushing package management coverage with low risk, the next practical work items are:

1. Evaluate whether `tmxparser` should stay vendored or be replaced by a maintained external library integration.
2. Decide whether `jwsmtp` should be replaced with a more maintainable SMTP client instead of looking for a nonexistent vcpkg port.
3. Clean unused vendored directories only after confirming no remaining non-vcpkg platform path depends on them.
