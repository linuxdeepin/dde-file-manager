cmake_minimum_required(VERSION 3.1.0)

project(dde-file-manager)


# Setup the environment
SET(CMAKE_CXX_STANDARD 17)
if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add subdirectory
add_subdirectory(${PROJECT_SOURCE_DIR}/src)
