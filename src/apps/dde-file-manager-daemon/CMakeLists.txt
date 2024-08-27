cmake_minimum_required(VERSION 3.10)

project(dde-file-manager-daemon)

message("PROJECT_BINARY_DIR: " ${PROJECT_BINARY_DIR})
configure_file(
    "../config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    )

set(SRCS
    main.cpp
)
find_package(Qt5 COMPONENTS Core)
add_executable(${PROJECT_NAME} ${SRCS})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${DtkWidget_INCLUDE_DIRS}
)

target_link_libraries(
    ${PROJECT_NAME}
    DFM::base
    DFM::framework
)


if (COMPLIE_ON_V23)
    if (NOT DEFINED SYSTEMD_USER_UNIT_DIR)
        pkg_get_variable(SYSTEMD_USER_UNIT_DIR systemd systemduserunitdir)
    endif()

    macro(install_symlink filepath wantsdir)
        file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/link/${wantsdir}/)
        execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${SYSTEMD_USER_UNIT_DIR}/${filepath} ${PROJECT_BINARY_DIR}/link/${wantsdir}/${filepath})
        install(FILES ${PROJECT_BINARY_DIR}/link/${wantsdir}/${filepath} DESTINATION ${SYSTEMD_USER_UNIT_DIR}/${wantsdir}/)
    endmacro(install_symlink)

    install_symlink(dde-file-manager.service dde-session-initialized.target.wants)
endif()

# binary
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# services
install(FILES dbusservice/org.deepin.Filemanager.Daemon.service
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/dbus-1/services)

# systemd service
install(FILES dbusservice/dde-file-manager.service
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/user)

