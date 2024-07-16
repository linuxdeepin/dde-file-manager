cmake_minimum_required(VERSION 3.10)

set(BIN_NAME "deepin-anything-service")
project(${BIN_NAME})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB SOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

add_executable(${BIN_NAME}
    ${SOURCE_FILES}
)

find_package(Qt5 COMPONENTS
    DBus
    REQUIRED
)

target_link_libraries(${BIN_NAME}
    DFM::framework
)

install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${BIN_NAME}.json DESTINATION share/deepin-service-manager/other/)
install(FILES org.deepin.Filemanager.Anything.conf DESTINATION share/dbus-1/system.d/)
install(FILES org.deepin.Filemanager.Anything.service DESTINATION share/dbus-1/system-services/)
