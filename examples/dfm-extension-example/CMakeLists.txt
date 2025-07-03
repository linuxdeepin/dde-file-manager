cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(dfm-extension-example)

set(CMAKE_CXX_STANDARD 17)

# 只需要依赖 dfm-extension
find_package(dfm-extension REQUIRED)


# 扩展插件源码
file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS
    "./*.h"
    "./*.cpp"
)

# 生成共享库
add_library(${PROJECT_NAME} SHARED ${SRCS})

# 链接 dfm-extension
target_link_libraries(${PROJECT_NAME}
    PUBLIC ${dfm-extension_LIBRARIES}
    )

# 安裝配置
include(GNUInstallDirs)
if(NOT DEFINED LIB_INSTALL_DIR)
    set(LIB_INSTALL_DIR ${CMAKE_INSTALL_FULL_LIBDIR})
endif()

if(NOT DEFINED DFM_EXT_PLUGIN_DIR)
    set(DFM_EXT_PLUGIN_DIR ${LIB_INSTALL_DIR}/dde-file-manager/plugins/extensions)
endif()

# 安裝插件
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${DFM_EXT_PLUGIN_DIR})
