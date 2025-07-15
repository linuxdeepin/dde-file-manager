# Resource files
if(${QT_VERSION_MAJOR} EQUAL "6")
    qt_add_resources(QRC_RESOURCES ${QRC_FILES})
else()
    qt5_add_resources(QRC_RESOURCES ${QRC_FILES})
endif()

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

# Configure library using unified configuration function
dfm_configure_base_library(${BIN_NAME})

add_library(DFM${DTK_VERSION_MAJOR}::base ALIAS ${BIN_NAME})

set_target_properties(${BIN_NAME} PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# install library file
install(TARGETS ${BIN_NAME} EXPORT ${BIN_NAME}Targets
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT ${BIN_NAME}Targets
    FILE ${BIN_NAME}Targets.cmake
    NAMESPACE DFM${DTK_VERSION_MAJOR}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${BIN_NAME}
)

# for pc file config
set(PC_LIBS_PRIVATE Qt${QT_VERSION_MAJOR}Core)
set(PC_REQ_PRIVATE "")
set(PC_REQ_PUBLIC "dfm${QT_VERSION_MAJOR}-io dfm${QT_VERSION_MAJOR}-mount dfm${QT_VERSION_MAJOR}-burn Qt${QT_VERSION_MAJOR}Core Qt${QT_VERSION_MAJOR}Widgets Qt${QT_VERSION_MAJOR}Gui Qt${QT_VERSION_MAJOR}Concurrent Qt${QT_VERSION_MAJOR}DBus Qt${QT_VERSION_MAJOR}Sql Qt${QT_VERSION_MAJOR}Network dtk${DTK_VERSION_MAJOR}core dtk${DTK_VERSION_MAJOR}gui dtk${DTK_VERSION_MAJOR}widget")

# config pkgconfig file
configure_file(${PROJECT_SOURCE_DIR}/assets/dev/${BIN_NAME}/${BIN_NAME}.pc.in ${BIN_NAME}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BIN_NAME}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# config cmake file
configure_file(${PROJECT_SOURCE_DIR}/assets/dev/${BIN_NAME}/${BIN_NAME}Config.cmake.in ${BIN_NAME}Config.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BIN_NAME}Config.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${BIN_NAME})


