cmake_minimum_required(VERSION 3.10)

add_definitions(-DDFM_ORGANIZATION_NAME="deepin")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
#    set(CMAKE_VERBOSE_MAKEFILEON ON)
#    message("debug type open sanitize check")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined,address,leak -fno-omit-frame-pointer")
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined,address,leak -fno-omit-frame-pointer")
#    set(CMAKE_L_FLAGS "${CMAKE_L_FLAGS} -fsanitize=undefined,address,leak -fno-omit-frame-pointer")
endif()

add_subdirectory(dde-desktop)
add_subdirectory(dde-file-manager)
add_subdirectory(dde-file-manager-daemon)
add_subdirectory(dde-file-manager-server)
add_subdirectory(dde-file-dialog)
add_subdirectory(dde-file-dialog-x11)
add_subdirectory(dde-file-dialog-wayland)
