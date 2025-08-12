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

# Skip build rpath for reproducible release build
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(${PROJECT_NAME} PROPERTIES SKIP_BUILD_RPATH TRUE)
endif()

target_link_libraries(
    ${PROJECT_NAME}
    DFM6::base
    DFM6::framework
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES data/com.deepin.filemanager.filedialog_x11.service DESTINATION share/dbus-1/services)

