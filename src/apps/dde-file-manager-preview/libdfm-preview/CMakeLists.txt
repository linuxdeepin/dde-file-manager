cmake_minimum_required(VERSION 3.10)

project(dfm-preview)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt6 COMPONENTS Core REQUIRED)

FILE(GLOB_RECURSE FILEPREVIEW_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    )

add_library(${PROJECT_NAME}
    SHARED
    ${FILEPREVIEW_FILES}
)

set(VERSION 1.8.2)
add_definitions(-DVERSION=\"${VERSION}\")

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ./)

# Skip build rpath for reproducible release build
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(${PROJECT_NAME} PROPERTIES SKIP_BUILD_RPATH TRUE)
endif()

target_link_libraries(${PROJECT_NAME}
    DFM6::base
    DFM6::framework
)

#install library file
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${DFM_PLUGIN_PREVIEW_DIR})
INSTALL_DCONFIG("org.deepin.dde.file-manager.preview.json")

