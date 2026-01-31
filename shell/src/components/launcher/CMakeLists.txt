# === PikselLauncher ===
qt_add_library(PikselLauncher STATIC
    Launcher.cpp
    Launcher.hpp
)

target_link_libraries(PikselLauncher PRIVATE Qt6::Widgets)
target_link_libraries(PikselLauncher PRIVATE Qt6::QuickWidgets Qt6::Quick Qt6::Qml)
