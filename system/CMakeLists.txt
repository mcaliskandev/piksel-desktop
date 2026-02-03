qt_add_dbus_adaptor(PIKSEL_SYSTEM_DBUS_SRCS
    "${CMAKE_CURRENT_LIST_DIR}/dbus/piksel.system.xml"
    "${CMAKE_CURRENT_LIST_DIR}/SystemService.hpp"
    SystemService
)

set(PIKSEL_SYSTEM_SRCS
    SystemService.cpp
    SystemService.hpp
    config/Config.cpp
    config/Config.hpp
    ${PIKSEL_SYSTEM_DBUS_SRCS}
)

add_library(piksel_system ${PIKSEL_SYSTEM_SRCS})

target_link_libraries(piksel_system PRIVATE
    Qt6::Core
    Qt6::DBus
)

target_include_directories(piksel_system PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}"
)
