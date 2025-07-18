cmake_minimum_required(VERSION 3.10)

project(filedialog-core-plugin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

# Include shared dependencies configuration
include(${CMAKE_CURRENT_SOURCE_DIR}/dependencies.cmake)

add_library(${PROJECT_NAME}
    SHARED
    ${SRC_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_FILEDIALOG_DIR})

# Apply plugin specific configuration
dfm_setup_filedialog_core_dependencies(${PROJECT_NAME})

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}
)

# install dbus
install(FILES dbus/com.deepin.filemanager.filedialog.xml DESTINATION share/dbus-1/interfaces)
install(FILES dbus/com.deepin.filemanager.filedialogmanager.xml DESTINATION share/dbus-1/interfaces)
