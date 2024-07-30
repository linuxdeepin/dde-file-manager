cmake_minimum_required(VERSION 3.10)

project(dfmplugin-workspace)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE WORKSPACE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

add_library(${PROJECT_NAME}
    SHARED
    ${WORKSPACE_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)
find_package(Dtk${DTK_VERSION_MAJOR} COMPONENTS Widget REQUIRED)

if(DFM_BUILD_WITH_QT6)
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${Qt6Widgets_PRIVATE_INCLUDE_DIRS}
    )
else()
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${Qt5Widgets_PRIVATE_INCLUDE_DIRS}
    )
endif()
target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}
)
