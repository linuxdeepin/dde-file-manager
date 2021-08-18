cmake_minimum_required(VERSION 3.0.2)

project(dfm-framework)

#set Module subdir
#   The module pattern cannot be broken, 
#   so the installation header file cannot be found
set(definitions ${CMAKE_CURRENT_SOURCE_DIR}/definitions)
set(lifecycle ${CMAKE_CURRENT_SOURCE_DIR}/lifecycle)
set(listener ${CMAKE_CURRENT_SOURCE_DIR}/listener)
set(service ${CMAKE_CURRENT_SOURCE_DIR}/service)
set(event ${CMAKE_CURRENT_SOURCE_DIR}/event)
set(abus ${CMAKE_CURRENT_SOURCE_DIR}/abus)
set(log ${CMAKE_CURRENT_SOURCE_DIR}/log)

set(${PROJECT_NAME}_cxx_private_h
    ${lifecycle}/private/pluginmanager_p.h
    ${abus}/private/appbus_p.h
    ${event}/private/event_p.h
    )

set(${PROJECT_NAME}_cxx_private_cpp
    ${lifecycle}/private/pluginmanager_p.cpp
    ${abus}/private/appbus_p.cpp
    ${event}/private/event_p.h
    )

set(${PROJECT_NAME}_cxx_public_h
    ${definitions}/globaldefinitions.h
    ${event}/event.h
    ${event}/eventcallproxy.h
    ${log}/frameworklog.h
    ${service}/pluginserviceglobal.h
    ${service}/pluginservice.h
    ${lifecycle}/pluginsetting.h
    ${lifecycle}/pluginmetaobject.h
    ${lifecycle}/plugincontext.h
    ${lifecycle}/plugin.h
    ${lifecycle}/pluginmanager.h
    ${lifecycle}/lifecycle.h
    ${abus}/appbus.h
    )

set(${PROJECT_NAME}_cxx_public_cpp
    ${event}/event.cpp
    ${event}/eventcallproxy.cpp
    ${service}/pluginserviceglobal.cpp
    ${log}/frameworklog.cpp
    ${lifecycle}/pluginsetting.cpp
    ${lifecycle}/pluginmetaobject.cpp
    ${lifecycle}/plugincontext.cpp
    ${lifecycle}/plugin.cpp
    ${lifecycle}/pluginmanager.cpp
    ${lifecycle}/lifecycle.cpp
    ${abus}/appbus.cpp
    )

add_library(${PROJECT_NAME} SHARED
    ${${PROJECT_NAME}_cxx_private_h}
    ${${PROJECT_NAME}_cxx_private_cpp}
    ${${PROJECT_NAME}_cxx_public_h}
    ${${PROJECT_NAME}_cxx_public_cpp}
    )

target_link_libraries(
    ${PROJECT_NAME}
    stdc++fs
    ${QtUseModules}
    ${PkgUseModules}
    )

#install library file
install(TARGETS
    ${PROJECT_NAME}
    DESTINATION
    ${LIBRARY_INSTALL_PREFIX}
    )

#recursion install head file
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
    DESTINATION
    ${INCLUDE_INSTALL_RPEFIX}
    FILES_MATCHING PATTERN "*.h"
    )
