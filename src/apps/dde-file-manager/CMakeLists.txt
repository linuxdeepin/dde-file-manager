cmake_minimum_required(VERSION 3.1.0)

# 定义可执行程序名称
set(BIN_NAME dde-file-manager)

set(SRCS
    main.cpp
    )

add_executable(${BIN_NAME} ${SRCS})

target_link_libraries(
    ${BIN_NAME}
    ${QtUseModules}
    ${PkgUseModules}
    )

# TODO(zhangs): install

# TODO(zhangs): translation
