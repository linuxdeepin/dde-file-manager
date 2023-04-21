cmake_minimum_required(VERSION 3.10)

project(test-dfm-framework)

set(HeaderPath ${PROJECT_INCLUDE_PATH}/dfm-framework/)
set(SourcePath ${PROJECT_SOURCE_PATH}/dfm-framework/)
# UT文件
file(GLOB_RECURSE UT_CXX_FILE
    FILES_MATCHING PATTERN "*.cpp" "*.h")
# 源文件
file(GLOB_RECURSE HEADER_FILES
    FILES_MATCHING PATTERN "${HeaderPath}/event/*.h"
    "${HeaderPath}/lifecycle/*.h" "${HeaderPath}/listener/*.h")
file(GLOB_RECURSE SRC_FILES
    FILES_MATCHING PATTERN "${SourcePath}/event/*.cpp" "${SourcePath}/event/*.h"
     "${SourcePath}/lifecycle/*.cpp" "${SourcePath}/lifecycle/*.h"
     "${SourcePath}/listener/*.cpp" "${SourcePath}/listener/*.h")

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Concurrent REQUIRED)
find_package(Dtk COMPONENTS Core REQUIRED)

add_executable(${PROJECT_NAME}
    ${HEADER_FILES}
    ${SRC_FILES}
    ${UT_CXX_FILE}
    ${CPP_STUB_SRC}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_INCLUDE_PATH}
    ${SourcePath}
)

target_link_libraries(
    ${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Concurrent
    ${DtkCore_LIBRARIES}
    ${CMAKE_DL_LIBS}
)

add_test(
  NAME dfm-framework
  COMMAND $<TARGET_FILE:${PROJECT_NAME}>
)
