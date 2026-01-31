# Shell Component

The Shell component coordinates the core UI pieces of PikselDesktop and provides a central place for shared signals and startup ordering.

## Responsibilities

- Load and initialize the **panel**, **launcher**, and **wallpaper** components
- Expose global signals used by other modules (startup, shutdown, visibility/state changes)
- Manage startup ordering and component initialization to ensure dependent systems come up in the correct order
- Register and dispatch global keyboard shortcuts

## Implementation notes

- Location: `shell/src/shell/`
- Main implementation: see `ShellManager.cpp`, `ShellManager.hpp`
- Signals: the shell exposes high-level signals for other components to connect to (avoid tight coupling; prefer signal-based initialization where possible)
