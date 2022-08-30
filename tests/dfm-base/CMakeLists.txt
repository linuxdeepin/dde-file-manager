cmake_minimum_required(VERSION 3.10)

project(test-dfm-base)

set(SourcePath ${PROJECT_SOURCE_PATH}/dfm-base/)

ADD_DEFINITIONS(-DENABLE_QUICK_SEARCH)

# UT文件
file(GLOB_RECURSE UT_CXX_FILE
    FILES_MATCHING PATTERN "*.cpp" "*.h")
file(GLOB_RECURSE SRC_FILES
    FILES_MATCHING PATTERN "${SourcePath}/*.cpp" "${SourcePath}/*.h")

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Concurrent REQUIRED)
find_package(Qt5 COMPONENTS DBus REQUIRED)
find_package(Qt5 COMPONENTS Sql REQUIRED)
find_package(Dtk REQUIRED Widget REQUIRED)
find_package(Qt5 COMPONENTS X11Extras REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(KF5Codecs REQUIRED)
find_package(X11 REQUIRED)

pkg_search_module(dfm-burn
    REQUIRED
    dfm-burn
    IMPORTED_TARGET
)

pkg_search_module(dfm-io
    REQUIRED
    dfm-io
    IMPORTED_TARGET
)

pkg_search_module(dfm-mount
    REQUIRED
    dfm-mount
    IMPORTED_TARGET
)

pkg_search_module(gsettings
    REQUIRED
    gsettings-qt
    IMPORTED_TARGET
)

pkg_search_module(libmount
    REQUIRED
    mount
    IMPORTED_TARGET
)

add_executable(${PROJECT_NAME}
    ${SRC_FILES}
    ${UT_CXX_FILE}
    ${CPP_STUB_SRC}
)

find_package(Dtk REQUIRED Widget REQUIRED)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${SourcePath}
    ${SourcePath}/dbusservice
    ${DtkWidget_INCLUDEDIRS}
    KF5/kCodecs
    ${Qt5Widgets_PRIVATE_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Widgets
    Qt5::Concurrent
    Qt5::DBus
    Qt5::X11Extras
    Qt5::Sql
    PkgConfig::dfm-burn
    PkgConfig::dfm-io
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
