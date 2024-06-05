cmake_minimum_required(VERSION 3.10)

project(appletwindow)

add_executable(${PROJECT_NAME} main.cpp
    Panel.qml
    AppletItemEx.qml)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick)

target_link_libraries(
    ${PROJECT_NAME}
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    DFM::framework
    DFM::gui)

file(GLOB_RECURSE QMLS CONFIGURE_DEPENDS
    "./*.qml"
)
EXECUTE_PROCESS(COMMAND cp -f ${QMLS} ${PROJECT_BINARY_DIR})
