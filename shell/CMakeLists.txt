set(PIKSEL_SHELL_SRCS
    PikselSystemClient.cpp
    PikselSystemClient.hpp
    AppDockModel.cpp
    AppDockModel.hpp
    ShellComponent.hpp
)

add_library(piksel_shell ${PIKSEL_SHELL_SRCS})

target_link_libraries(piksel_shell PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::DBus
)

target_include_directories(piksel_shell PUBLIC
    "${CMAKE_SOURCE_DIR}"
)
