cmake_minimum_required(VERSION 3.10)

project(test-extractor-demo)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 COMPONENTS Core Widgets REQUIRED)
find_package(Dtk6 COMPONENTS Core Widget REQUIRED)

FILE(GLOB_RECURSE TEST_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    )

add_executable(${PROJECT_NAME}
    ${TEST_FILES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Dtk6::Core
    Dtk6::Widget
    dde-file-manager-extractor-lib
)

if(EXISTS "${DFM_APP_SOURCE_DIR}/config.h.in")
    configure_file(
        "${DFM_APP_SOURCE_DIR}/config.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    )
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Install test program
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})

message(STATUS "DFM: extractor-test configured")
