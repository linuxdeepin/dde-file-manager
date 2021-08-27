cmake_minimum_required(VERSION 3.1.0)

# 定义可执行程序名称
project(dfm-common-service)

set(${PROJECT_NAME}_global_h
    dfm_common_service_global.h)

set(${PROJECT_NAME}_event_h
    )

set(${PROJECT_NAME}_event_cpp
    )

set(${PROJECT_NAME}_job_h
    job/abstractjobhandler.h
    )

set(${PROJECT_NAME}_job_cpp
    job/abstractjobhandler.cpp
    )

add_library(${PROJECT_NAME}
    SHARED
    ${${PROJECT_NAME}_global_h}
    ${${PROJECT_NAME}_event_h}
    ${${PROJECT_NAME}_event_cpp}
    ${${PROJECT_NAME}_job_h}
    ${${PROJECT_NAME}_job_cpp}
    )

target_link_libraries(
    ${PROJECT_NAME}
    ${QtUseModules}
    ${PkgUseModules}
    )
