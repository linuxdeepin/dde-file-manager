cmake_minimum_required(VERSION 3.10)

set(BIN_NAME "dde-file-manager-extractor")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 COMPONENTS Core REQUIRED)

FILE(GLOB_RECURSE EXTRACTOR_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    )

add_executable(${BIN_NAME}
    ${EXTRACTOR_FILES}
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt6::Core
    DFM6::base
    dde-file-manager-extractor-lib
)

target_include_directories(${BIN_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../libextractor
    ${CMAKE_CURRENT_SOURCE_DIR}/../plugins
)

# Enable position-independent executables for improved security
target_link_options(${BIN_NAME} PRIVATE -pie)

set_target_properties(${BIN_NAME} PROPERTIES
    INSTALL_RPATH ${DFM_PLUGIN_EXTRACTOR_DIR}
)

install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})

message(STATUS "DFM: dde-file-manager-extractor configured")
