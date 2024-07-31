cmake_minimum_required(VERSION 3.10)
project(dfmmusic-preview)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE MUSICPREVIEW_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.qrc"
    "${GlobalFiles}"
    )

# Qt6
if (DFM_BUILD_WITH_QT6)
    set(BIN_NAME dfm6music-preview)
    set(QT_VERSION_MAJOR 6)
    set(DTK_VERSION_MAJOR 6)

    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Multimedia REQUIRED)
    find_package(ICU COMPONENTS i18n io uc REQUIRED)
    find_package(PkgConfig REQUIRED)

    pkg_search_module(taglib REQUIRED taglib)

    add_library(${BIN_NAME}
        SHARED
        ${MUSICPREVIEW_FILES}
    )

    set_target_properties(${BIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../previews)

    target_link_libraries(${BIN_NAME}
        DFM${DTK_VERSION_MAJOR}::base
        DFM${DTK_VERSION_MAJOR}::framework
        Qt${QT_VERSION_MAJOR}::Multimedia
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
endif()

# Qt5
set(BIN_NAME dfmmusic-preview)
set(QT_VERSION_MAJOR 5)
set(DTK_VERSION_MAJOR "")

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Multimedia REQUIRED)
find_package(ICU COMPONENTS i18n io uc REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_search_module(taglib REQUIRED taglib)

add_library(${BIN_NAME}
    SHARED
    ${MUSICPREVIEW_FILES}
)

set_target_properties(${BIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../previews)

target_link_libraries(${BIN_NAME}
    DFM::base
    DFM::framework
    Qt${QT_VERSION_MAJOR}::Multimedia
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

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/dde-music-preview-plugin.json DESTINATION ../../previews)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dde-music-preview-plugin.json DESTINATION ${DFM_PLUGIN_PREVIEW_DIR})

