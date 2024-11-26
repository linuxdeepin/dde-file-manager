cmake_minimum_required(VERSION 3.10)

project(dde-filemanager-textindex)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(TextIndex_XML ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.TextIndex.xml)

FILE(GLOB_RECURSE FILEOPERATIONS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.xml"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.policy"
    )

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core DBus REQUIRED)

# qt5_generate_dbus_interface(
#    textindexdbus.h
#    ${TextIndex_XML}
#    OPTIONS -M -S
# )

qt5_add_dbus_adaptor(FILEOPERATIONS_FILES ${TextIndex_XML}
    textindexdbus.h TextIndexDBus)

#full-text search dependences
pkg_check_modules(Lucene REQUIRED IMPORTED_TARGET liblucene++ liblucene++-contrib)
pkg_check_modules(Docparser REQUIRED IMPORTED_TARGET docparser)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(PCRE REQUIRED libpcre)

add_library(${PROJECT_NAME}
    SHARED
    ${FILEOPERATIONS_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        target_include_directories
        ${GLIB_INCLUDE_DIRS}
        ${PCRE_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::DBus
    DFM::base
    ${GLIB_LIBRARIES}
    ${PCRE_LIBRARIES}
    PkgConfig::Lucene
    PkgConfig::Docparser
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/deepin-service-manager/)
install(FILES ${PROJECT_NAME}.json DESTINATION share/deepin-service-manager/user/)
INSTALL_DBUS_SERVICE(org.deepin.Filemanager.TextIndex)
