# DFMBaseTestUtils.cmake - Helper functions for dde-file-manager dfm-base component testing
#
# This file contains the specific dependency and resource configuration needed to
# successfully compile the dfm-base unit tests.

#[[
Function: dfm_configure_dfm_base_dependencies
Purpose: Configures the extensive build dependencies required by the dfm-base component.
          This is a direct port of the logic from src/dfm-base/dfm-base.cmake
          to be used exclusively for the dfm-base unit test target.
Params:   TARGET_NAME - The name of the test object library to link against.
]]
function(dfm_configure_dfm_base_dependencies TARGET_NAME)
    message(STATUS "    Configuring dfm-base specific dependencies for ${TARGET_NAME}...")

    # Find required Qt and DTK packages
    find_package(Qt6 COMPONENTS Core Widgets Gui Concurrent DBus Sql Network REQUIRED)
    find_package(Dtk6 COMPONENTS Core Widget Gui REQUIRED)
    find_package(dfm6-io REQUIRED)
    find_package(dfm6-mount REQUIRED)
    find_package(dfm6-burn REQUIRED)

    # Find system libraries via PkgConfig
    find_package(PkgConfig REQUIRED)
    pkg_search_module(PolkitQt1 REQUIRED polkit-qt6-1)

    pkg_search_module(UDISKS2 REQUIRED udisks2)
    pkg_search_module(GIO_UNIX REQUIRED gio-unix-2.0)
    pkg_search_module(MOUNT REQUIRED mount)
    pkg_search_module(X11 REQUIRED x11)
    pkg_check_modules(LIBHEIF REQUIRED libheif)

    # Link libraries
    target_link_libraries(${TARGET_NAME} PRIVATE
        Qt6::Core Qt6::Widgets Qt6::Gui Qt6::Concurrent Qt6::DBus Qt6::Sql Qt6::Network
        Dtk6::Core Dtk6::Widget Dtk6::Gui
        dfm6-io
        dfm6-mount
        dfm6-burn
        ${UDISKS2_LIBRARIES}
        ${GIO_UNIX_LIBRARIES}
        ${PolkitQt1_LIBRARIES}
        ${MOUNT_LIBRARIES}
        ${X11_LIBRARIES}
        ${LIBHEIF_LIBRARIES}
        poppler-cpp
    )

    # Add include directories from pkg-config
    target_include_directories(${TARGET_NAME} PRIVATE
        ${UDISKS2_INCLUDE_DIRS}
        ${GIO_UNIX_INCLUDE_DIRS}
        ${PolkitQt1_INCLUDE_DIRS}
        ${MOUNT_INCLUDE_DIRS}
        ${X11_INCLUDE_DIRS}
    )

    # Add compile definitions
    target_compile_definitions(${TARGET_NAME} PRIVATE QT_NO_SIGNALS_SLOTS_KEYWORDS)

    message(STATUS "    -> Configured dfm-base dependencies.")
endfunction()

#[[
Function: dfm_handle_dfm_base_resources
Purpose:  Handles QRC resource compilation and D-Bus interface generation for dfm-base.
          a prefix header to solve incomplete type issues in the original source headers.
Params:   TARGET_NAME - The test object library to add the generated sources to.
]]
function(dfm_handle_dfm_base_resources TARGET_NAME)
    message(STATUS "    Handling dfm-base resources for ${TARGET_NAME}...")

    # Find and process QRC files
    set(QRC_FILES
        ${DFM_SOURCE_DIR}/src/dfm-base/qrc/skin/skin.qrc
        ${DFM_SOURCE_DIR}/src/dfm-base/qrc/skin/filemanager.qrc
        ${DFM_SOURCE_DIR}/src/dfm-base/qrc/themes/themes.qrc
        ${DFM_SOURCE_DIR}/src/dfm-base/qrc/configure.qrc
        ${DFM_SOURCE_DIR}/src/dfm-base/qrc/resources/resources.qrc
        ${DFM_SOURCE_DIR}/src/dfm-base/qrc/chinese2pinyin/chinese2pinyin.qrc
    )
    qt_add_resources(${TARGET_NAME} ${QRC_FILES})

    # Generate D-Bus interface exactly as in dfm-base.cmake
    qt6_add_dbus_interface(Qt6App_dbus
        ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.Daemon.DeviceManager.xml
        devicemanager_interface_qt6)
    target_sources(${TARGET_NAME} PRIVATE ${Qt6App_dbus})
    
    # Add the directory containing the generated D-Bus interface header to include path
    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

    message(STATUS "    -> Handled dfm-base resources.")
endfunction() 