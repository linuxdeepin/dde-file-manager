# add depends
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Widgets Gui Concurrent DBus Sql Network REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR} COMPONENTS Core Widget Gui REQUIRED)
find_package(dfm${DTK_VERSION_MAJOR}-io REQUIRED)
find_package(dfm${DTK_VERSION_MAJOR}-mount REQUIRED)
find_package(dfm${DTK_VERSION_MAJOR}-burn REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(gio REQUIRED gio-unix-2.0 IMPORTED_TARGET)
pkg_check_modules(mount REQUIRED mount IMPORTED_TARGET)
pkg_check_modules(LIBHEIF REQUIRED libheif)

pkg_search_module(X11 REQUIRED x11 IMPORTED_TARGET)
if(${QT_VERSION_MAJOR} EQUAL "6")
    qt_add_resources(QRC_RESOURCES ${QRC_FILES})
    set(DFM_EXTRA_LIBRARIES "")
else()
    qt5_add_resources(QRC_RESOURCES ${QRC_FILES})
    find_package(KF5Codecs REQUIRED)
    find_package(Qt5 COMPONENTS X11Extras REQUIRED)
    pkg_search_module(gsettings REQUIRED gsettings-qt IMPORTED_TARGET)
    set(DFM_EXTRA_LIBRARIES
        Qt${QT_VERSION_MAJOR}::X11Extras
        PkgConfig::gsettings
        PkgConfig::X11
        KF5::Codecs)
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

if(${QT_VERSION_MAJOR} EQUAL "6")
    qt6_add_dbus_interface(Qt6App_dbus
        ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.Daemon.DeviceManager.xml
        devicemanager_interface_qt6)
    target_sources(${BIN_NAME} PRIVATE ${Qt6App_dbus})
    set(DFM_IO_HEADERS ${dfm6-io_INCLUDE_DIR})
    set(DFM_MOUNT_HEADERS ${dfm6-mount_INCLUDE_DIR})
    set(DFM_BURN_HEADERS ${dfm6-burn_INCLUDE_DIR})
else()
    qt5_add_dbus_interface(Qt5App_dbus
        ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.Daemon.DeviceManager.xml
        devicemanager_interface)
    target_sources(${BIN_NAME} PRIVATE ${Qt5App_dbus})
    set(DFM_IO_HEADERS ${dfm-io_INCLUDE_DIR})
    set(DFM_MOUNT_HEADERS ${dfm-mount_INCLUDE_DIR})
    set(DFM_BURN_HEADERS ${dfm-burn_INCLUDE_DIR})
endif()

# TODO: move Widgets to ${DFM_EXTRA_LIBRARIES}
target_link_libraries(${BIN_NAME} 
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Concurrent
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Sql
        Qt${QT_VERSION_MAJOR}::Network
        Dtk${DTK_VERSION_MAJOR}::Core
        Dtk${DTK_VERSION_MAJOR}::Widget
        Dtk${DTK_VERSION_MAJOR}::Gui
        dfm${DTK_VERSION_MAJOR}-io
        dfm${DTK_VERSION_MAJOR}-mount
        dfm${DTK_VERSION_MAJOR}-burn
        PkgConfig::mount
        PkgConfig::gio
        PkgConfig::X11
        poppler-cpp
        ${DFM_EXTRA_LIBRARIES}
        ${LIBHEIF_LIBRARIES}
)

target_include_directories(${BIN_NAME} 
    PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        ${DFM_IO_HEADERS}
        ${DFM_MOUNT_HEADERS}
        ${DFM_BURN_HEADERS}
)

set(ShareDir ${CMAKE_INSTALL_PREFIX}/share/dde-file-manager) # also use for install
target_compile_definitions(
        ${BIN_NAME}
        PRIVATE APPSHAREDIR="${ShareDir}"
        PRIVATE DFM_BASE_INTERNAL_USE=1
)

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


