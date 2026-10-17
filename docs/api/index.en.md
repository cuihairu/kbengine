> **English** | [中文](index.md)

# API Overview

> This is a reference-desk entry, not the learning track. To build systematic understanding, go back to [/study/](/study/); to read concrete implementations, go back to [/architecture/source-analysis/](/architecture/source-analysis/).

## Engine APIs

- [KBEngine API](/api/kbengine/) — Chinese only for now, still generated from the CHM source
- English access note: keep `api/kbengine/**` as the Chinese authoritative pages; add future `*.en.md` counterparts and expose them through this directory-level index.
- [BigWorld API](/api/bigworld/) — English available

## Source boundary

- The KBEngine API comes from the Chinese API help CHM `docs/api/kbengine/kbengine_api(cn).chm`.
- The BigWorld API is currently derived from the `/*~ ... */` script documentation comments and Python binding points in the `BigWorld-Engine-14.4.1` source. It is a "source-extracted API", not a migration of the CHM text.
- The KBEngine API original file is the repository file `docs/api/kbengine/kbengine_api(cn).chm` (Chinese only), which is not published to the documentation site.

New references must use `/api/kbengine/**` or `/api/bigworld/**`; the old root-level KBEngine API entry is no longer kept.

## Verification and source analysis

- [KBEngine API classification and audit checklist](/api/kbengine/audit-checklist.md)
- [Source analysis topics](/architecture/source-analysis/)
