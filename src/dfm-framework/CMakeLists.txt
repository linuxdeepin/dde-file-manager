cmake_minimum_required(VERSION 3.10)

# If do't define version number, specify the version number
set (VERSION "1.0.0" CACHE STRING "define project version")

# find source code
file(GLOB_RECURSE INCLUDE_FILES  CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/dfm-framework/*")
file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS
    "./*.h"
    "./*.cpp"
)

include(dfm-framework-qt5.cmake)

# Qt6
if (DFM_BUILD_WITH_QT6)
include(dfm-framework-qt6.cmake)
endif()




