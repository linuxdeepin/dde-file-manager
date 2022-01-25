cmake_minimum_required(VERSION 3.10)

project(test-dfm-base)

add_executable(${PROJECT_NAME}
    main.cpp
    utils/ut_fileutils.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    DFM::base
)

add_test(
  NAME base
  COMMAND $<TARGET_FILE:test-dfm-base>
)
