# Shell (C++ / Qt)

This tree contains the C++ side of the shell: system services, shell coordination, and the code that hosts QML-driven UI modules.

## Conventions
- C++ standard is C++23 (see top-level `CMakeLists.txt`).
- Headers use classic include guards (`#ifndef ... #define ...`), not `#pragma once`.
- Components commonly expose functionality to QML via `Q_INVOKABLE` methods and Qt signals.

## Responsibilities
- Load and initialize **surfaces** (panel, desktop) and **plugins** (applets + launcher).
- Expose global signals used by other modules (startup, shutdown, visibility/state changes).
- Manage startup ordering and component initialization so dependent systems come up in the correct order.
- Register and dispatch global keyboard shortcuts.

## Where to put things
- `shell/`: global coordination (startup ordering, shared signals, command dispatch) and system/client integration (e.g., DBus clients).
- `surfaces/`: host containers such as the panel and desktop (load applets/wallpaper plugins).
- `applets/`: small status/utility applets consumed by surfaces (battery, clock, network, running apps).
- `launcher/`: the application launcher plugin.
- `settings/`: settings windows/modules.

## Implementation notes
- Main implementation: see `ShellManager.cpp`, `ShellManager.hpp` in `shell/`.
- Host modules live in `surfaces/` and `launcher/`; applets in `applets/`.
- Signals: the shell exposes high-level signals for other components to connect to (avoid tight coupling; prefer signal-based initialization where possible).
