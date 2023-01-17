cmake_minimum_required(VERSION 3.10)

# If do't define version number, specify the version number
set (VERSION "1.0.0" CACHE STRING "define project version")

set(BIN_NAME dfm-extension)

project(${BIN_NAME}
    LANGUAGES CXX
    HOMEPAGE_URL https://github.com/linuxdeepin/dde-file-manager
    DESCRIPTION "Extension library of dde-file-manager which is provided for third parties"
)

# specify install dir
include(GNUInstallDirs)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Wextra")
set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} "-Wl,--as-needed")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Install settings
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

set(CMAKE_CXX_FLAGS Release CACHE STRING "define CXXFLAGS")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_TESTING ON)
#   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
endif ()

add_subdirectory(src)

# TODO: docs

# TODO: tests

# config pkgconfig file
configure_file(misc/${BIN_NAME}.pc.in ${BIN_NAME}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BIN_NAME}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
# config cmake file
# TODO: install cmake file use debian
configure_file(misc/${BIN_NAME}Config.cmake.in ${BIN_NAME}Config.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BIN_NAME}Config.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${BIN_NAME})

set(ASSERTS_DIR ../../assets)
set(EXTENSIONS_LIB_DIR ${DFM_PLUGIN_DIR}/extensions)
install(FILES ${ASSERTS_DIR}/.readme DESTINATION ${EXTENSIONS_LIB_DIR})
