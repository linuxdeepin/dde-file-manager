cmake_minimum_required(VERSION 3.1.0)

project(dfmservice-common)

set(GLOBAL_H
    dfm_common_service_global.h)

set(EVENT_H
    event/laucheventreceiver.h
    event/fileeventreceiver.h
    )

set(EVENT_CPP
    event/laucheventreceiver.cpp
    event/fileeventreceiver.cpp
    )

set(JOB_H
    job/abstractjobhandler.h
    )

set(JOB_CPP
    job/abstractjobhandler.cpp
    )

set(SERVICE_H
    device/deviceservice.h
    )

set(SERVICE_CPP
    device/deviceservice.cpp
    )

add_library(${PROJECT_NAME}
    SHARED
    ${GLOBAL_H}
    ${EVENT_H}
    ${EVENT_CPP}
    ${JOB_H}
    ${JOB_CPP}
    ${SERVICE_H}
    ${SERVICE_CPP}
    )

target_link_libraries(
    ${PROJECT_NAME}
    dfm-framework
    ${QtUseModules}
    ${PkgUseModules}
    )
