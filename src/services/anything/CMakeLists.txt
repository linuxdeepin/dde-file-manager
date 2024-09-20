cmake_minimum_required(VERSION 3.10)

project(dde-filemanager-anything)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Core REQUIRED)

FILE(GLOB SOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

add_library(${PROJECT_NAME}
    SHARED
    ${SOURCE_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_link_libraries(${PROJECT_NAME}
    DFM::framework
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/deepin-service-manager/)
install(FILES ${PROJECT_NAME}.json DESTINATION share/deepin-service-manager/system/)
install(FILES org.deepin.Filemanager.Anything.conf DESTINATION share/dbus-1/system.d/)
