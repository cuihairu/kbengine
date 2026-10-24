# Architecture Overview

> This page is only a system map; it no longer carries structural design proposals. It answers "where background, deep dives, and comparison belong", not "how the site directories should be renamed".

## System map

```mermaid
flowchart TD
    A["study/**\nMain learning path\nProgressive topics build the whole picture"] --> B["architecture/\nArchitecture overview and system map"]
    A --> C["source-analysis/**\nSource deep dives\nDrill further into concrete questions"]
    A --> D["api/kbengine/**\nKBEngine interface contracts\nKept consistent with the CHM"]
    A --> G["api/bigworld/**\nSource-extracted BigWorld API"]
    B --> E["bigworld/**\nBackground concepts and design origins"]
    B --> F["comparison/**\nBigWorld / KBEngine comparison"]
    C --> D
    C --> G
    C --> F

    style A fill:#f5f1e8,stroke:#8a6d3b,color:#3b2d1f
    style B fill:#e8f1f5,stroke:#3b6d8a,color:#173042
    style C fill:#edf5e8,stroke:#5a8a3b,color:#203817
    style D fill:#f4f4f4,stroke:#666,color:#222
    style E fill:#f8efe3,stroke:#9a6a2f,color:#4a2f12
    style F fill:#efe8f5,stroke:#6b4a8c,color:#2e1b45
    style G fill:#f8efe3,stroke:#9a6a2f,color:#4a2f12
```

## Directory roles

- `BigWorld` builds the conceptual model and answers "why is the architecture designed this way".
- `Source deep dives` chase concrete questions and answer "how is it actually implemented in the current code".
- `Comparison` answers "which ideas came from BigWorld, and where KBEngine turned them into its own implementation".
- `Redesign proposal` is an appendix-style engineering draft, not a main line for source analysis.

This is not a second learning track. For ordered systematic study, still enter [/study/](/study/) first; this area provides the background map, deep-dive entries, and the comparison.

## Recommended reading order

4. [KBEngine 2.0 redesign proposal](redesign.md)

## Current authoring rules

- `architecture/` owns the system map, directory boundaries, and reading routes; it does not expand problem-driven source detail.
- `source-analysis` owns concrete problem study, with the body organized around source paths, call chains, and key classes.
- The `bigworld` directory provides conceptual background and does not replace reading the KBEngine code itself.
- The `comparison` directory records only relationships that have evidence behind them, not assumed "inheritance".
- `redesign` is explicitly an engineering draft, kept separate from source-material transcription.

## Material boundary

- The current `architecture` directory is not a page-by-page transcription of the CHM/PDF material.
- Conclusions in this directory should be based primarily on the repository source, with BigWorld used only as a reference frame.