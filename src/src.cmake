if (NOT DEFINED DEFAULT_WEB_API_KEY)
    set(DEFAULT_WEB_API_KEY "")
endif ()
add_definitions(-DDEFAULT_WEB_API_KEY="${DEFAULT_WEB_API_KEY}")

include_directories(api)
include_directories(log)

set(HEADERS
    "global.h"
    "plugin.h"
    "api/config.hpp"
    "api/data.hpp"
    "api/qweatherapi.h"
    "log/logging_categories.h"
    "settingsdialog.h"
    "weatherdisplaywidget.h"
)

set(SOURCES
    "plugin.cpp"
    "api/qweatherapi.cpp"
    "log/logging_categories.cpp"
    "settingsdialog.cpp"
    "weatherdisplaywidget.cpp"
)

qt5_add_resources(QRC_FILE ${CMAKE_SOURCE_DIR}/resources/resources.qrc)
