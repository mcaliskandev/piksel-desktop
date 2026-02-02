set(PIKSEL_APPLETS_SRCS
    battery/PanelBattery.cpp
    battery/PanelBattery.hpp
    clock/PanelClock.cpp
    clock/PanelClock.hpp
    network/PanelNetwork.cpp
    network/PanelNetwork.hpp
    runningapps/PanelRunningApps.cpp
    runningapps/PanelRunningApps.hpp
)

add_library(piksel_applets ${PIKSEL_APPLETS_SRCS})

target_link_libraries(piksel_applets PRIVATE
    Qt6::Core
    piksel_shell
)

target_include_directories(piksel_applets PUBLIC
    "${CMAKE_SOURCE_DIR}"
)
