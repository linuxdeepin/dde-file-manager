cmake_minimum_required(VERSION 3.10)

# If do't define version number, specify the version number
set (VERSION "1.0.0" CACHE STRING "define project version")
set(BIN_NAME dfm6-base) # Qt6
set(OLD_BIN_NAME dfm-base)

if (NOT VERSION)
    set(VERSION "1.0.0")
endif()

if (NOT PROJECT_VERSION_MAJOR)
    set(PROJECT_VERSION_MAJOR 1)
endif()

# Disbale qDebug(), qInfo(), qWaring(), qCritical()
# Use qCDebug() instead of qDebug()
if(OPT_DISABLE_QDEBUG)
    add_definitions(-DDFM_DISABLE_DEBUG_MACRO)
endif()

# signals and slots keyword is disable, use Q_SIGNALS and Q_SLOTS instead
add_compile_definitions(QT_NO_SIGNALS_SLOTS_KEYWORDS)
add_compile_definitions(THUMBNAIL_TOOL_DIR="${DFM_THUMBNAIL_TOOL}")

# find source code
file(GLOB_RECURSE INCLUDE_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/dfm-base/*")
file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS
    "./*.h"
    "./*.hpp"
    "./*.cpp"
)

# add qrc resources
set(QRC_FILES
    qrc/skin/skin.qrc
    qrc/skin/filemanager.qrc
    qrc/themes/themes.qrc
    qrc/configure.qrc
    qrc/resources/resources.qrc
    qrc/chinese2pinyin/chinese2pinyin.qrc
)

include(dfm-base-qt6.cmake)

