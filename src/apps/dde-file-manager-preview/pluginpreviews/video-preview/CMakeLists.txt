cmake_minimum_required(VERSION 3.10)

project(deepin-video-preview-plugin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB VIDEOPREVIEW_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.qrc"
    "${GlobalFiles}"
    )


find_package(PkgConfig REQUIRED)
find_package(Qt6 COMPONENTS Core REQUIRED)

pkg_search_module(libdmr REQUIRED libdmr)

add_library(${PROJECT_NAME}
    SHARED
    ${VIDEOPREVIEW_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY  ${DFM_BUILD_PLUGIN_PREVIEW_DIR}/previews)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${libdmr_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    DFM6::base
    DFM6::framework
    ${libdmr_LIBRARIES}
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/dde-video-preview-plugin.json DESTINATION  ${DFM_BUILD_PLUGIN_PREVIEW_DIR}/previews)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dde-video-preview-plugin.json DESTINATION ${DFM_PLUGIN_PREVIEW_DIR})

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_PREVIEW_DIR}
)
