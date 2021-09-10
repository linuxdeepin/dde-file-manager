cmake_minimum_required(VERSION 3.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-need -fPIE")

# 工程名称
project(test-dfm-framework)

# 依赖包
find_package(PkgConfig REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkGui REQUIRED)
find_package(DtkCMake REQUIRED)
find_package(Qt5 COMPONENTS
    Core
    Gui
    Widgets
    Concurrent
    DBus
    Network
    REQUIRED)

set(Qt_LIBS
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::DBus
    Qt5::Concurrent
    Qt5::Network
    )

set(PROJECT_SRC_PATH "${PROJECT_SOURCE_PATH}/apps/dde-file-manager")
include_directories(${PROJECT_SRC_PATH})

# UT文件
file(GLOB_RECURSE UT_CXX_FILE
    FILES_MATCHING PATTERN "*.cpp") #public dir

# 源工程文件
set(SRC_DIR ${CMAKE_SOURCE_DIR}/../src/dfm-framework)
include_directories(${SRC_DIR}) #include source dir

#set Module subdir
#   The module pattern cannot be broken,
#   so the installation header file cannot be found
set(lifecycle ${SRC_DIR}/lifecycle)
set(listener ${SRC_DIR}/listener)
set(service ${SRC_DIR}/service)
set(event ${SRC_DIR}/event)
set(abus ${SRC_DIR}/abus)
set(log ${SRC_DIR}/log)
set(listener ${SRC_DIR}/listener)

set(${PROJECT_NAME}_cxx_private_h
    ${lifecycle}/private/pluginmanager_p.h
    ${abus}/private/appbus_p.h
    ${event}/private/event_p.h
    ${event}/private/eventhandler_p.h
    )

set(${PROJECT_NAME}_cxx_private_cpp
    ${lifecycle}/private/pluginmanager_p.cpp
    ${abus}/private/appbus_p.cpp
    ${listener}/private/listener_p.h
    )

set(${PROJECT_NAME}_cxx_public_h
    ${event}/event.h
    ${event}/eventcallproxy.h
    ${event}/eventhandler.h
    ${log}/frameworklog.h
    ${log}/codetimecheck.h
    ${log}/logutils.h
    ${service}/qtclassmanager.h
    ${service}/qtclassfactory.h
    ${service}/pluginservice.h
    ${lifecycle}/pluginsetting.h
    ${lifecycle}/pluginmetaobject.h
    ${lifecycle}/plugin.h
    ${lifecycle}/pluginmanager.h
    ${lifecycle}/lifecycle.h
    ${abus}/appbus.h
    ${listener}/listener.h
    )

set(${PROJECT_NAME}_cxx_public_cpp
    ${event}/event.cpp
    ${event}/eventcallproxy.cpp
    ${event}/eventhandler.cpp
    ${log}/frameworklog.cpp
    ${log}/codetimecheck.cpp
    ${log}/logutils.cpp
    ${lifecycle}/pluginsetting.cpp
    ${lifecycle}/pluginmetaobject.cpp
    ${lifecycle}/plugin.cpp
    ${lifecycle}/pluginmanager.cpp
    ${lifecycle}/lifecycle.cpp
    ${abus}/appbus.cpp
    ${listener}/listener.cpp
    )

set(SRC_CXX_FILE
    ${${PROJECT_NAME}_cxx_public_h}
    ${${PROJECT_NAME}_cxx_public_cpp}
    )

set(SRC_CXX_PRI_FILE
    ${${PROJECT_NAME}_cxx_private_h}
    ${${PROJECT_NAME}_cxx_private_cpp}
    )

message(SRC_CXX_FILE ${SRC_CXX_FILE})

add_executable(${PROJECT_NAME}
    ${UT_CXX_FILE}
    ${SRC_CXX_FILE}
    ${SRC_CXX_PRI_FILE}
    ${CPP_STUB_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${DtkWidget_INCLUDE_DIRS}
    ${DtkGUI_INCLUDE_DIRS}
    )

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${Qt_LIBS}
    ${DtkWidget_LIBRARIES}
    ${DtkGUI_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${Qt5Test_LIBRARIES}
    pthread
    stdc++fs
    )
