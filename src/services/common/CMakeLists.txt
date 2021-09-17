cmake_minimum_required(VERSION 3.1.0)

# 定义可执行程序名称
project(dfmservice-common)

set(GLOBAL_H
    dfm_common_service_global.h)

set(EVENT_H
    eventservice/laucheventreceiver.h
    eventservice/fileeventreceiver.h
    )

set(EVENT_CPP
    eventservice/laucheventreceiver.cpp
    eventservice/fileeventreceiver.cpp
    )

set(JOB_H
    jobservice/abstractjobhandler.h
    )

set(JOB_CPP
    jobservice/abstractjobhandler.cpp
    )

add_library(${PROJECT_NAME}
    SHARED
    ${GLOBAL_H}
    ${EVENT_H}
    ${EVENT_CPP}
    ${JOB_H}
    ${JOB_CPP}
    )

target_link_libraries(
    ${PROJECT_NAME}
    ${QtUseModules}
    ${PkgUseModules}
    )
