add_library(${BIN_NAME}
    SHARED
    ${SRC_FILES}
    ${EXT_FILES}
    ${QRCS}
)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Svg)
find_package(Dtk${DTK_VERSION_MAJOR} COMPONENTS Widget REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(xcb REQUIRED xcb IMPORTED_TARGET)

if(${QT_VERSION_MAJOR} EQUAL "6")
    set(INCLUDE_DIRS
        ${Qt6Widgets_PRIVATE_INCLUDE_DIRS}
    )
    set(EXTRA_LINK_LIBS "")
else()
    pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus)
    set(INCLUDE_DIRS
        ${DFrameworkDBus_INCLUDE_DIRS}
        ${Qt5Widgets_PRIVATE_INCLUDE_DIRS}
    )
    set(EXTRA_LINK_LIBS
        ${DFrameworkDBus_LIBRARIES}
    )
endif()

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_DESKTOP_DIR})


target_include_directories(${BIN_NAME} PUBLIC
    ${INCLUDE_DIRS}
)

target_link_libraries(${BIN_NAME}
    Qt${QT_VERSION_MAJOR}::Svg
    DFM${DTK_VERSION_MAJOR}::base
    DFM${DTK_VERSION_MAJOR}::framework
    PkgConfig::xcb
    ${EXTRA_LINK_LIBS}
    )

#install library file
install(TARGETS
    ${BIN_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DESKTOP_CORE_DIR}
)
