cmake_minimum_required(VERSION 3.10)
project(dtkdciicon-preview)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB DCIICONPREVIEW_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.xml"
    "${GlobalFiles}"
    )
find_package(Dtk COMPONENTS Widget REQUIRED)

add_library(${PROJECT_NAME}
    SHARED
    ${DCIICONPREVIEW_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../previews)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${DtkGui_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    ${DtGui_LIBRARIES}
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/dtk-dciicon-preview-plugin.json DESTINATION ../../previews)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dtk-dciicon-preview-plugin.json DESTINATION ${DFM_PLUGIN_PREVIEW_DIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dtk-dci.xml DESTINATION ${CMAKE_INSTALL_PREFIX}/share/mime/packages)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${DFM_PLUGIN_PREVIEW_DIR})
