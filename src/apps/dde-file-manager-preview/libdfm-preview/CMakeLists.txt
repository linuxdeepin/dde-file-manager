cmake_minimum_required(VERSION 3.10)

project(dfm-preview)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5 COMPONENTS Core REQUIRED)

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

target_link_libraries(${PROJECT_NAME}
    DFM${DTK_VERSION_MAJOR}::base
    DFM${DTK_VERSION_MAJOR}::framework
)

#install library file
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${DFM_PLUGIN_PREVIEW_DIR})
INSTALL_DCONFIG("org.deepin.dde.file-manager.preview.json")

