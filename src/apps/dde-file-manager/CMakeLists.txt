cmake_minimum_required(VERSION 3.10)

project(dde-file-manager)

message("PROJECT_BINARY_DIR: " ${PROJECT_BINARY_DIR})

configure_file(
    "${DFM_APP_SOURCE_DIR}/config.h.in"
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
    ${CMAKE_CURRENT_SOURCE_DIR}/dragmonitor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dragmonitor.h
    )

find_package(Qt6 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt6 COMPONENTS Network REQUIRED)
find_package(Dtk6 COMPONENTS Widget REQUIRED)


add_executable(${PROJECT_NAME} ${SRCS})

target_include_directories(${PROJECT_NAME} 
    PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    )

target_link_libraries(
    ${PROJECT_NAME}
    Qt6::Widgets
    Qt6::Network
    DFM6::base
    DFM6::framework
    Dtk6::Widget
)

# Enable position-independent executables for improved security
target_link_options(${PROJECT_NAME} PRIVATE -pie)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})

# sh
install(FILES dde-property-dialog DESTINATION bin)

# desktop
install(FILES dde-file-manager.desktop DESTINATION share/applications)
install(FILES dde-open.desktop DESTINATION share/applications)

# pkexec
install(FILES pkexec/com.deepin.pkexec.dde-file-manager.policy DESTINATION share/polkit-1/actions/)
install(PROGRAMS pkexec/dde-file-manager-pkexec DESTINATION bin)

# manual
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets/manual/dde-file-manager
    DESTINATION share/deepin-manual/manual-assets/application)

# translations
set(TS_DIR "${CMAKE_SOURCE_DIR}/translations")
file(GLOB TS_FILES "${TS_DIR}/*.ts")
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}")
qt6_add_translation(QM_FILES ${TS_FILES})
add_custom_target(${PROJECT_NAME}_translations ALL DEPENDS ${QM_FILES})
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_translations)

install(FILES ${QM_FILES} DESTINATION share/dde-file-manager/translations)
