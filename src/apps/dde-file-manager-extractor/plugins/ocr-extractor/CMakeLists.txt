cmake_minimum_required(VERSION 3.10)

project(ocr-extractor)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE OCREXTRACTOR_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

set(BIN_NAME ${PROJECT_NAME})

set(OCREXTRACTOR_COMMON_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../abstractextractorplugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../extractor_plugin_global.h
)

find_package(Qt6 COMPONENTS Core Gui REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(DtkOcr REQUIRED IMPORTED_TARGET dtk6ocr)
find_package(Dtk6 COMPONENTS Core REQUIRED)

add_library(${BIN_NAME}
    SHARED
    ${OCREXTRACTOR_FILES}
    ${OCREXTRACTOR_COMMON_HEADERS}
)

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/..
)

target_link_libraries(${BIN_NAME}
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Dtk6::Core
        DFM6::base
        PkgConfig::DtkOcr
)

target_include_directories(${BIN_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

install(TARGETS ${BIN_NAME}
    LIBRARY
    DESTINATION ${DFM_PLUGIN_EXTRACTOR_DIR}
)

install(FILES ocrextractor.json
    DESTINATION ${DFM_PLUGIN_EXTRACTOR_DIR}
)

message(STATUS "DFM: ocr-extractor plugin configured")
