# API Overview

> This is a reference-desk entry, not the learning track. To build systematic understanding, go back to [/study/](); to read concrete implementations, go back to /architecture/source-analysis/.

## Engine APIs

- English access note: `zh/api/kbengine/**` remains the Chinese authoritative source; the counterpart set now starts at [KBEngine API (English)](zh/api/kbengine/).

## Source boundary

- The KBEngine API comes from the Chinese API help CHM `docs/api/kbengine/kbengine_api(cn).chm`.
- The BigWorld API is currently derived from the `/*~ ... */` script documentation comments and Python binding points in the `BigWorld-Engine-14.4.1` source. It is a "source-extracted API", not a migration of the CHM text.
- The KBEngine API original file is the repository file `docs/api/kbengine/kbengine_api(cn).chm` (Chinese only), which is not published to the documentation site.

New references must use `/api/kbengine/**` or `/api/bigworld/**`; the old root-level KBEngine API entry is no longer kept.

## Verification and source analysis

- [KBEngine API classification and audit checklist](kbengine/audit-checklist.md
