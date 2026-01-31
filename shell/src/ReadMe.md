## `shell/src/` notes (C++ / Qt)

This tree contains the C++ side of the shell: core services, shell coordination, and widget/QML-hosting components.

### Conventions in this repo
- C++ standard is C++23 (see top-level `CMakeLists.txt`).
- Headers use classic include guards (`#ifndef ... #define ...`), not `#pragma once`.
- Components commonly expose functionality to QML via `Q_INVOKABLE` methods and Qt signals.

### Where to put things
- `shell/src/shell/`: global coordination (startup ordering, shared signals, command dispatch)
- `shell/src/components/`: user-facing UI pieces (panel, launcher, wallpaper, settings)
- `shell/src/core/`: core/client integration (e.g., DBus clients)

### When you change behavior
- If you modify startup ordering or add/remove a top-level component, also update `shell/src/shell/ReadMe.md`.
