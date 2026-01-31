## `shell/qml/` notes (Qt Quick)

QML files here are compiled into the application via the Qt resource system.

### Resource system rules of thumb
- If you add a new QML file (or move/rename one), update `shell/resources/resources.qrc`.
- Runtime references typically use `qrc:/...` (for example, icons under `shell/resources/icons/...`).

### Integration pattern
- Keep UI logic in QML; keep system/business logic in C++.
- Prefer calling into C++ via `Q_INVOKABLE` methods or Qt signals rather than duplicating logic in QML.

### Style
- Keep bindings simple and readable; extract repeated logic into small JS functions inside the component.
