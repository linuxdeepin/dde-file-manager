cmake_minimum_required(VERSION 3.10)

project(ddplugin-background)

set(CMAKE_INCLUDE_CURRENT_DIR ON)


FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

#delete files which are not needed on the current system
if (COMPLIE_ON_V23)
    FILE(GLOB_RECURSE RM_SRC
        "${CMAKE_CURRENT_SOURCE_DIR}/backgroundwm.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/backgroundwm.cpp"
        )
else()
    FILE(GLOB_RECURSE RM_SRC
        "${CMAKE_CURRENT_SOURCE_DIR}/backgrounddde.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/backgrounddde.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/appearance_interface.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/appearance_interface.cpp"
        )
endif()

list(REMOVE_ITEM SRC_FILES ${RM_SRC})

add_library(${PROJECT_NAME}
    SHARED
    ${SRC_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

find_package(Dtk COMPONENTS Widget REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus )

target_include_directories(${PROJECT_NAME} PUBLIC
    ${DFrameworkDBus_INCLUDE_DIRS}
    ${Qt5Widgets_PRIVATE_INCLUDE_DIRS}

)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    ${DFrameworkDBus_LIBRARIES}
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DESKTOP_EDGE_DIR}
)
