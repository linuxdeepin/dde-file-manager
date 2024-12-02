add_library(${BIN_NAME}
    SHARED
    ${SRC_FILES}
)

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY
    ${DFM_BUILD_PLUGIN_DESKTOP_DIR})

find_package(Dtk${DTK_VERSION_MAJOR} COMPONENTS Widget REQUIRED)

if(${QT_VERSION_MAJOR} EQUAL "6")
    target_include_directories(${BIN_NAME} PUBLIC
        ${Qt6Widgets_PRIVATE_INCLUDE_DIRS}
    )
    target_link_libraries(${BIN_NAME}
        DFM6::base
        DFM6::framework
    )
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus )
    target_include_directories(${BIN_NAME} PUBLIC
        ${Qt5Widgets_PRIVATE_INCLUDE_DIRS}
        ${DFrameworkDBus_INCLUDE_DIRS}
    )
    target_link_libraries(${BIN_NAME}
        DFM::base
        DFM::framework
        ${DFrameworkDBus_LIBRARIES}
    )
endif()

#install library file
install(TARGETS
    ${BIN_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DESKTOP_CORE_DIR}
)
