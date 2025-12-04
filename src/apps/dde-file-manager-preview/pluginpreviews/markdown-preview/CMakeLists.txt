cmake_minimum_required(VERSION 3.10)
project(dde-markdown-preview-plugin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE MARKDOWNPREVIEW_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${GlobalFiles}"
    )

set(BIN_NAME ${PROJECT_NAME})

find_package(Qt6 COMPONENTS Core REQUIRED)

add_library(${BIN_NAME}
    SHARED
    ${MARKDOWNPREVIEW_FILES}
)

set_target_properties(${BIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_PREVIEW_DIR}/previews)

target_link_libraries(${BIN_NAME}
    DFM6::base
    DFM6::framework
)

#install library file
install(TARGETS
    ${BIN_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_PREVIEW_DIR}
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/dde-markdown-preview-plugin.json DESTINATION ${DFM_BUILD_PLUGIN_PREVIEW_DIR}/previews)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dde-markdown-preview-plugin.json DESTINATION ${DFM_PLUGIN_PREVIEW_DIR})
