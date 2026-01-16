cmake_minimum_required(VERSION 3.10)

# Compatibility history issues, GTK check app name
project(dde-select-dialog-x11)

configure_file(
    "${DFM_APP_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    )

set(SRCS
    main.cpp
)

find_package(Qt6 COMPONENTS Core)
add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(
    ${PROJECT_NAME}
    DFM6::base
    DFM6::framework
)

# Enable position-independent executables for improved security
target_link_options(${PROJECT_NAME} PRIVATE -pie)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES data/com.deepin.filemanager.filedialog_x11.service DESTINATION share/dbus-1/services)

