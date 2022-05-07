cmake_minimum_required(VERSION 3.10)

project(dfm-framework)

if (NOT VERSION)
    set(VERSION "1.0.0")
endif()

if (NOT PROJECT_VERSION_MAJOR)
    set(PROJECT_VERSION_MAJOR 0)
endif()

add_definitions(-DDPF_LIBRARY)

FILE(GLOB FRAMEWORK_GLOBAL_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    )

FILE(GLOB FRAMEWORK_FEATURE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/backtrace/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/backtrace/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/event/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/event/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/event/dispatcher/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/event/dispatcher/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/event/unicast/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/event/unicast/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/event/sequence/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/event/sequence/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/lifecycle/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lifecycle/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/lifecycle/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lifecycle/private/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/listener/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/listener/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/listener/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/listener/private/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/log/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/log/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/log/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/log/private/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/service/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/service/*.cpp"
    )

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Concurrent REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)
find_package(Dtk COMPONENTS Widget REQUIRED)

add_library(${PROJECT_NAME} SHARED
    ${FRAMEWORK_GLOBAL_FILES}
    ${FRAMEWORK_FEATURE_FILES}
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
