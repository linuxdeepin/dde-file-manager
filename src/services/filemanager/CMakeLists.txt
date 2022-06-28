cmake_minimum_required(VERSION 3.10)

project(dfmservice-filemanager)

if (NOT VERSION)
    set(VERSION "1.0.0")
endif()

if (NOT PROJECT_VERSION_MAJOR)
    set(PROJECT_VERSION_MAJOR 0)
endif()

# windows service sources
FILE(GLOB GLOBAL_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

FILE(GLOB WINDOWS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/windows/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/windows/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/windows/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/windows/private/*.cpp"
)


FILE(GLOB FILEENCRYPT_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/fileencrypt/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/fileencrypt/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/fileencrypt/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/fileencrypt/private/*.cpp"
)

FILE(GLOB SIDEBAR_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/sidebar/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/sidebar/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/sidebar/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/sidebar/private/*.cpp"
)


FILE(GLOB WORKSPACE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/workspace/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/workspace/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/workspace/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/workspace/private/*.cpp"
)

FILE(GLOB DETAILSPACE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/detailspace/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/detailspace/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/detailspace/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/detailspace/*/*.cpp"
)

FILE(GLOB SEARCH_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/search/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/search/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/search/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/search/maincontroller/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/search/maincontroller/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/search/maincontroller/task/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/search/maincontroller/task/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/search/searcher/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/search/searcher/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/search/searcher/fulltextsearcher/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/search/searcher/fulltextsearcher/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/search/searcher/filenamesearcher/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/search/searcher/filenamesearcher/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/search/utils/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/search/utils/*.cpp"
)

FILE(GLOB COMMAND_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/command/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/command/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/command/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/command/private/*.cpp"
)

FILE (GLOB BOOKMARK_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/bookmark/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/bookmark/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bookmark/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/bookmark/private/*.cpp"
    )

add_library(
    ${PROJECT_NAME}
    SHARED
    ${GLOBAL_FILES}
    ${WINDOWS_FILES}
    ${FILEENCRYPT_FILES}
    ${SIDEBAR_FILES}
    ${WORKSPACE_FILES}
    ${DETAILSPACE_FILES}
    ${SEARCH_FILES}
    ${COMMAND_FILES}
    ${BOOKMARK_FILES}
)

find_package(Dtk REQUIRED Widget REQUIRED)
find_package(Qt5 COMPONENTS X11Extras REQUIRED)
find_package(PkgConfig REQUIRED)

#full-text search dependences
pkg_check_modules(Lucene REQUIRED IMPORTED_TARGET liblucene++ liblucene++-contrib)
pkg_check_modules(Docparser REQUIRED IMPORTED_TARGET docparser)

add_library(DFMSERVICE::filemanager ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/windows
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    DFMSERVICE::common
    Qt5::X11Extras
    ${DtkWidget_LIBRARIES}
    PkgConfig::Lucene
    PkgConfig::Docparser
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
    DESTINATION include/dfmservice-filemanager
    FILES_MATCHING PATTERN "*.h"
    PATTERN "private" EXCLUDE
)
