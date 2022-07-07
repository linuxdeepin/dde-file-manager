cmake_minimum_required(VERSION 3.10)

project(dfmservice-common)

if (NOT VERSION)
    set(VERSION "1.0.0")
endif()

if (NOT PROJECT_VERSION_MAJOR)
    set(PROJECT_VERSION_MAJOR 0)
endif()

FILE(GLOB GLOBAL_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

FILE(GLOB DELEGATE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/delegate/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/delegate/*.cpp"
)

add_library(${PROJECT_NAME}
    SHARED
    ${GLOBAL_FILES}
    ${DELEGATE_FILES}
)

find_package(Dtk REQUIRED Widget REQUIRED)
find_package(Dtk REQUIRED Widget REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS DBus REQUIRED)
find_package(Qt5 COMPONENTS Svg REQUIRED)


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

add_library(DFMSERVICE::common ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/dialog
    ${DtkWidget_INCLUDE_DIRS}
)

target_link_libraries(
    ${PROJECT_NAME}
    DFM::base
    DFM::framework
    PkgConfig::gsettings
    PkgConfig::dfm-mount
    ${DtkWidget_LIBRARIES}
    Qt5::DBus
    Qt5::Svg
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
    DESTINATION include/dfmservice-common
    FILES_MATCHING PATTERN "*.h"
    PATTERN "private" EXCLUDE
)
