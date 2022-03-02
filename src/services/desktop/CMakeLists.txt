cmake_minimum_required(VERSION 3.10)

project(dfmservice-desktop)

if (NOT VERSION)
    set(VERSION "1.0.0")
endif()

if (NOT PROJECT_VERSION_MAJOR)
    set(PROJECT_VERSION_MAJOR 0)
endif()

FILE(GLOB GLOBAL_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

FILE(GLOB_RECURSE SCREEN_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/screen/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/screen/*.cpp"
)

FILE(GLOB_RECURSE CANVAS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/canvas/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/canvas/*.cpp"
)

FILE(GLOB_RECURSE BACKGROUND_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/background/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/background/*.cpp"
)

FILE(GLOB_RECURSE FRAME_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/frame/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/frame/*.cpp"
)

FILE(GLOB_RECURSE WALLPAPER_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/wallpapersetting/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/wallpapersetting/*.cpp"
)
FILE(GLOB_RECURSE EVENT_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(${PROJECT_NAME}
    SHARED
    ${GLOBAL_FILES}
    ${SCREEN_FILES}
    ${CANVAS_FILES}
    ${BACKGROUND_FILES}
    ${FRAME_FILES}
    ${WALLPAPER_FILES}
    ${EVENT_FILES}
    )

add_library(DFMSERVICE::desktop ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${LIB_INSTALL_DIR}
)

# install head file
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION include/dfmservice-desktop
    FILES_MATCHING PATTERN "*.h"
    PATTERN "private" EXCLUDE
)
