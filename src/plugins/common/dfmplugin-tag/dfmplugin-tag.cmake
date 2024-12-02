find_package(Qt${QT_VERSION_MAJOR} COMPONENTS DBus REQUIRED)

add_library(${BIN_NAME}
    SHARED
    ${TAG_FILES}
)

# generate dbus interface
if(${QT_VERSION_MAJOR} EQUAL "6")
    qt6_add_dbus_interface(Qt6App_dbus
        ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.Daemon.TagManager.xml
        tagmanager_interface_qt6)
    target_sources(${BIN_NAME} PRIVATE ${Qt6App_dbus})
else()
    qt5_add_dbus_interface(Qt5App_dbus
        ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.Daemon.TagManager.xml
        tagmanager_interface)
    target_sources(${BIN_NAME} PRIVATE ${Qt5App_dbus})
endif()

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_COMMON_DIR})

target_link_libraries(${BIN_NAME}
    Qt${QT_VERSION_MAJOR}::DBus
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
