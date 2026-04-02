cmake_minimum_required(VERSION 3.10)

project(text-extractor)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE TEXTEXTRACTOR_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

set(BIN_NAME ${PROJECT_NAME})

set(TEXTEXTRACTOR_COMMON_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../abstractextractorplugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../extractor_plugin_global.h
)

find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(Docparser REQUIRED IMPORTED_TARGET docparser)
find_package(Dtk6 COMPONENTS Core REQUIRED)

add_library(${BIN_NAME}
    SHARED
    ${TEXTEXTRACTOR_FILES}
    ${TEXTEXTRACTOR_COMMON_HEADERS}
)

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/..
)

target_link_libraries(${BIN_NAME}
    PRIVATE
        Qt6::Core
        Dtk6::Core
        DFM6::base
        PkgConfig::Docparser
)

target_include_directories(${BIN_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Install the plugin
install(TARGETS ${BIN_NAME}
    LIBRARY
    DESTINATION ${DFM_PLUGIN_EXTRACTOR_DIR}
)

# Install metadata file
install(FILES textextractor.json
    DESTINATION ${DFM_PLUGIN_EXTRACTOR_DIR}
)

message(STATUS "DFM: text-extractor plugin configured")
