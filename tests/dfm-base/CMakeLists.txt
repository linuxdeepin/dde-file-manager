cmake_minimum_required(VERSION 3.10)

project(test-dfm-base)

set(HeaderPath ${PROJECT_INCLUDE_PATH}/dfm-base/)
set(SourcePath ${PROJECT_SOURCE_PATH}/dfm-base/)

add_compile_definitions(APPSHAREDIR="/usr/share/dde-file-manager")

# UT文件
file(GLOB_RECURSE UT_CXX_FILE
    FILES_MATCHING PATTERN "*.cpp" "*.h")
file(GLOB_RECURSE HEADER_FILES
    FILES_MATCHING PATTERN "${HeaderPath}/*.h"  "${HeaderPath}/*.hpp")
file(GLOB_RECURSE SRC_FILES
    FILES_MATCHING PATTERN "${SourcePath}/*.cpp" "${SourcePath}/*.h" "${SourcePath}/*.hpp")

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Concurrent REQUIRED)
find_package(Qt5 COMPONENTS DBus REQUIRED)
find_package(Qt5 COMPONENTS Sql REQUIRED)
find_package(Dtk COMPONENTS Widget REQUIRED)
find_package(Qt5 COMPONENTS X11Extras REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(KF5Codecs REQUIRED)
find_package(X11 REQUIRED)
find_package(Dtk COMPONENTS Widget REQUIRED)

pkg_search_module(libmount REQUIRED mount IMPORTED_TARGET)
pkg_search_module(dfm-burn REQUIRED dfm-burn IMPORTED_TARGET)
pkg_search_module(dfm-io REQUIRED dfm-io IMPORTED_TARGET)
pkg_search_module(dfm-mount REQUIRED dfm-mount IMPORTED_TARGET)
pkg_search_module(gsettings REQUIRED gsettings-qt IMPORTED_TARGET)
pkg_search_module(Dtk REQUIRED dtkcore IMPORTED_TARGET)

qt5_add_dbus_interface(SRC_FILES ${DFM_DBUS_XML_DIR}/org.deepin.filemanager.server.DeviceManager.xml devicemanager_interface)

add_executable(${PROJECT_NAME}
    ${HEADER_FILES}
    ${SRC_FILES}
    ${UT_CXX_FILE}
    ${CPP_STUB_SRC}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_INCLUDE_PATH}
    ${SourcePath}
    ${DtkWidget_INCLUDEDIRS}
    ${Qt5Widgets_PRIVATE_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Widgets
    Qt5::Concurrent
    Qt5::DBus
    Qt5::X11Extras
    Qt5::Sql
    PkgConfig::dfm-io
    PkgConfig::dfm-burn
    PkgConfig::dfm-mount
    PkgConfig::gsettings
    PkgConfig::libmount
    poppler-cpp
    KF5::Codecs
    ${DtkWidget_LIBRARIES}
    ${X11_LIBRARIES}
)

add_test(
  NAME dfm-base
  COMMAND $<TARGET_FILE:${PROJECT_NAME}>
)

