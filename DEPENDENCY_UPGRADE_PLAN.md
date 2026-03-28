# KBEngine Dependency Upgrade Plan

Last updated: 2026-03-28

This document summarizes the current third-party dependency status in this repository, with these goals:

- keep the project compatible with `Python 3.12.13`
- prefer dependencies that work well with `C++17`
- reduce very old vendored libraries where practical

## Baseline

- Embedded Python version: `3.12.13`
- Current focus standard: `C++17`
- Windows/OpenSSL build path has already been adapted to OpenSSL 3 based binaries

## Already Handled

### fmt

- Local version: `12.1.0`
- Status: already upgraded
- Upstream: <https://github.com/fmtlib/fmt>
- Latest checked upstream release: `12.1.0`

### OpenSSL

- Status: build path already switched to OpenSSL 3 for Windows/CI
- Notes:
  - merged work removed old `libeay32/ssleay32` project dependencies
  - CI copies OpenSSL 3 headers/libs/dlls into the build inputs
  - old `vsopenssl` 1.0.2e files still remain in the tree as historical residue
- Upstream: <https://github.com/openssl/openssl>
- Latest checked upstream release: `3.6.1`

## Recommended Direct Upgrades

These are the most reasonable candidates for direct upgrade while staying aligned with `Python 3.12.13` and `C++17`.

### zlib

- Local version: `1.2.11`
- Latest checked upstream release: `1.3.2`
- Upstream: <https://github.com/madler/zlib>
- Priority: high
- Reason:
  - low-level, widely used, not modern enough in current tree
  - directly relevant to Python build/runtime stack
  - usually lower migration risk than larger framework dependencies

### expat

- Local version: `2.1.0`
- Latest checked upstream release: `2.7.5`
- Upstream: <https://github.com/libexpat/libexpat>
- Priority: high
- Reason:
  - current version is very old
  - relevant to XML parsing and Python `pyexpat`
  - should be modernized early

### APR

- Local version: `1.5.1`
- Latest checked upstream release: `1.7.6`
- Upstream: <https://github.com/apache/apr>
- Priority: medium-high
- Reason:
  - old but still on the same major line
  - needed by `log4cxx`

### APR-util

- Local version: `1.5.4`
- Latest checked upstream release: `1.6.3`
- Upstream: <https://github.com/apache/apr-util>
- Priority: medium-high
- Reason:
  - typically should move together with APR
  - dependency chain for `log4cxx`

### curl

- Local version: `7.61.1`
- Latest checked upstream release: `8.19.0`
- Upstream: <https://github.com/curl/curl>
- Priority: medium
- Reason:
  - current version is old
  - upgrade is worthwhile, but version gap is large
  - Windows project integration and OpenSSL combination need extra verification

### hiredis

- Local version: `0.13.3`
- Latest checked upstream release: `1.3.0`
- Upstream: <https://github.com/redis/hiredis>
- Priority: medium
- Reason:
  - current version is old
  - likely manageable, but needs API compatibility review

## Migration Candidates, Not Simple Upgrades

These are not good "just bump the version" tasks.

### tinyxml -> tinyxml2

- Current library: `tinyxml 2.6.2`
- Suggested replacement: `tinyxml2`
- Latest checked upstream release: `11.0.0`
- Upstream: <https://github.com/leethomason/tinyxml2>
- Priority: high technical debt, but separate project
- Reason:
  - current code uses the old `TiXml*` API widely
  - `tinyxml2` is a different API family, so this is a migration
  - impact spans `xml`, `entitydef`, `serverconfig`, `network`, tools, and client config parsing
- Suggested approach:
  - keep `lib/xml` as the compatibility boundary
  - refactor wrapper interfaces first
  - replace backend implementation in a dedicated migration branch

### log4cxx

- Local version: `0.10.0`
- Latest checked upstream release line observed: `1.6.1`
- Upstream: <https://github.com/apache/logging-log4cxx>
- Priority: medium
- Reason:
  - current vendored version is old
  - upgrade should be grouped with `APR`, `APR-util`, and `expat`
  - not ideal as an isolated dependency bump

## High-Risk or Low-Value-for-Now Dependencies

These look too legacy or too invasive to be immediate upgrade targets.

### g3dlite

- Local version marker: `G3D_VER 61000`
- Status: very old codebase
- Recommendation: do not treat as a simple version bump

### jwsmtp

- Local version: `1.32`
- Status: very old project
- Recommendation: replacement is more realistic than upgrade

### sigar

- Status: legacy dependency with weak modern upgrade value
- Recommendation: defer unless there is a concrete runtime need

### MySQL embedded/client headers

- Local version marker: `5.7.10`
- Status: likely too invasive for a straightforward dependency refresh
- Recommendation: defer unless database integration work already justifies it

## Suggested Upgrade Order

1. `zlib`
2. `expat`
3. `APR`
4. `APR-util`
5. `log4cxx`
6. `curl`
7. `hiredis`
8. dedicated migration: `tinyxml -> tinyxml2`
9. later evaluation: `g3dlite`, `jwsmtp`, `sigar`, `MySQL`

## Notes

- `Python 3.12.13` compatibility matters most for `OpenSSL`, `zlib`, and `expat`.
- OpenSSL should be considered "already moved onto the modern path", but repository cleanup is still incomplete.
- `tinyxml` is the main XML parsing debt in the project, while `expat` is a separate dependency line.
- There is currently an untracked local directory at `kbe/src/lib/dependencies/openssl/`; this document does not rely on that untracked directory as committed project state.
