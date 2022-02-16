cmake_minimum_required(VERSION 3.10)

project(ddplugin-devices)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(INTERFACES_H
    devices.h
    devicemanagerdbus.h
    dbus_adaptor/devicemanagerdbus_adaptor.h
)

set(INTERFACES_CPP
    devices.cpp
    devicemanagerdbus.cpp
    dbus_adaptor/devicemanagerdbus_adaptor.cpp
    devices.json
)

add_library(${PROJECT_NAME}
    SHARED
    ${INTERFACES_H}
    ${INTERFACES_CPP}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

# TODO(xust): remove dfm-base depends
target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    DFMSERVICE::common
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DIR}
)
