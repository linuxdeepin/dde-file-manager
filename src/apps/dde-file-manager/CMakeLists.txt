cmake_minimum_required(VERSION 3.10)

project(dde-file-manager)

message("PROJECT_BINARY_DIR: " ${PROJECT_BINARY_DIR})

configure_file(
    "../config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-DDPF_FULLSIG_STRACE_ENABLE)

set(SRCS
    main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/singleapplication.h
    ${CMAKE_CURRENT_SOURCE_DIR}/singleapplication.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/commandparser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/commandparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/private/commandparser_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sessionloader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sessionloader.cpp
    )

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Network REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR} COMPONENTS Widget REQUIRED)

add_executable(${PROJECT_NAME} ${SRCS})

target_include_directories(${PROJECT_NAME} 
    PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    )

if(OPT_ENABLE_QML)
    set(DFM_QML_LIBRARIE DFM::gui)
else()
    set(DFM_QML_LIBRARIE "")
endif()

target_link_libraries(
    ${PROJECT_NAME}
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    DFM::base
    DFM::framework
    Dtk${DTK_VERSION_MAJOR}::Widget
    ${DFM_QML_LIBRARIE}
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})

# sh
install(FILES dde-property-dialog DESTINATION bin)

# desktop
install(FILES dde-file-manager.desktop DESTINATION share/applications)
install(FILES dde-open.desktop DESTINATION share/applications)

# pkexec
install(FILES pkexec/com.deepin.pkexec.dde-file-manager.policy DESTINATION share/polkit-1/actions/)
install(FILES pkexec/dde-file-manager-pkexec DESTINATION bin)

# manual
install(DIRECTORY assets/dde-file-manager DESTINATION share/deepin-manual/manual-assets/application)

# translation
install(DIRECTORY ${CMAKE_SOURCE_DIR}/translations DESTINATION share/${PROJECT_NAME} FILES_MATCHING PATTERN "*.qm")

