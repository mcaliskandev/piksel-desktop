if(TEST_WALLPAPER)
    add_definitions(-DTEST_WALLPAPER)
endif()

qt_add_resources(WALLPAPER_RES ../../../resources/resources.qrc)

qt_add_executable(PikselWallpaper
    Wallpaper.cpp
    Wallpaper.hpp
    ../../core/PikselCoreClient.cpp
    ../../core/PikselCoreClient.hpp
    ${WALLPAPER_RES}
)

target_link_libraries(PikselWallpaper PRIVATE Qt6::Widgets Qt6::Qml Qt6::Quick Qt6::QuickWidgets Qt6::DBus)
