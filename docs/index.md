---
layout: home
hero:
  image:
    src: /logo.png
    alt: KBEngine logo
  text: KBEngine Docs (English)
  tagline: A single ordered reading track, supplemented by source-code deep dives, background comparison, API lookup, and doc notes
  actions:
    - theme: brand
      text: Start reading
      link: /study/
    - theme: alt
      text: Deep dives
      link: /architecture/source-analysis/
    - theme: alt
      text: API reference
      link: /api/
features:
  - title: Learning track
    details: "study/** is the only ordered main entry. On a first pass, follow the 24 chapters through to build a complete system map."
    link: /study/
  - title: Deep dives
    details: "architecture/source-analysis/** drills specific questions down to files, classes, functions, and call chains; it is not a second body of text."
    link: /architecture/source-analysis/
  - title: Background & comparison
    details: "architecture/** covers the BigWorld background, the system maps, and a diff between the two engines, answering 'why it is designed this way'."
    link: /architecture/
  - title: API reference
    details: "api/** holds interface contracts and quick lookup only. Implementation and architecture explanations go back to the deep dives and the learning track."
    link: /api/
  - title: Doc notes
    details: "resources/** records source boundaries, verification status, and structure design, answering 'is this page from original material, and how should it be maintained'."
    link: /resources/
footer: Docs are built from material already in this repository; continuously updated
---

## Process architecture

The KBEngine MMOG engine runs as a set of cooperative processes that scale horizontally through dynamic load balancing. The player path runs `Client → LoginApp → BaseApp → CellApp → DBMgr`: LoginApp handles access and authentication, BaseApp holds long-lived player logic and the client session, CellApp performs real-time space simulation, and DBMgr centralizes persistence to MySQL / Redis. Peripheral processes (Machine, BaseAppMgr, CellAppMgr, Logger, Interfaces, Bots) provide service registration, scheduling, log aggregation, external integration, and load testing. Components and data flow are derived from `kbe/src/server`.

![KBEngine multi-process architecture: player path Client → LoginApp → BaseApp → CellApp → DBMgr, with registry, schedulers, logger, interfaces and load-test bots](/images/architecture-en.svg)

[Open full SVG](/images/architecture-en.svg) · [Architecture overview](/architecture/) · [System map](/architecture/)

