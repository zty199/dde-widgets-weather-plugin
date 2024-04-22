find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Network LinguistTools)
find_package(Dtk REQUIRED COMPONENTS Core Gui Widget)
find_package(DdeWidgets REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_search_module(Zlib REQUIRED zlib)

include(src.cmake)

add_library(${PROJECT_NAME} MODULE ${HEADERS} ${SOURCES} ${QRC_FILE})

target_include_directories(${PROJECT_NAME} PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    Dtk::Core
    Dtk::Gui
    Dtk::Widget
    ${DdeWidgets_INCLUDE_DIR}
    ${Zlib_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    Dtk::Core
    Dtk::Gui
    Dtk::Widget
    ${DdeWidgets_LIBRARIES}
    ${Zlib_LIBRARIES}
)

set(PLUGIN_NAME dde-weatherwidget-plugin)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PLUGIN_NAME})

install_dde_widgets_plugin(${PROJECT_NAME})
