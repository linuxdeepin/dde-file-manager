cmake_minimum_required(VERSION 3.10)

project(dfm-upgrade)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(BIN_NAME dfm-upgrade-qt6)

# 共用的配置
FILE(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    )

configure_file(
    "${APP_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    )

# 通用的库配置函数
function(configure_upgrade_library BIN_NAME)
    add_library(${BIN_NAME}
        SHARED
        ${SRC_FILES}
    )

    set_target_properties(${BIN_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_TOOLS_DIR})

    target_include_directories(${BIN_NAME}
        PUBLIC
            ${CMAKE_CURRENT_BINARY_DIR}
    )

    install(TARGETS
        ${BIN_NAME}
        LIBRARY
        DESTINATION
        ${DFM_LIB_DIR}/tools
    )
endfunction()

find_package(Qt6 COMPONENTS Core REQUIRED)

configure_upgrade_library(${BIN_NAME})

target_link_libraries(${BIN_NAME}
    DFM6::base
    DFM6::framework
    Qt6::Core
)
