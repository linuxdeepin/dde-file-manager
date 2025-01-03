cmake_minimum_required(VERSION 3.10)
project(dde-pdf-preview-plugin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB PDFPREVIEW_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${GlobalFiles}"
    )
find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(Dtk6 COMPONENTS Widget REQUIRED)
find_package(deepin-pdfium REQUIRED)

add_library(${PROJECT_NAME}
    SHARED
    ${PDFPREVIEW_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_PREVIEW_DIR}/previews)

target_link_libraries(${PROJECT_NAME}
    DFM6::base
    DFM6::framework
    Dtk6::Widget
    deepin-pdfium::deepin-pdfium
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/dde-pdf-preview-plugin.json DESTINATION ${DFM_BUILD_PLUGIN_PREVIEW_DIR}/previews)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dde-pdf-preview-plugin.json DESTINATION ${DFM_PLUGIN_PREVIEW_DIR})

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_PREVIEW_DIR}
)
