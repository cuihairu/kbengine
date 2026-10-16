> **English** | [中文](concepts.md)

# BigWorld Core Concepts

> This page defines the core objects and terms that keep coming up while studying BigWorld and KBEngine.

## Topics to cover

- Entity: logical entity, Base entity, Cell entity, and the client-side representation
- Space: scene container, space ownership, and cross-space migration
- Cell: space partitioning, load splitting, and cross-Cell synchronization
- Witness / View: vision, visible set, and client synchronization
- Ghost: cross-boundary copies and cross-Cell observation
- Manager components: coordination, load balancing, and process management

## How to write it

- Define each term by "responsibility / which process hosts it / how it relates to other objects".
- If KBEngine has a same-named or similar implementation, build the mapping later in `comparison/terminology.md`.