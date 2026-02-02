set(PIKSEL_SURFACES_SRCS
    panel/Panel.cpp
    panel/Panel.hpp
    desktop/Wallpaper.cpp
    desktop/Wallpaper.hpp
)

add_library(piksel_surfaces ${PIKSEL_SURFACES_SRCS})

target_link_libraries(piksel_surfaces PRIVATE
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickWidgets
    piksel_applets
    piksel_settings
    piksel_shell
)

target_include_directories(piksel_surfaces PUBLIC
    "${CMAKE_SOURCE_DIR}"
)
