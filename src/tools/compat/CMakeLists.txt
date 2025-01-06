cmake_minimum_required(VERSION 3.10)

project(dfm-compat)

set (VERSION "1.0.0" CACHE STRING "define project version")

set(BIN_NAME dde-file-manager)

# 一些 v5 的库链接了 libdde-file-manager.so.1
# 然而 v6 中已经移除了它，因此这需要创建一个空的库，保证 v5 升级到 v6 的兼容性
find_package(Qt6 REQUIRED COMPONENTS Core)
add_library(${BIN_NAME}_lib SHARED empty.cpp)

if (NOT VERSION)
    set(VERSION "1.0.0")
endif()

if (NOT PROJECT_VERSION_MAJOR)
    set(PROJECT_VERSION_MAJOR 1)
endif()
set_target_properties(${BIN_NAME}_lib PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME ${BIN_NAME}
)
target_compile_definitions(${BIN_NAME}_lib PRIVATE VERSION="${CMAKE_PROJECT_VERSION}")

install(TARGETS ${BIN_NAME}_lib LIBRARY DESTINATION ${LIB_INSTALL_DIR})
