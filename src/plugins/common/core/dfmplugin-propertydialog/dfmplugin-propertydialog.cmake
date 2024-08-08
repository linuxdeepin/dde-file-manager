set(QRC_FILES
    qrc/images.qrc
    )
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
if(${QT_VERSION_MAJOR} EQUAL "6")
    qt_add_resources(QRC_RESOURCES ${QRC_FILES})
else()
    qt5_add_resources(QRC_RESOURCES ${QRC_FILES})
endif()

add_library(${BIN_NAME}
    SHARED
    ${DETAILSPCE_FILES}
    ${QRC_RESOURCES}
)

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
    ${DFM_PLUGIN_COMMON_CORE_DIR}
)
