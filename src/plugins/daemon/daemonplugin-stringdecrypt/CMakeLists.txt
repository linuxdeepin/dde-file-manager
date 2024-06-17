cmake_minimum_required(VERSION 3.10)

project(daemonplugin-stringdecrypt)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB FILEOPERATIONS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.xml"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.policy"
    )

find_package(PkgConfig REQUIRED)
pkg_check_modules(OpenSSL REQUIRED openssl)

add_library(${PROJECT_NAME}
    SHARED
    ${FILEOPERATIONS_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

find_package(Qt5 COMPONENTS
    DBus
    REQUIRED
)

target_link_libraries(${PROJECT_NAME}
    DFM::framework
    DFM::base
    Qt5::DBus
    ${OpenSSL_LIBRARIES}
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DAEMON_EDGE_DIR}
)

# execute_process(COMMAND qdbuscpp2xml stringdecryptdbus.h -o ./stringdecryptdbus.xml
#    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
# execute_process(COMMAND qdbusxml2cpp -i stringdecryptdbus.h -c StringDecryptAdapter -l StringDecryptDBus -a stringdecrypt_adapter stringdecryptdbus.xml
#    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
