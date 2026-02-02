set(PIKSEL_SETTINGS_SRCS
    SettingsWindow.hpp
    SettingsWindow.cpp
    ui/SettingsWindow.ui
    ui/SettingsWallpaper.ui
)

add_library(piksel_settings ${PIKSEL_SETTINGS_SRCS})

set_property(TARGET piksel_settings PROPERTY AUTOUIC_SEARCH_PATHS
    "${CMAKE_CURRENT_LIST_DIR}/ui"
)

target_link_libraries(piksel_settings PRIVATE
    Qt6::Widgets
    piksel_shell
)

target_include_directories(piksel_settings PUBLIC
    "${CMAKE_SOURCE_DIR}"
)
