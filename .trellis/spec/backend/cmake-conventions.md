# CMake 约定

> DDE 应用的 CMake 构建约定和 Qt6/Qt5 双版本支持。

---

## 概览

本指南定义 DDE 应用 CMake 构建的标准约定，支持 Qt6/Qt5 双版本。

---

## Qt6/Qt5 双版本支持

### 基础配置

```cmake
# 最低 CMake 版本
cmake_minimum_required(VERSION 3.10)

# 项目信息
project(MyApp VERSION 1.0.0)

# 启用 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt 版本检测
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
if(QT_VERSION_MAJOR EQUAL 6)
    set(DTK_VERSION_MAJOR 6)
    set(QT_VERSION_MAJOR 6)
else()
    set(DTK_VERSION_MAJOR "")
    set(QT_VERSION_MAJOR 5)
endif()

message(STATUS "Qt version: ${QT_VERSION_MAJOR}")
message(STATUS "DTK version: ${DTK_VERSION_MAJOR}")

# 查找 Qt 和 DTK
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Concurrent
    DBus
    Sql
    Network
)

find_package(Dtk${DTK_VERSION_MAJOR}Widget REQUIRED)
```

---

## 目标配置

### 可执行文件

```cmake
# 主可执行文件
add_executable(myapp
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
)

target_link_libraries(myapp PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Network
    Dtk${DTK_VERSION_MAJOR}::Widget
)

# 编译定义
target_compile_definitions(myapp PRIVATE
    QT_DISABLE_DEPRECATED_BEFORE=0x050F00
)

# 使用 DTK 命名空间
target_compile_definitions(myapp PRIVATE
    DWIDGET_USE_NAMESPACE
)
```

### 库

```cmake
# 共享库
add_library(mylib SHARED
    src/myclass.cpp
    src/myclass.h
)

# 创建别名便于引用
add_library(MyLib::mylib ALIAS mylib)

target_link_libraries(mylib PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Dtk${DTK_VERSION_MAJOR}::Widget
)

# 导出符号
set_target_properties(mylib PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)
```

---

## 编译选项

### Debug 分支默认启用测试

```cmake
# Debug 模式启用测试选项
option(OPT_ENABLE_BUILD_UT "Enable unit tests" ON)
option(OPT_ENABLE_BUILD_TESTS "Enable build tests" OFF)
option(OPT_DISABLE_QDEBUG "Disable qDebug macro" OFF)

if(OPT_ENABLE_BUILD_UT)
    enable_testing()
    add_subdirectory(tests)
endif()

if(OPT_DISABLE_QDEBUG)
    add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()
```

### 警告级别

```cmake
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()
```

---

## 目录约定

### 标准布局

```
project/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── mainwindow.cpp
│   └── mainwindow.h
├── include/
│   └── myapp/
│       └── myclass.h
├── tests/
│   └── CMakeLists.txt
├── assets/
│   ├── icons/
│   └── translations/
└── cmake/
    └── DFMCommon.cmake
```

### CMake 模块

```cmake
# cmake/DFMCommon.cmake
function(dtk_add_executable name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Dtk${DTK_VERSION_MAJOR}::Widget
    )
    target_compile_definitions(${name} PRIVATE DWIDGET_USE_NAMESPACE)
endfunction()
```

---

## Debian Control 约定

### Qt6 (V25)

```
Source: myapp
Maintainer: Deepin Developers <deepin@uniontech.com>

Build-Depends:
    debhelper-compat (=13),
    qt6-base-dev,
    qt6-tools-dev,
    libdtk6widget-dev (>= 6.0.0),
    libdtk6core-dev (>= 6.0.0),
    libpolkit-qt6-1-dev,
    libglib2.0-dev
```

### Qt5 (V20)

```
Source: myapp
Maintainer: Deepin Developers <deepin@uniontech.com>

Build-Depends:
    debhelper-compat (=12),
    qtbase5-dev,
    qttools5-dev,
    libdtkwidget-dev (>= 2.0.0),
    libdtkcore-dev (>= 2.0.0),
    libpolkit-qt5-1-dev,
    libglib2.0-dev
```

---

## MOC/UIC/RCC 处理

CMake 自动处理 Qt 元对象编译：

```cmake
# CMake 会自动处理
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 如果需要自定义选项
set_property(SOURCE myclass.h PROPERTY AUTOMOC_OPTIONS "-MURI=my.uri")
```

---

## 安装规则

```cmake
# 安装可执行文件
install(TARGETS myapp
    RUNTIME DESTINATION bin
)

# 安装库
install(TARGETS mylib
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# 安装头文件
install(DIRECTORY include/myapp
    DESTINATION include
)

# 安装翻译文件
install(FILES assets/translations/*.qm
    DESTINATION share/${PROJECT_NAME}/translations
)

# 安装桌面文件
install(FILES assets/myapp.desktop
    DESTINATION share/applications
)

# 安装图标
install(FILES assets/icons/myapp.png
    DESTINATION share/icons/hicolor/48x48/apps
)
```

---

## 快速参考

| 任务 | 命令 |
|------|------|
| Qt6/Qt5 双版本 | `find_package(QT NAMES Qt6 Qt5 REQUIRED)` |
| DTK Widget | `find_package(Dtk${DTK_VERSION_MAJOR}Widget REQUIRED)` |
| DTK 命名空间 | `target_compile_definitions(... DWIDGET_USE_NAMESPACE)` |
| 测试 | `enable_testing()` + `add_subdirectory(tests)` |
| 禁用 qDebug | `-DQT_NO_DEBUG_OUTPUT` |
| 安装到 bin | `install(TARGETS myapp RUNTIME DESTINATION bin)` |
