# BigWorld Process Model

> This page explains why a BigWorld-style server is split into several long-running component processes.

## Topics to cover

- The basic responsibilities of Login / Base / Cell / DB
- The control and load-coordination role of the Manager processes
- Typical message flow between components
- Why spatial logic and non-spatial logic are separated
- Why the player proxy usually lives on the Base side

## Value for studying KBEngine

- The goal is not to restate BigWorld history, but to provide a frame for understanding the multi-process structure in the KBEngine source.