cmake_minimum_required(VERSION 3.10)
project(dfmimage-preview)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB IMAGEPREVIEW_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${GlobalFiles}"
    )
find_package(Dtk COMPONENTS Widget REQUIRED)

add_library(${PROJECT_NAME}
    SHARED
    ${IMAGEPREVIEW_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../previews)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/dde-image-preview-plugin.json DESTINATION ../../previews)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dde-image-preview-plugin.json DESTINATION ${DFM_PLUGIN_PREVIEW_DIR})

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_PREVIEW_DIR}
)
