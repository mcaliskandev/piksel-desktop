# PikselDesktop

Piksel Desktop is a lightweight Linux desktop system focused on simplicity, performance, and clarity written in C++ and Qt.

## Build Requirements

- OS: Linux
- Architecture : amd64
- CMake: >= 3.28
- Qt: 6.6+
- Compilers:
  - Clang 17+ (recommended, tested)
  - GCC 13+ (may work)
- C++ version: 23

### Tested With
- clang++ 17.0.6
- Qt 6.6.1
- Ubuntu 24.04.3

## License

PikselDesktop is licensed under the MIT License.
See the [LICENSE](LICENSE) file for details.

## Modules
- Shell Manager
- Core
- Components
  - Panel
  - Launcher
  - Wallpaper
  - Settings 

## Build

### Helper script (clean, build, run)

A small helper script is available at `scripts/dev.sh` to clean, configure, build, and run the default `PikselDesktop` target.

Usage:

```bash
./scripts/dev.sh build
./scripts/dev.sh run
./scripts/dev.sh clean
```

## How to Run PikselDesktop for Testing

You can run PikselDesktop with any wayland compositor. But, recommended one is PikselCompositor which is developed for Piksel Desktop Environment (Piksel DE).

Run below command to run the piksel-desktop with your favorite wayland compositor. 

*Note : Below command assumes that target compositor uses wayland-0.*

```bash
WAYLAND_DISPLAY=wayland-0 build/PikselDesktop
```

## Developer Notes

### Repository layout (high level)
- `shell/src/`: C++ implementation for shell and UI components
- `shell/qml/`: QML UI files bundled into the binary via `shell/resources/resources.qrc`
- `shell/resources/`: icons and other assets (also referenced by `shell/resources/resources.qrc`)
- `shell/resources/resources.qrc`: source for QML/assets are compiled into the Qt resource system
- `scripts/dev.sh`: script for developer to easily configure/build/run/clean the code

### Change guidelines
- Prefer small, focused patches; keep unrelated refactors out of feature/bugfix PRs.
- When adding/removing source files, update `CMakeLists.txt` (executable sources list).
- When adding/removing QML or resource files, update `shell/resources/resources.qrc`.
- Keep UI wiring signal-based (Qt signals/slots, QML signals) to avoid tight coupling across components.

### Suggested checks before handoff
- Build locally: `./scripts/dev.sh build`
- If you touched QML/resources: ensure the resource paths still resolve as `qrc:/...`
