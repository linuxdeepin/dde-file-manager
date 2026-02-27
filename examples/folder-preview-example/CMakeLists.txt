cmake_minimum_required(VERSION 3.10)
project(dde-folder-preview-plugin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# ── 依赖 ──────────────────────────────────────────────────────────────
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Concurrent)
find_package(dfm6-base REQUIRED)

# ── 源码 ──────────────────────────────────────────────────────────────
file(GLOB PLUGIN_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

add_library(${PROJECT_NAME} SHARED ${PLUGIN_SOURCES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Concurrent
        dfm6-base
)

# ── 安装 ──────────────────────────────────────────────────────────────
include(GNUInstallDirs)

if(NOT DEFINED DFM_PLUGIN_PREVIEW_DIR)
    set(DFM_PLUGIN_PREVIEW_DIR
        ${CMAKE_INSTALL_FULL_LIBDIR}/dde-file-manager/plugins/previews
    )
endif()

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${DFM_PLUGIN_PREVIEW_DIR}
)

install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/dde-folder-preview-plugin.json
    DESTINATION ${DFM_PLUGIN_PREVIEW_DIR}
)
