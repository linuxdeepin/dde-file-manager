cmake_minimum_required(VERSION 3.10)

# If do't define version number, specify the version number
set (VERSION "1.0.0" CACHE STRING "define project version")

set(BIN_NAME dfm-gui)

if (NOT VERSION)
    set(VERSION "1.0.0")
endif()

if (NOT PROJECT_VERSION_MAJOR)
    set(PROJECT_VERSION_MAJOR 1)
endif()

# signals and slots keyword is disable, use Q_SIGNALS and Q_SLOTS instead
add_compile_definitions(QT_NO_SIGNALS_SLOTS_KEYWORDS)
add_compile_definitions(THUMBNAIL_TOOL_DIR="${DFM_THUMBNAIL_TOOL}")

# add qrc resources
set(QRC_FILES)

if(DFM_BUILD_WITH_QT6)
    qt_add_resources(QRC_RESOURCES ${QRC_FILES})
else()
    qt5_add_resources(QRC_RESOURCES ${QRC_FILES})
endif()

# add code
file(GLOB_RECURSE INCLUDE_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/${BIN_NAME}/*")
file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS
    "./*.h"
    "./*.hpp"
    "./*.cpp"
)

# add depends
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Quick REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Concurrent REQUIRED)

# for generating middle source files of SettingsTemplate to translate.
set(TRANS_OF_SETTINGS_CPP)
set (DTK_SETTINGS_TOOLS_EXECUTABLE ${DTKCORE_TOOL_DIR}/dtk-settings)
if (EXISTS ${DTK_SETTINGS_TOOLS_EXECUTABLE})
    FILE (GLOB SETTING_TEMPALTE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/qrc/configure/*.js")

    foreach(temp ${SETTING_TEMPALTE_FILES})
#        message("---- found template: ${temp}")
        string(REPLACE ".js" "-trans.cpp" TARGET_CPP ${temp})
#        message("---- convert to ${TARGET_CPP}")
        execute_process(COMMAND ${DTK_SETTINGS_TOOLS_EXECUTABLE} ${temp} -o ${TARGET_CPP})

        string(REPLACE "/" ";" PATH_FRAGS ${TARGET_CPP})
        list(GET PATH_FRAGS -1 FUNC_NAME)
        string(REPLACE ".cpp" "" FUNC_NAME ${FUNC_NAME})
        string(REPLACE "-" "_" FUNC_NAME ${FUNC_NAME})
        execute_process(COMMAND sed -i "5s/GenerateSettingTranslate/${FUNC_NAME}/" ${TARGET_CPP})

        list(APPEND TRANS_OF_SETTINGS_CPP ${TARGET_CPP})
    endforeach()

#    message(">>>> cpp source files for translating are generated: ${TRANS_OF_SETTINGS_CPP}")
    list(APPEND TRANS_OF_SETTINGS_CPP
        ${CMAKE_CURRENT_SOURCE_DIR}/qrc/configure/global-setting-template-manully-trans.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qrc/configure/global-setting-template-dfmio-trans.cpp)
endif()

# build
add_library(${BIN_NAME}
    SHARED
    ${QRC_RESOURCES}
    ${INCLUDE_FILES}
    ${SRCS}
)

target_link_libraries(${BIN_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Concurrent
    DFM::base
)

target_include_directories(${BIN_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

set(ShareDir ${CMAKE_INSTALL_PREFIX}/share/dde-file-manager) # also use for install
target_compile_definitions(
    ${BIN_NAME}
    PRIVATE
    APPSHAREDIR="${ShareDir}"
)

add_library(DFM::gui ALIAS ${BIN_NAME})

set_target_properties(${BIN_NAME} PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# install library file
install(TARGETS ${BIN_NAME} LIBRARY DESTINATION  ${LIB_INSTALL_DIR})
# install header files
install(DIRECTORY
    ${PROJECT_SOURCE_DIR}/include/${BIN_NAME}
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# for pc file config
set(PC_LIBS_PRIVATE Qt${QT_VERSION_MAJOR}Core)
set(PC_REQ_PRIVATE)
set(PC_REQ_PUBLIC)

# config pkgconfig file
configure_file(${PROJECT_SOURCE_DIR}/assets/dev/${BIN_NAME}/${BIN_NAME}.pc.in ${BIN_NAME}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BIN_NAME}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# config cmake file
configure_file(${PROJECT_SOURCE_DIR}/assets/dev/${BIN_NAME}/${BIN_NAME}Config.cmake.in ${BIN_NAME}Config.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BIN_NAME}Config.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${BIN_NAME})
