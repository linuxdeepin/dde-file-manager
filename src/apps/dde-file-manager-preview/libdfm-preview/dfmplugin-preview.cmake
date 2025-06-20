find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

add_executable(${BIN_NAME}
    ${FILEPREVIEW_FILES}
)

add_definitions(-DPLUGINDIR=\"${DFM_PLUGIN_PREVIEW_DIR}\")
set(VERSION 1.8.2)
add_definitions(-DVERSION=\"${VERSION}\")

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_COMMON_DIR})

target_link_libraries(${BIN_NAME}
    DFM${DTK_VERSION_MAJOR}::base
    DFM${DTK_VERSION_MAJOR}::framework
)

#install library file
install(TARGETS
    ${BIN_NAME}
    LIBRARY
    DESTINATION
    ${CMAKE_INSTALL_BINDIR}
)
