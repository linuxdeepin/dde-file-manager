cmake_minimum_required(VERSION 3.10)
project(dde-music-preview-plugin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE MUSICPREVIEW_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.qrc"
    "${GlobalFiles}"
    )

set(BIN_NAME ${PROJECT_NAME})
find_package(Qt6 COMPONENTS Widgets Multimedia REQUIRED)
find_package(ICU COMPONENTS i18n io uc REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_search_module(taglib REQUIRED taglib)

add_library(${BIN_NAME}
    SHARED
    ${MUSICPREVIEW_FILES}
)

set_target_properties(${BIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_PREVIEW_DIR}/previews)

# Skip build rpath for reproducible release build
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(${BIN_NAME} PROPERTIES SKIP_BUILD_RPATH TRUE)
endif()

target_link_libraries(${BIN_NAME}
    DFM6::base
    DFM6::framework
    Qt6::Widgets
    Qt6::Multimedia
    ${taglib_LIBRARIES}
    ICU::i18n
)

#install library file
install(TARGETS
    ${BIN_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_PREVIEW_DIR}
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/dde-music-preview-plugin.json DESTINATION ${DFM_BUILD_PLUGIN_PREVIEW_DIR}/previews)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dde-music-preview-plugin.json DESTINATION ${DFM_PLUGIN_PREVIEW_DIR})

