cmake_minimum_required(VERSION 3.10)

project(dfm-upgrade)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Dtk COMPONENTS Widget REQUIRED)

FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    )
    
add_library(${PROJECT_NAME}
    SHARED
    ${SRC_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../tools)

 target_include_directories(${PROJECT_NAME}
     PUBLIC
         ${DtkWidget_INCLUDE_DIRS}
 )

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    ${DtkWidget_LIBRARIES}
)

install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${LIB_INSTALL_DIR}/dde-file-manager/tools
)
