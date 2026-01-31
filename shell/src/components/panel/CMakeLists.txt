if(TEST_PANEL)
    add_definitions(-DTEST_PANEL)
endif()

qt_standard_project_setup()

qt_add_executable(PikselPanel
    Panel.cpp
    Panel.hpp
    PanelBattery.cpp
    PanelBattery.hpp
    PanelClock.cpp
    PanelClock.hpp
    PanelNetwork.cpp
    PanelNetwork.hpp
    PanelRunningApps.cpp
    PanelRunningApps.hpp
)

target_link_libraries(PikselPanel PRIVATE Qt6::Widgets Qt6::Qml Qt6::Quick Qt6::QuickWidgets Qt6::DBus)
