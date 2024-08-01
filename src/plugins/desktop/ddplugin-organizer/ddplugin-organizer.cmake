# 指定资源文件
set(QRC_FILE
    resources/images.qrc
)
if(${QT_VERSION_MAJOR} EQUAL "6")
    qt_add_resources(QRC_RESOURCES ${QRC_FILE})
else()
    qt5_add_resources(QRC_RESOURCES ${QRC_FILE})
endif()

add_library(${BIN_NAME}
    SHARED
    ${CORE_FILES}
    ${EXT_FILES}
    ${QRC_RESOURCES}
)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core)
set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_DESKTOP_DIR})

target_link_libraries(${BIN_NAME}
    DFM${DTK_VERSION_MAJOR}::base
    DFM${DTK_VERSION_MAJOR}::framework
    )

#install library file
install(TARGETS
    ${BIN_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DESKTOP_EDGE_DIR}
    )
