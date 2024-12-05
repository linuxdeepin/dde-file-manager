cmake_minimum_required(VERSION 3.0)

project(encryptManagerDemo)

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
find_package(dfm-framework REQUIRED)
find_package(Dtk COMPONENTS Widget REQUIRED)

file(GLOB_RECURSE SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(${PROJECT_NAME} ${SRC})

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
    ${dfm-framework_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    )
