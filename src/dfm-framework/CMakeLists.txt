cmake_minimum_required(VERSION 3.0.2)

project(dfm-framework)

#set Module subdir
#   The module pattern cannot be broken, 
#   so the installation header file cannot be found
set(lifecycle ${CMAKE_CURRENT_SOURCE_DIR}/lifecycle)
set(listener ${CMAKE_CURRENT_SOURCE_DIR}/listener)
set(service ${CMAKE_CURRENT_SOURCE_DIR}/service)
set(event ${CMAKE_CURRENT_SOURCE_DIR}/event)
set(abus ${CMAKE_CURRENT_SOURCE_DIR}/abus)
set(log ${CMAKE_CURRENT_SOURCE_DIR}/log)
set(listener ${CMAKE_CURRENT_SOURCE_DIR}/listener)

set(${PROJECT_NAME}_global_g
    dfm_framework_global.h
    )

set(${PROJECT_NAME}_cxx_private_h
    lifecycle/private/pluginmanager_p.h
    lifecycle/private/pluginmetaobject_p.h
    abus/private/appbus_p.h
    event/private/event_p.h
    event/private/eventhandler_p.h
    )

set(${PROJECT_NAME}_cxx_private_cpp
    lifecycle/private/pluginmanager_p.cpp
    abus/private/appbus_p.cpp
    )

set(${PROJECT_NAME}_cxx_public_h
    event/event.h
    event/eventcallproxy.h
    event/eventhandler.h
    log/frameworklog.h
    log/codetimecheck.h
    log/logutils.h
    service/pluginserviceglobal.h
    service/pluginservice.h
    lifecycle/pluginsetting.h
    lifecycle/pluginmetaobject.h
    lifecycle/plugincontext.h
    lifecycle/plugin.h
    lifecycle/pluginmanager.h
    lifecycle/lifecycle.h
    lifecycle/plugindepend.h
    abus/appbus.h
    listener/listener.h
    )

set(${PROJECT_NAME}_cxx_public_cpp
    event/event.cpp
    event/eventcallproxy.cpp
    event/eventhandler.cpp
    service/pluginserviceglobal.cpp
    log/frameworklog.cpp
    log/codetimecheck.cpp
    log/logutils.cpp
    lifecycle/pluginsetting.cpp
    lifecycle/pluginmetaobject.cpp
    lifecycle/plugincontext.cpp
    lifecycle/plugin.cpp
    lifecycle/pluginmanager.cpp
    lifecycle/lifecycle.cpp
    abus/appbus.cpp
    listener/listener.cpp
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
