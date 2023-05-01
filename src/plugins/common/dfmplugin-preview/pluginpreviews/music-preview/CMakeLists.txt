cmake_minimum_required(VERSION 3.10)
project(dfmmusic-preview)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB MUSICPREVIEW_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.qrc"
    "${GlobalFiles}"
    )
find_package(Dtk COMPONENTS Widget REQUIRED)
find_package(Qt5 COMPONENTS Multimedia REQUIRED)
find_package(ICU COMPONENTS i18n io uc REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_search_module(taglib REQUIRED taglib)

add_library(${PROJECT_NAME}
    SHARED
    ${MUSICPREVIEW_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../previews)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${DtkWidget_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    ${DtkWidget_LIBRARIES}
    ${Qt5Multimedia_LIBRARIES}
    ${taglib_LIBRARIES}
    ICU::i18n
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/dde-music-preview-plugin.json DESTINATION ../../previews)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dde-music-preview-plugin.json DESTINATION ${DFM_PLUGIN_PREVIEW_DIR})

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_PREVIEW_DIR}
)
