# 共用的配置
FILE(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    )

configure_file(
    "${CMAKE_SOURCE_DIR}/src/apps/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    )

# 通用的库配置函数
function(configure_upgrade_library BIN_NAME)
    add_library(${BIN_NAME}
        SHARED
        ${SRC_FILES}
    )

    set_target_properties(${BIN_NAME} PROPERTIES 
        LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_TOOLS_DIR})

    target_include_directories(${BIN_NAME}
        PUBLIC
            ${CMAKE_CURRENT_BINARY_DIR}
    )

    install(TARGETS
        ${BIN_NAME}
        LIBRARY
        DESTINATION
        ${DFM_LIB_DIR}/tools
    )
endfunction() 
