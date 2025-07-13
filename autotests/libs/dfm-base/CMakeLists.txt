cmake_minimum_required(VERSION 3.10)

project(test-dfm-base)

set(SRC_HEADER_PATH ${PROJECT_INCLUDE_PATH}/dfm-base/)
set(SRC_PATH ${PROJECT_SOURCE_PATH}/dfm-base/)

add_compile_definitions(QT_NO_SIGNALS_SLOTS_KEYWORDS)
add_compile_definitions(THUMBNAIL_TOOL_DIR="${DFM_THUMBNAIL_TOOL}")
add_compile_definitions(APPSHAREDIR="${CMAKE_INSTALL_PREFIX}/share/dde-file-manager")

# UT文件
file(GLOB_RECURSE UT_CXX_FILE
    FILES_MATCHING PATTERN "*.cpp" "*.h")
file(GLOB_RECURSE HEADER_FILES
    FILES_MATCHING PATTERN "${SRC_HEADER_PATH}/*.h"  "${SRC_HEADER_PATH}/*.hpp")
file(GLOB_RECURSE SRC_FILES
    FILES_MATCHING PATTERN "${SRC_PATH}/*.cpp" "${SRC_PATH}/*.h" "${SRC_PATH}/*.hpp")

set(QT_VERSION_MAJOR 6)
set(DTK_VERSION_MAJOR 6)

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
qt_add_resources(QRC_RESOURCES ${QRC_FILES})
qt6_add_dbus_interface(Qt6App_dbus
    ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.Daemon.DeviceManager.xml
    devicemanager_interface_qt6)

add_executable(${PROJECT_NAME}
    ${HEADER_FILES}
    ${SRC_FILES}
    ${UT_CXX_FILE}
    ${CPP_STUB_SRC}
)
target_sources(${PROJECT_NAME} PRIVATE ${Qt6App_dbus})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_INCLUDE_PATH}
    ${SRC_PATH}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
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
  ${LIBHEIF_LIBRARIES}
)

add_test(
  NAME dfm-base
  COMMAND $<TARGET_FILE:${PROJECT_NAME}>
)

