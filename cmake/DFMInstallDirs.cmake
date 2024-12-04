if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif()

# dbus xml dir
set(DFM_DBUS_XML_DIR ${CMAKE_SOURCE_DIR}/assets/dbus)

# defines plugins install dir
if(NOT DEFINED LIB_INSTALL_DIR)
    set(LIB_INSTALL_DIR ${CMAKE_INSTALL_FULL_LIBDIR})
endif()

# defines dde-file-manager lib dir
if(NOT DEFINED DFM_LIB_DIR)
    set(DFM_LIB_DIR ${LIB_INSTALL_DIR}/dde-file-manager)
endif()

# root plguins dir
if(NOT DEFINED DFM_PLUGIN_DIR)
    set(DFM_PLUGIN_DIR ${DFM_LIB_DIR}/plugins)
endif()

# build dir
if(NOT DEFINED DFM_BUILD_DIR)
    set(DFM_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/src)
endif()

# build plugins dir
if(NOT DEFINED DFM_BUILD_PLUGIN_DIR)
    set(DFM_BUILD_PLUGIN_DIR ${DFM_BUILD_DIR}/plugins)
endif()

set(DFM_BUILD_PLUGIN_COMMON_DIR ${DFM_BUILD_PLUGIN_DIR}/common)
set(DFM_BUILD_PLUGIN_DAEMON_DIR ${DFM_BUILD_PLUGIN_DIR}/daemon)
set(DFM_BUILD_PLUGIN_DESKTOP_DIR ${DFM_BUILD_PLUGIN_DIR}/desktop)
set(DFM_BUILD_PLUGIN_FILEDIALOG_DIR ${DFM_BUILD_PLUGIN_DIR}/filedialog)
set(DFM_BUILD_PLUGIN_FILEMANAGER_DIR ${DFM_BUILD_PLUGIN_DIR}/filemanager)
set(DFM_BUILD_PLUGIN_SERVER_DIR ${DFM_BUILD_PLUGIN_DIR}/server)
set(DFM_BUILD_PLUGIN_TOOLS_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/tools)

# common-core plugins dir
if(NOT DEFINED DFM_PLUGIN_COMMON_CORE_DIR)
    set(DFM_PLUGIN_COMMON_CORE_DIR ${DFM_PLUGIN_DIR}/common-core)
endif()

# desktop-core plugins dir
if(NOT DEFINED DFM_PLUGIN_DESKTOP_CORE_DIR)
    set(DFM_PLUGIN_DESKTOP_CORE_DIR ${DFM_PLUGIN_DIR}/desktop-core)
endif()

# filemanager-core plugins dir
if(NOT DEFINED DFM_PLUGIN_FILEMANAGER_CORE_DIR)
    set(DFM_PLUGIN_FILEMANAGER_CORE_DIR ${DFM_PLUGIN_DIR}/filemanager-core)
endif()

# common-edge plugins dir
if(NOT DEFINED DFM_PLUGIN_COMMON_EDGE_DIR)
    set(DFM_PLUGIN_COMMON_EDGE_DIR ${DFM_PLUGIN_DIR}/common-edge)
endif()

# daemon-edge plugins dir
if(NOT DEFINED DFM_PLUGIN_DAEMON_EDGE_DIR)
    set(DFM_PLUGIN_DAEMON_EDGE_DIR ${DFM_PLUGIN_DIR}/daemon-edge)
endif()

# server-edge plugins dir
if(NOT DEFINED DFM_PLUGIN_SERVER_EDGE_DIR)
    set(DFM_PLUGIN_SERVER_EDGE_DIR ${DFM_PLUGIN_DIR}/server-edge)
endif()

# desktop-edge plugins dir
if(NOT DEFINED DFM_PLUGIN_DESKTOP_EDGE_DIR)
    set(DFM_PLUGIN_DESKTOP_EDGE_DIR ${DFM_PLUGIN_DIR}/desktop-edge)
endif()

# filemanager-edge plugins dir
if(NOT DEFINED DFM_PLUGIN_FILEMANAGER_EDGE_DIR)
    set(DFM_PLUGIN_FILEMANAGER_EDGE_DIR ${DFM_PLUGIN_DIR}/filemanager-edge)
endif()

# tools
if(NOT DEFINED DFM_TOOLS_DIR)
    set(DFM_TOOLS_DIR ${DFM_LIB_DIR}/tools)
endif()

# dde-file-thumbnail-tool
if(NOT DEFINED DFM_THUMBNAIL_TOOL)
    set(DFM_THUMBNAIL_TOOL ${LIB_INSTALL_DIR}/dde-file-manager/tools)
endif()
