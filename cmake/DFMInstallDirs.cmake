if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif()

# dbus xml dir
set(DFM_DBUS_XML_DIR ${CMAKE_SOURCE_DIR}/assets/dbus)

# defines plugins install dir
if(NOT DEFINED LIB_INSTALL_DIR)
    set(LIB_INSTALL_DIR ${CMAKE_INSTALL_FULL_LIBDIR})
endif()

# root plguins dir
if(NOT DEFINED DFM_PLUGIN_DIR)
    set(DFM_PLUGIN_DIR ${LIB_INSTALL_DIR}/dde-file-manager/plugins)
endif()

# build plugins dir
if(NOT DEFINED DFM_BUILD_PLUGIN_DIR)
    set(DFM_BUILD_PLUGIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/plugins)
endif()

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
    set(DFM_TOOLS_DIR ${LIB_INSTALL_DIR}/dde-file-manager/tools)
endif()

# dde-file-thumbnail-tool
if(NOT DEFINED DFM_THUMBNAIL_TOOL)
    set(DFM_THUMBNAIL_TOOL ${LIB_INSTALL_DIR}/dde-file-manager/tools)
endif()

# qml module
if(NOT DEFINED DFM_QML_MODULE)
    set(DFM_QML_MODULE ${DFM_PLUGIN_DIR}/qml)
endif()

# qml import path for qt creator
list(APPEND QML_DIRS ${DFM_BUILD_PLUGIN_DIR}/qml)
set(QML_IMPORT_PATH ${QML_DIRS} CACHE STRING "Import local module" FORCE)
