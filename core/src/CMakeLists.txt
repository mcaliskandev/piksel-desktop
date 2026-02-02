qt_add_dbus_adaptor(PIKSEL_CORE_DBUS_SRCS
    "${CMAKE_CURRENT_LIST_DIR}/dbus/piksel.core.xml"
    "${CMAKE_CURRENT_LIST_DIR}/CoreService.hpp"
    CoreService
)

set(PIKSEL_CORE_SRCS
    CoreService.cpp
    CoreService.hpp
    config/Config.cpp
    config/Config.hpp
    ${PIKSEL_CORE_DBUS_SRCS}
)

add_library(piksel_core ${PIKSEL_CORE_SRCS})

target_link_libraries(piksel_core PRIVATE
    Qt6::Core
    Qt6::DBus
)

target_include_directories(piksel_core PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}"
)
