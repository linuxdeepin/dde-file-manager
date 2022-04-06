cmake_minimum_required(VERSION 3.10)

project(ddplugin-background)

set(CMAKE_INCLUDE_CURRENT_DIR ON)


FILE(GLOB_RECURSE SCREEN_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

add_library(${PROJECT_NAME}
    SHARED
    ${SCREEN_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

find_package(Dtk REQUIRED Widget REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus )

target_include_directories(${PROJECT_NAME} PUBLIC
    ${DFrameworkDBus_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    DFMSERVICE::common
    DFMSERVICE::desktop
    ${DFrameworkDBus_LIBRARIES}
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DIR}
)
