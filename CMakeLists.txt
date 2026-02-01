cmake_minimum_required(VERSION 3.28.3)
project(PikselDesktop VERSION 0.1 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Qml Quick QuickWidgets DBus)

qt_add_resources(RESOURCES shared/resources/resources.qrc)

qt_add_dbus_adaptor(PIKSEL_CORE_DBUS_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/core/src/dbus/piksel.core.xml"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/src/CoreService.hpp"
    CoreService
)

set(PIKSEL_FM_RESOURCES "")
set(PIKSEL_FM_QML_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../Pusula/qml/Pusula/Pusula.qml")
if(EXISTS "${PIKSEL_FM_QML_FILE}")
    set(PIKSEL_FM_QRC_FILE "${CMAKE_CURRENT_BINARY_DIR}/pusula_resources.qrc")
    file(WRITE "${PIKSEL_FM_QRC_FILE}" "<!DOCTYPE RCC>\n<RCC>\n  <qresource prefix=\"/\">\n    <file alias=\"qml/Pusula/Pusula.qml\">${PIKSEL_FM_QML_FILE}</file>\n  </qresource>\n</RCC>\n")
    qt_add_resources(PIKSEL_FM_RESOURCES "${PIKSEL_FM_QRC_FILE}")
endif()

add_executable(PikselDesktop
    shell/main.cpp
    surfaces/panel/Panel.cpp
    surfaces/panel/Panel.hpp
    applets/battery/PanelBattery.cpp
    applets/battery/PanelBattery.hpp
    applets/clock/PanelClock.cpp
    applets/clock/PanelClock.hpp
    applets/network/PanelNetwork.cpp
    applets/network/PanelNetwork.hpp
    applets/runningapps/PanelRunningApps.cpp
    applets/runningapps/PanelRunningApps.hpp
    launcher/Launcher.cpp
    launcher/Launcher.hpp
    launcher/LauncherAppsModel.cpp
    launcher/LauncherAppsModel.hpp
    surfaces/desktop/Wallpaper.cpp
    surfaces/desktop/Wallpaper.hpp
    settings/SettingsWindow.hpp
    settings/SettingsWindow.cpp
    settings/ui/SettingsWindow.ui
    settings/ui/SettingsWallpaper.ui
    shell/PikselCoreClient.cpp
    shell/PikselCoreClient.hpp
    shell/AppDockModel.cpp
    shell/AppDockModel.hpp
    shell/ShellManager.cpp
    shell/ShellManager.hpp
    shell/ShellComponent.hpp
    core/src/CoreService.cpp
    core/src/config/Config.cpp
    ${RESOURCES}
    ${PIKSEL_FM_RESOURCES}
    ${PIKSEL_CORE_DBUS_SRCS}
)

set_property(TARGET PikselDesktop PROPERTY AUTOUIC_SEARCH_PATHS
    "${CMAKE_CURRENT_SOURCE_DIR}/settings/ui"
)

# Use the external Pusula project (placed alongside PikselDesktop)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../Pusula/CMakeLists.txt")
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../Pusula" "${CMAKE_BINARY_DIR}/thirdparty/Pusula" EXCLUDE_FROM_ALL)
    target_link_libraries(PikselDesktop PRIVATE Pusula::Pusula)
    target_compile_definitions(PikselDesktop PRIVATE PIKSEL_WITH_PUSULA=1)
else()
    message(WARNING "Pusula project not found at ../Pusula — fall back to building without Pusula")
endif()

target_link_libraries(PikselDesktop PRIVATE Qt6::Widgets Qt6::Qml Qt6::Quick Qt6::QuickWidgets Qt6::DBus)

target_include_directories(PikselDesktop PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/shell"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/src"
)
