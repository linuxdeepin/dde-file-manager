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

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Multimedia REQUIRED)
find_package(ICU COMPONENTS i18n io uc REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_search_module(taglib REQUIRED taglib)

add_library(${PROJECT_NAME}
    SHARED
    ${MUSICPREVIEW_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../previews)


target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    Qt${QT_VERSION_MAJOR}::Multimedia
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
