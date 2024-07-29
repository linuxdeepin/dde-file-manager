cmake_minimum_required(VERSION 3.10)

project(dde-file-dialog)

configure_file(
    "../config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    )

set(SRCS
    main.cpp
)

add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(
    ${PROJECT_NAME}
    DFM::base
    DFM::framework
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES data/com.deepin.filemanager.filedialog.service DESTINATION share/dbus-1/services)

