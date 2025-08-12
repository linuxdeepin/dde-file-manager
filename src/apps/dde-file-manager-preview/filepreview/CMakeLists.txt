cmake_minimum_required(VERSION 3.10)

set(BIN_NAME "dde-file-manager-preview")

configure_file(
    "${DFM_APP_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    )

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(Dtk6 COMPONENTS Widget REQUIRED)

FILE(GLOB_RECURSE FILEPREVIEW_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    )

add_executable(${BIN_NAME}
    ${FILEPREVIEW_FILES}
)

# Skip build rpath for reproducible release build
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(${BIN_NAME} PROPERTIES SKIP_BUILD_RPATH TRUE)
endif()

target_link_libraries(${BIN_NAME} PRIVATE
    Qt6::Core
    Dtk6::Widget
    DFM6::base
)

install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
