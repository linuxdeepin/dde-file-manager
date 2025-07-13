cmake_minimum_required(VERSION 3.10)
set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTORCC OFF)
set(CMAKE_AUTOUIC OFF)

project(test-dfm-extension)

set(SRC_HEADER_PATH ${PROJECT_INCLUDE_PATH}/dfm-extension/)
set(SRC_PATH ${PROJECT_SOURCE_PATH}/dfm-extension/)

# UT文件
file(GLOB_RECURSE UT_CXX_FILE
    FILES_MATCHING PATTERN "*.cpp" "*.h")
# 源文件
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "${SRC_HEADER_PATH}/*")
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${SRC_PATH}/*.h"
    "${SRC_PATH}/*.cpp"
)

find_package(Qt6 COMPONENTS
    Core
    REQUIRED
)

add_executable(${PROJECT_NAME}
    ${HEADER_FILES}
    ${SRC_FILES}
    ${UT_CXX_FILE}
    ${CPP_STUB_SRC}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_INCLUDE_PATH}
    ${SRC_PATH}
)

target_link_libraries(
    ${PROJECT_NAME} PRIVATE
    Qt6::Core
    ${CMAKE_DL_LIBS}
)

add_test(
  NAME dfm-extension
  COMMAND $<TARGET_FILE:${PROJECT_NAME}>
)
