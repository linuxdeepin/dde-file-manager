# Tests 目录 - 包含各种测试/演示程序

# 添加 filescanner 测试/演示程序
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/filescanner/CMakeLists.txt)
    add_subdirectory(filescanner)
endif()

# 可以在此添加更多测试/演示程序
# 例如:
# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/another-test/CMakeLists.txt)
#     add_subdirectory(another-test)
# endif()
