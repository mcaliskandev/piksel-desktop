set(PIKSEL_LAUNCHER_SRCS
    Launcher.cpp
    Launcher.hpp
    LauncherAppsModel.cpp
    LauncherAppsModel.hpp
)

add_library(piksel_launcher ${PIKSEL_LAUNCHER_SRCS})

target_link_libraries(piksel_launcher PRIVATE
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickWidgets
    piksel_shell
)

target_include_directories(piksel_launcher PUBLIC
    "${CMAKE_SOURCE_DIR}"
)
