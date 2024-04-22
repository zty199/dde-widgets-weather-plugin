# Define built-in KEY when building binary package
if (NOT DEFINED DEFAULT_WEB_API_KEY)
    set(DEFAULT_WEB_API_KEY "")
endif ()
add_definitions(-DDEFAULT_WEB_API_KEY="${DEFAULT_WEB_API_KEY}")

include_directories(api)
include_directories(log)

# https://cmake.org/cmake/help/v3.12/command/file.html#glob-recurse
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

qt5_add_resources(QRC_FILE ${CMAKE_SOURCE_DIR}/resources/resources.qrc)
