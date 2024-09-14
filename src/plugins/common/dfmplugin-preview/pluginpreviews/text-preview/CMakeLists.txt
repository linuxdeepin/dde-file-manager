cmake_minimum_required(VERSION 3.10)
project(dfmtext-preview)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE TEXTPREVIEW_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${GlobalFiles}"
    )

# Qt5
set(BIN_NAME dfmtext-preview)
set(QT_VERSION_MAJOR 5)
set(DTK_VERSION_MAJOR "")
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

add_library(${BIN_NAME}
    SHARED
    ${TEXTPREVIEW_FILES}
)

set_target_properties(${BIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../previews)

target_link_libraries(${BIN_NAME}
    DFM::base
    DFM::framework
)

#install library file
install(TARGETS
    ${BIN_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_PREVIEW_DIR}
)

# Qt6
if (DFM_BUILD_WITH_QT6)
    set(BIN_NAME dfm6text-preview)
    set(QT_VERSION_MAJOR 6)
    set(DTK_VERSION_MAJOR 6)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

    add_library(${BIN_NAME}
        SHARED
        ${TEXTPREVIEW_FILES}
    )

    set_target_properties(${BIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../previews)

    target_link_libraries(${BIN_NAME}
        DFM${DTK_VERSION_MAJOR}::base
        DFM${DTK_VERSION_MAJOR}::framework
    )

    #install library file
    install(TARGETS
        ${BIN_NAME}
        LIBRARY
        DESTINATION
        ${DFM_PLUGIN_PREVIEW_DIR}
    )
endif()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/dde-text-preview-plugin.json DESTINATION ../../previews)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dde-text-preview-plugin.json DESTINATION ${DFM_PLUGIN_PREVIEW_DIR})

