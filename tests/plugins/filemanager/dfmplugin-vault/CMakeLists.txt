cmake_minimum_required(VERSION 3.10)

project(test-dfmplugin-vault)

set(PluginPath ${PROJECT_SOURCE_PATH}/plugins/filemanager/dfmplugin-vault)
file(GLOB_RECURSE PLUGIN_FILES
    FILES_MATCHING PATTERN "${PluginPath}/*.cpp" "${PluginPath}/*.h")
file(GLOB_RECURSE SRC_FILES
    FILES_MATCHING PATTERN "*.cpp" "*.h")
add_executable(${PROJECT_NAME}
    ${PLUGIN_FILES}
    ${SRC_FILES}
    ${CPP_STUB_SRC}
)

find_package(Dtk COMPONENTS Widget REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(polkit REQUIRED polkit-agent-1 polkit-qt5-1)
pkg_check_modules(openssl REQUIRED libcrypto)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PluginPath}
    ${DtkWidget_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    DFM::base
    DFM::framework
    ${DtkWidget_LIBRARIES}
    ${polkit_LIBRARIES}
    ${openssl_LIBRARIES}
)

add_test(
    NAME vault
    COMMAND $<TARGET_FILE:${PROJECT_NAME}>
)
