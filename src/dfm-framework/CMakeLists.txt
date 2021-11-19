cmake_minimum_required(VERSION 3.0.2)

project(dfm-framework)

if (NOT VERSION)
    set(VERSION "1.0.0")
endif()

if (NOT PROJECT_VERSION_MAJOR)
    set(PROJECT_VERSION_MAJOR 0)
endif()

add_definitions(-DDPF_LIBRARY)
add_definitions(-DDTK_LOG)

set(LOBAL_H
    dfm_framework_global.h
)

set(PRIVATE_H
    lifecycle/private/pluginmanager_p.h
    lifecycle/private/pluginmetaobject_p.h
    abus/private/appbus_p.h
    event/private/event_p.h
)

set(PRIVATE_CPP
    lifecycle/private/pluginmanager_p.cpp
    abus/private/appbus_p.cpp
)

set(PUBLIC_H
    event/event.h
    event/eventcallproxy.h
    event/eventhandler.h
    log/frameworklog.h
    log/codetimecheck.h
    log/logutils.h
    service/qtclassmanager.h
    service/qtclassfactory.h
    service/pluginservicecontext.h
    service/pluginservice.h
    lifecycle/pluginsetting.h
    lifecycle/pluginmetaobject.h
    lifecycle/plugin.h
    lifecycle/pluginmanager.h
    lifecycle/lifecycle.h
    lifecycle/plugindepend.h
    abus/appbus.h
    listener/private/listener_p.h
    listener/listener.h
    backtrace/backtrace.h
    framework.h
)

set(PUBLIC_CPP
    event/event.cpp
    event/eventcallproxy.cpp
    event/eventhandler.cpp
    log/frameworklog.cpp
    log/codetimecheck.cpp
    log/logutils.cpp
    lifecycle/pluginsetting.cpp
    lifecycle/pluginmetaobject.cpp
    lifecycle/plugin.cpp
    lifecycle/pluginmanager.cpp
    lifecycle/lifecycle.cpp
    service/pluginservicecontext.cpp
    abus/appbus.cpp
    listener/listener.cpp
    backtrace/backtrace.cpp
    framework.cpp
)

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Concurrent REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)
find_package(Dtk COMPONENTS Widget REQUIRED)

add_library(${PROJECT_NAME} SHARED
    ${LOBAL_H}
    ${PRIVATE_H}
    ${PRIVATE_CPP}
    ${PUBLIC_H}
    ${PUBLIC_CPP}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_link_libraries(
    ${PROJECT_NAME}
    Qt5::Core
    Qt5::Concurrent
    Qt5::Network
    ${DtkWidget_LIBRARIES}
)

add_library(DFM::framework ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/../
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${DtkWidget_INCLUDEDIRS}
)

# install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${LIB_INSTALL_DIR}
)

# install head file
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
    PATTERN "private" EXCLUDE
)