add_library(${BIN_NAME}
    SHARED
    ${CORE_FILES}
    ${SCREEN_FILES}
    ${EXT_FILES}
)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Gui)
set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_DESKTOP_DIR})

target_link_libraries(${BIN_NAME}
    DFM${DTK_VERSION_MAJOR}::base
    DFM${DTK_VERSION_MAJOR}::framework
    )

if(${QT_VERSION_MAJOR} EQUAL "6")
    target_include_directories(${BIN_NAME} PUBLIC
        ${Qt6Widgets_PRIVATE_INCLUDE_DIRS}
    )
else()
    target_include_directories(${BIN_NAME} PUBLIC
        ${Qt5Widgets_PRIVATE_INCLUDE_DIRS}
    )
endif()

#install library file
install(TARGETS
    ${BIN_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DESKTOP_CORE_DIR}
)

