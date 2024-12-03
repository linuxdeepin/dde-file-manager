find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

add_library(${BIN_NAME}
    SHARED
    ${MENU_FILES}
)

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_COMMON_DIR})

target_link_libraries(${BIN_NAME}
    DFM${DTK_VERSION_MAJOR}::base
    DFM${DTK_VERSION_MAJOR}::framework
)

target_compile_definitions(${BIN_NAME} PRIVATE MENU_CHECK_FOCUSONLY)

#install library file
install(TARGETS
    ${BIN_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_COMMON_CORE_DIR}
)
