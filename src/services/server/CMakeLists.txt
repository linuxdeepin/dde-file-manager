cmake_minimum_required(VERSION 3.1.0)

project(dfmservice-server)

if (NOT VERSION)
    set(VERSION "1.0.0")
endif()

if (NOT PROJECT_VERSION_MAJOR)
    set(PROJECT_VERSION_MAJOR 0)
endif()

set(DFM_BASE_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/../../dfm-base)

# qrc
set(QRC_FILE
    ${DFM_BASE_DIR}/qrc/configure.qrc
    )
qt5_add_resources(QRC_RESOURCES ${QRC_FILE})

FILE(GLOB GLOBAL_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

FILE(GLOB DEVICE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/device/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/device/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/device/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/device/private/*.cpp"
)

add_library(${PROJECT_NAME}
    SHARED
    ${QRC_RESOURCES}
    ${GLOBAL_FILES}
    ${DEVICE_FILES}
)

find_package(PkgConfig REQUIRED)

pkg_search_module(gsettings
    REQUIRED
    gsettings-qt
    IMPORTED_TARGET
)

pkg_search_module(dfm-mount
    REQUIRED
    dfm-mount
    IMPORTED_TARGET
)


add_library(DFMSERVICE::server ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/device
    ${CMAKE_CURRENT_SOURCE_DIR}/dialog
    ${DtkWidget_INCLUDE_DIRS}
)

# TODO(xust): remove dfm-base depends
target_link_libraries(
    ${PROJECT_NAME}
    DFM::base
    DFM::framework
    PkgConfig::gsettings
    PkgConfig::dfm-mount
    ${DtkWidget_LIBRARIES}
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${LIB_INSTALL_DIR}
)

# install head file
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION include/dfmservice-server
    FILES_MATCHING PATTERN "*.h"
    PATTERN "private" EXCLUDE
)
