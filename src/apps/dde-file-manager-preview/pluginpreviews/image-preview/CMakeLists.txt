cmake_minimum_required(VERSION 3.10)
project(dde-image-preview-plugin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE IMAGEPREVIEW_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${GlobalFiles}"
    )

set(BIN_NAME ${PROJECT_NAME})
find_package(Qt6 COMPONENTS Core REQUIRED)

add_library(${BIN_NAME}
    SHARED
    ${IMAGEPREVIEW_FILES}
)

set_target_properties(${BIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_PREVIEW_DIR}/previews)

# Skip build rpath for reproducible release build
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(${BIN_NAME} PROPERTIES SKIP_BUILD_RPATH TRUE)
endif()

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

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/dde-image-preview-plugin.json DESTINATION ${DFM_BUILD_PLUGIN_PREVIEW_DIR}/previews)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dde-image-preview-plugin.json DESTINATION ${DFM_PLUGIN_PREVIEW_DIR})


