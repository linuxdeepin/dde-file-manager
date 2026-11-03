cmake_minimum_required(VERSION 3.10)
project(dde-rofd-preview-plugin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB ROFDPREVIEW_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${GlobalFiles}"
    )
find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(Dtk6 COMPONENTS Widget REQUIRED)

add_library(${PROJECT_NAME}
    SHARED
    ${ROFDPREVIEW_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_PREVIEW_DIR}/previews)

target_link_libraries(${PROJECT_NAME}
    DFM6::base
    DFM6::framework
    Dtk6::Widget
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/dde-rofd-preview-plugin.json DESTINATION ${DFM_BUILD_PLUGIN_PREVIEW_DIR}/previews)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dde-rofd-preview-plugin.json DESTINATION ${DFM_PLUGIN_PREVIEW_DIR})

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_PREVIEW_DIR}
)
