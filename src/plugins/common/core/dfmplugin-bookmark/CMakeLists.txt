cmake_minimum_required(VERSION 3.10)

project(dfmplugin-bookmark)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE BOOKMARK_FILES CONFIGURE_DEPENDS
    "./*.h"
    "./*.cpp"
    "./*.json"
)


add_library(${PROJECT_NAME}
    SHARED
    ${BOOKMARK_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_COMMON_CORE_DIR}
)
