# DDE 文件管理器核心功能扩展插件规范

## 概述

dfm-extension 是 DDE 文件管理器提供的一套 C++ 插件接口，允许开发者深度扩展文件管理器的核心功能。通过实现标准化的接口，插件可以扩展右键菜单、添加文件角标、监听窗口事件、拦截文件操作等。

本规范定义了插件接口的实现方式、内存管理规则、生命周期管理以及与文件管理器主程序的交互约定。

### 扩展能力

dfm-extension 支持以下四种扩展类型：

| 扩展类型 | 功能描述 | 适用场景 |
|---------|---------|---------|
| Menu Plugin | 扩展右键菜单，添加自定义菜单项和子菜单 | 添加文件操作快捷方式、集成第三方工具 |
| EmblemIcon Plugin | 为文件图标添加角标标识 | 文件状态标记、分类标签、重要程度标识 |
| Window Plugin | 监听文件管理器窗口生命周期事件 | 窗口状态同步、插件生命周期管理 |
| File Plugin | 拦截文件操作，自定义打开行为 | 特定文件类型处理、自定义打开程序 |

### 设计原则

1. **ABI 稳定性**：使用 pimpl 技术隐藏实现细节，避免二进制兼容性问题
2. **回调机制**：采用 `std::function` + `std::bind` 替代传统虚函数
3. **内存安全**：通过代理模式管理对象生命周期，防止内存泄露
4. **进程隔离**：插件运行在文件管理器进程空间，需遵循系统安全策略

## 前置条件

### 开发环境

#### 操作系统
- UOS 20 或更高版本
- Deepin V23 或更高版本

#### 编译工具链
- GCC 7.5 或更高版本
- CMake 3.10 或更高版本

#### 开发库依赖
- libdfm-extension-dev（dfm-extension 开发包）

### 安装开发包

```bash
sudo apt install libdfm-extension-dev
```

开发包安装后包含以下内容：

| 路径 | 内容 |
|------|------|
| `/usr/include/dfm-extension/` | 公共头文件 |
| `/usr/lib/<arch>/libdfm-extension.so` | 共享库 |
| `/usr/lib/<arch>/pkgconfig/dfm-extension.pc` | pkg-config 配置 |
| `/usr/lib/<arch>/cmake/dfm-extension/` | CMake 配置 |

其中 `<arch>` 为系统架构标识，如 `x86_64-linux-gnu` 或 `aarch64-linux-gnu`。

### 编程要求

- **C++ 标准**：C++17 或更高
- **编译语言**：C++
- **链接方式**：动态共享库（.so）
- **调用约定**：C 风格导出函数

## 实现规范

### 插件生命周期

插件加载和卸载遵循以下流程：

1. 文件管理器启动时扫描插件目录
2. 加载检测到的共享库文件
3. 调用 `dfm_extension_initialize()` 初始化插件
4. 调用类型获取函数获取插件实例
5. 运行时通过回调函数与插件交互
6. 文件管理器退出时调用 `dfm_extension_shutdown()` 清理资源

### 元接口定义

所有插件必须实现以下导出函数（`extern "C"` 约定）：

```cpp
extern "C" void dfm_extension_initialize();
extern "C" void dfm_extension_shutdown();
extern "C" DFMExtMenuPlugin *dfm_extension_menu();
extern "C" DFMExtEmblemIconPlugin *dfm_extension_emblem();
extern "C" DFMExtWindowPlugin *dfm_extension_window();
extern "C" DFMExtFilePlugin *dfm_extension_file();
```

#### 接口说明

| 函数 | 调用时机 | 职责 |
|------|---------|------|
| `dfm_extension_initialize()` | 插件加载后首先调用 | 创建插件实例，进行初始化操作 |
| `dfm_extension_shutdown()` | 文件管理器退出前调用 | 释放插件资源，销毁实例 |
| `dfm_extension_menu()` | 初始化后调用 | 返回菜单插件实例指针，无需实现返回 nullptr |
| `dfm_extension_emblem()` | 初始化后调用 | 返回角标插件实例指针，无需实现返回 nullptr |
| `dfm_extension_window()` | 初始化后调用 | 返回窗口插件实例指针，无需实现返回 nullptr |
| `dfm_extension_file()` | 初始化后调用 | 返回文件插件实例指针，无需实现返回 nullptr |

**重要约束**：
- 必须使用 `extern "C"` 避免符号修饰
- 函数名称必须完全匹配
- 插件实例需要在 `initialize()` 中创建并保持静态或全局生命周期
- `shutdown()` 中必须销毁实例并将指针置空


### Menu Plugin 接口

#### 核心类：DFMExtMenuPlugin

头文件：`<dfm-extension/menu/dfmextmenuplugin.h>`

```cpp
namespace DFMEXT {

class DFMExtMenuPlugin {
public:
    using InitializeFunc = std::function<void(DFMExtMenuProxy*)>;
    using BuildNormalMenuFunc = std::function<bool(DFMExtMenu*,
                                   const std::string&,
                                   const std::string&,
                                   const std::list<std::string>&,
                                   bool)>;
    using BuildEmptyAreaMenuFunc = std::function<bool(DFMExtMenu*,
                                       const std::string&,
                                       bool)>;

    DFMExtMenuPlugin();
    virtual ~DFMExtMenuPlugin();

    // 需注册的回调函数
    virtual void initialize(DFMExtMenuProxy *proxy);
    virtual bool buildNormalMenu(DFMExtMenu *main,
                                 const std::string &currentPath,
                                 const std::string &focusPath,
                                 const std::list<std::string> &pathList,
                                 bool onDesktop);
    virtual bool buildEmptyAreaMenu(DFMExtMenu *main,
                                    const std::string &currentPath,
                                    bool onDesktop);

    // 注册函数
    void registerInitialize(const InitializeFunc &func);
    void registerBuildNormalMenu(const BuildNormalMenuFunc &func);
    void registerBuildEmptyAreaMenu(const BuildEmptyAreaMenuFunc &func);
};

} // namespace DFMEXT
```

#### 接口说明

**initialize(DFMExtMenuProxy *proxy)**
- **调用时机**：插件初始化后，首次构建菜单前调用一次
- **参数说明**：
  - `proxy`：菜单代理对象，用于创建菜单和动作对象，需保存供后续使用
- **实现要求**：保存 proxy 指针，执行插件初始化逻辑

**buildNormalMenu(...)**
- **调用时机**：用户在文件上右键触发菜单时调用
- **参数说明**：
  - `main`：主菜单对象，插件通过此对象添加菜单项
  - `currentPath`：当前目录路径（V6 版本为文件路径，V5 为 URL）
  - `focusPath`：当前右键选中的文件路径
  - `pathList`：所有选中文件的路径列表
  - `onDesktop`：是否在桌面环境触发
- **返回值**：`true` 表示成功构建菜单，`false` 表示不处理
- **实现要求**：通过 proxy 创建菜单和动作，添加到 main，返回 true

**buildEmptyAreaMenu(...)**
- **调用时机**：用户在空白区域右键触发菜单时调用
- **参数说明**：
  - `main`：主菜单对象
  - `currentPath`：当前目录路径
  - `onDesktop`：是否在桌面环境触发
- **返回值**：`true` 表示成功构建菜单，`false` 表示不处理

#### DFMExtMenuProxy 接口

头文件：`<dfm-extension/menu/dfmextmenuproxy.h>`

```cpp
class DFMExtMenuProxy {
public:
    DFMExtMenu *createMenu();
    bool deleteMenu(DFMExtMenu *menu);
    DFMExtAction *createAction();
    bool deleteAction(DFMExtAction *action);
};
```

**方法说明**：
- `createMenu()`：在堆上创建菜单对象，返回指针
- `deleteMenu()`：删除菜单对象，释放内存（通常无需手动调用）
- `createAction()`：在堆上创建动作对象，返回指针
- `deleteAction()`：删除动作对象，释放内存（通常无需手动调用）

**内存管理规则**：通过 proxy 创建的菜单和动作，一旦被添加到 main 菜单中，其生命周期将由文件管理器自动管理，无需手动调用 delete。


#### DFMExtMenu 接口

头文件：`<dfm-extension/menu/dfmextmenu.h>`

```cpp
class DFMExtMenu {
public:
    using TriggeredFunc = std::function<void(DFMExtAction*)>;
    using HoveredFunc = std::function<void(DFMExtAction*)>;
    using DeletedFunc = std::function<void(DFMExtMenu*)>;

    // 属性方法
    std::string title() const;
    void setTitle(const std::string &title);
    std::string icon() const;
    void setIcon(const std::string &iconName);

    // 菜单项操作
    bool addAction(DFMExtAction *action);
    bool insertAction(DFMExtAction *before, DFMExtAction *action);
    DFMExtAction *menuAction() const;
    std::list<DFMExtAction*> actions() const;

    // 事件回调（需注册）
    virtual void triggered(DFMExtAction *action);
    virtual void hovered(DFMExtAction *action);
    virtual void deleted(DFMExtMenu *self);

    // 注册函数
    void registerTriggered(const TriggeredFunc &func);
    void registerHovered(const HoveredFunc &func);
    void registerDeleted(const DeletedFunc &func);
};
```

**关键接口说明**：
- `setTitle()`：设置菜单标题
- `setIcon()`：设置菜单图标，参数为主题图标名称或绝对路径
- `addAction()`：添加菜单项到末尾
- `insertAction()`：在指定菜单项前插入
- `deleted()`：菜单销毁前的回调，用于释放插件自定义资源

#### DFMExtAction 接口

头文件：`<dfm-extension/menu/dfmextaction.h>`

```cpp
class DFMExtAction {
public:
    using TriggeredFunc = std::function<void(DFMExtAction*, bool)>;
    using HoveredFunc = std::function<void(DFMExtAction*)>;
    using DeletedFunc = std::function<void(DFMExtAction*)>;

    // 属性设置
    void setText(const std::string &text);
    std::string text() const;
    void setIcon(const std::string &icon);
    std::string icon() const;
    void setToolTip(const std::string &tip);
    std::string toolTip() const;

    // 菜单关联
    void setMenu(DFMExtMenu *menu);
    DFMExtMenu *menu() const;

    // 状态控制
    void setSeparator(bool b);
    bool isSeparator() const;
    void setCheckable(bool b);
    bool isCheckable() const;
    void setChecked(bool b);
    bool isChecked() const;
    void setEnabled(bool b);
    bool isEnabled() const;

    // 事件回调（需注册）
    virtual void triggered(DFMExtAction *self, bool checked = false);
    virtual void hovered(DFMExtAction *self);
    virtual void deleted(DFMExtAction *self);

    // 注册函数
    void registerTriggered(const TriggeredFunc &func);
    void registerHovered(const HoveredFunc &func);
    void registerDeleted(const DeletedFunc &func);
};
```

**常用方法说明**：
- `setText()`：设置菜单项显示文本
- `setIcon()`：设置图标，支持主题图标名或绝对路径
- `setMenu()`：关联子菜单，实现多级菜单
- `setSeparator()`：设置为分隔线
- `setCheckable()`：设置为可选中的菜单项
- `setChecked()`：设置选中状态
- `triggered()`：菜单项点击时触发的回调


### EmblemIcon Plugin 接口

#### 核心类：DFMExtEmblemIconPlugin

头文件：`<dfm-extension/emblemicon/dfmextemblemiconplugin.h>`

```cpp
namespace DFMEXT {

class DFMExtEmblemIconPlugin {
public:
    using LocationEmblemIconsFunc =
        std::function<DFMExtEmblem(const std::string&, int)>;

    DFMExtEmblemIconPlugin();
    virtual ~DFMExtEmblemIconPlugin();

    // 需注册的回调函数
    virtual DFMExtEmblem locationEmblemIcons(
        const std::string &filePath,
        int systemIconCount) const;

    // 注册函数
    void registerLocationEmblemIcons(const LocationEmblemIconsFunc &func);

    // 已废弃的接口
    [[deprecated]] virtual IconsType emblemIcons(
        const std::string &filePath) const;
    void registerEmblemIcons(const EmblemIconsFunc &func);
};

} // namespace DFMEXT
```

#### 接口说明

**locationEmblemIcons(const std::string &filePath, int systemIconCount)**
- **调用时机**：文件管理器渲染文件图标时调用
- **参数说明**：
  - `filePath`：文件路径（V6 版本为本地路径，非 URL）
  - `systemIconCount`：当前文件已有的系统角标数量（0-4）
- **返回值**：`DFMExtEmblem` 对象，包含扩展角标的布局信息
- **约束条件**：
  - 单个文件最多显示 4 个角标（含系统角标）
  - 当 `systemIconCount >= 4` 时，扩展角标无法显示
  - 位置冲突时，系统角标优先于扩展角标

#### DFMExtEmblem 接口

头文件：`<dfm-extension/emblemicon/dfmextemblem.h>`

```cpp
class DFMExtEmblem {
public:
    DFMExtEmblem();
    ~DFMExtEmblem();
    DFMExtEmblem(const DFMExtEmblem &emblem);
    DFMExtEmblem &operator=(const DFMExtEmblem &emblem);

    void setEmblem(const std::vector<DFMExtEmblemIconLayout> &iconPaths);
    std::vector<DFMExtEmblemIconLayout> emblems() const;
};
```

#### DFMExtEmblemIconLayout 接口

头文件：`<dfm-extension/emblemicon/dfmextemblemiconlayout.h>`

```cpp
class DFMExtEmblemIconLayout {
public:
    enum class LocationType : uint8_t {
        BottomRight = 0,    // 右下角
        BottomLeft,         // 左下角
        TopLeft,            // 左上角
        TopRight,           // 右上角
        Custom = 0xff       // 自定义位置（预留）
    };

    DFMExtEmblemIconLayout(LocationType type,
                           const std::string &path,
                           int x = 0, int y = 0);
    ~DFMExtEmblemIconLayout();

    LocationType locationType() const;
    std::string iconPath() const;
    int x() const;
    int y() const;
};
```

**位置说明**：
- `BottomRight`：右下角，通常被只读角标占用
- `BottomLeft`：左下角，推荐用于扩展角标
- `TopLeft`：左上角，通常被系统角标占用
- `TopRight`：右上角，通常被共享角标占用

#### 角标资源管理

角标图标需遵循 Freedesktop 图标主题规范：

1. **安装位置**：`/usr/share/icons/<theme>/<size>/emblems/`
2. **命名规范**：`emblem-<name>.png`
3. **尺寸要求**：16x16、24x24、32x32、48x48、64x64
4. **注册方式**：创建索引文件或 symbolic link

示例安装命令：
```bash
sudo cp my-emblem.png /usr/share/icons/hicolor/16x16/emblems/emblem-my.png
sudo gtk-update-icon-cache /usr/share/icons/hicolor/
```

#### 角标持久化

插件可通过文件扩展属性（extended attributes）持久化角标状态：

```cpp
#include <sys/xattr.h>

// 设置角标属性
setxattr(filePath, "user.icon", iconName.c_str(), iconName.size(), 0);

// 读取角标属性
char buffer[256];
getxattr(filePath, "user.icon", buffer, sizeof(buffer));

// 删除角标属性
removexattr(filePath, "user.icon");
```

### Window Plugin 接口

#### 核心类：DFMExtWindowPlugin

头文件：`<dfm-extension/window/dfmextwindowplugin.h>`

```cpp
namespace DFMEXT {

class DFMExtWindowPlugin {
public:
    using WindowFunc = std::function<void(uint64_t)>;
    using WindowUrlFunc = std::function<void(uint64_t, const std::string&)>;

    DFMExtWindowPlugin();
    virtual ~DFMExtWindowPlugin();

    // 需注册的回调函数
    virtual void windowOpened(uint64_t winId);
    virtual void windowClosed(uint64_t winId);
    virtual void firstWindowOpened(uint64_t winId);
    virtual void lastWindowClosed(uint64_t winId);
    virtual void windowUrlChanged(uint64_t winId, const std::string &urlString);

    // 注册函数
    void registerWindowOpened(const WindowFunc &func);
    void registerWindowClosed(const WindowFunc &func);
    void registerFirstWindowOpened(const WindowFunc &func);
    void registerLastWindowClosed(const WindowFunc &func);
    void registerWindowUrlChanged(const WindowUrlFunc &func);
};

} // namespace DFMEXT
```

#### 接口说明

| 函数 | 调用时机 | 参数说明 |
|------|---------|---------|
| `windowOpened()` | 每个窗口打开时 | `winId`：窗口唯一标识符 |
| `windowClosed()` | 每个窗口关闭时 | `winId`：窗口唯一标识符 |
| `firstWindowOpened()` | 首个窗口打开时 | `winId`：窗口唯一标识符 |
| `lastWindowClosed()` | 最后一个窗口关闭时 | `winId`：窗口唯一标识符 |
| `windowUrlChanged()` | 窗口路径变化时 | `winId`：窗口标识符，`urlString`：新路径 |


### File Plugin 接口

#### 核心类：DFMExtFilePlugin

头文件：`<dfm-extension/file/dfmextfileplugin.h>`

```cpp
namespace DFMEXT {

class DFMExtFilePlugin {
public:
    using FileOperationHooksFunc =
        std::function<void(DFMExtFileOperationHook*)>;

    DFMExtFilePlugin();
    virtual ~DFMExtFilePlugin();

    // 需注册的回调函数
    virtual void fileOperationHooks(DFMExtFileOperationHook *hook);

    // 注册函数
    void registerFileOperationHooks(const FileOperationHooksFunc &func);
};

} // namespace DFMEXT
```

#### DFMExtFileOperationHook 接口

头文件：`<dfm-extension/file/dfmextfileoperationhook.h>`

```cpp
class DFMExtFileOperationHook {
public:
    using OpenFilesFunc = std::function<bool(
        const std::vector<std::string> &srcPaths,
        std::vector<std::string> *ignorePaths)>;

    void registerOpenFiles(const OpenFilesFunc &func);
};
```

#### 接口说明

**fileOperationHooks(DFMExtFileOperationHook *hook)**
- **调用时机**：插件初始化后调用一次
- **参数说明**：`hook` 对象用于注册文件操作的钩子函数
- **实现要求**：通过 hook 对象注册需要的钩子函数

**OpenFilesFunc 签名说明**
```cpp
bool openFiles(const std::vector<std::string> &srcPaths,
               std::vector<std::string> *ignorePaths)
```

**参数说明**：
- `srcPaths`：待打开的文件路径列表
- `ignorePaths`：输出参数，不处理的文件路径列表

**返回值含义**：
- `true`：插件处理了文件打开，阻止文件管理器默认行为
- `false`：插件不处理，使用文件管理器默认行为

### 回调注册机制

所有插件接口均采用回调注册机制，而非传统虚函数继承：

```cpp
class MyMenuPlugin : public DFMEXT::DFMExtMenuPlugin {
public:
    MyMenuPlugin() {
        // 使用 lambda 注册回调
        registerInitialize([this](DFMExtMenuProxy *proxy) {
            initialize(proxy);
        });

        registerBuildNormalMenu([this](
            DFMExtMenu *main,
            const std::string &currentPath,
            const std::string &focusPath,
            const std::list<std::string> &pathList,
            bool onDesktop) {
            return buildNormalMenu(main, currentPath, focusPath,
                                  pathList, onDesktop);
        });
    }

    // 实现实际的业务逻辑
    void initialize(DFMExtMenuProxy *proxy);
    bool buildNormalMenu(...);
};
```

**注册机制优势**：
- 避免 ABI 兼容性问题
- 支持动态绑定和 lambda 表达式
- 隐藏实现细节（pimpl 技术）

## 开发指南

### 项目结构

推荐的插件项目结构：

```
my-dfm-plugin/
├── CMakeLists.txt
├── src/
│   ├── my-plugin.cpp       # 元接口实现
│   ├── mymenuplugin.h
│   ├── mymenuplugin.cpp
│   ├── myemblemplugin.h
│   └── myemblemplugin.cpp
└── debian/
    ├── changelog
    ├── control
    └── rules
```

### CMakeLists.txt 配置

```cmake
cmake_minimum_required(VERSION 3.10)
project(my-dfm-plugin)

set(CMAKE_CXX_STANDARD 17)

# 查找依赖
find_package(dfm-extension REQUIRED)

# 收集源文件
file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS
    "src/*.h"
    "src/*.cpp"
)

# 构建共享库
add_library(${PROJECT_NAME} SHARED ${SRCS})

# 链接库
target_link_libraries(${PROJECT_NAME}
    PUBLIC ${dfm-extension_LIBRARIES}
)

# 包含目录
target_include_directories(${PROJECT_NAME}
    PRIVATE ${dfm-extension_INCLUDE_DIRS}
)

# 安装配置
include(GNUInstallDirs)
set(DFM_EXT_PLUGIN_DIR
    ${CMAKE_INSTALL_FULL_LIBDIR}/dde-file-manager/plugins/extensions
)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${DFM_EXT_PLUGIN_DIR}
)
```

### 构建流程

```bash
# 常规的 CMake 构建步骤如下：
$ cmake -B build -DCMAKE_INSTALL_PREFIX=/usr
$ cmake --build build

# 构建完成后，您可以通过以下命令安装：
$ sudo cmake --build build --target install

# 项目提供了 debian 文件夹，用于在 deepin Linux 桌面发行版下构建软件包。要构建软件包，请使用以下命令：
$ sudo apt build-dep . # 安装构建依赖
$ dpkg-buildpackage -uc -us -nc -b # 构建二进制包
```


### 内存管理

#### 创建对象

通过 `DFMExtMenuProxy` 创建所有菜单和动作对象：

```cpp
auto menu = m_proxy->createMenu();
auto action = m_proxy->createAction();
```

#### 自动管理

将对象添加到菜单后，文件管理器将自动管理其生命周期：

```cpp
main->addAction(action);   // action 的生命周期由文件管理器接管
menu->addAction(subAction); // subAction 的生命周期由 menu 接管
```

#### 手动管理

对于未添加到菜单的对象，需手动释放：

```cpp
auto tempAction = m_proxy->createAction();
// 使用 tempAction
m_proxy->deleteAction(tempAction);
```

#### 自定义资源

使用 `deleted` 回调管理插件自定义资源：

```cpp
auto customData = new MyCustomData;
main->registerDeleted([customData](DFMExtMenu*) {
    delete customData;
});
```

### 调试方法

#### 日志输出

使用标准输出流进行调试：

```cpp
#include <iostream>

std::cout << "Debug info" << std::endl;
std::cerr << "Error info" << std::endl;
```

查看日志：
```bash
# 启动文件管理器并查看输出
/usr/libexec/dde-file-manager 
```

#### 加载验证

验证插件是否被加载：

```cpp
extern "C" void dfm_extension_initialize() {
    std::cerr << "My plugin loaded!" << std::endl;
    // 初始化代码
}
```

#### 符号检查

检查导出符号是否正确：

```bash
# 查看导出符号
nm -D my-dfm-plugin.so | grep dfm_extension

# 预期输出应包含：
# dfm_extension_initialize
# dfm_extension_shutdown
# dfm_extension_menu
```

### 最佳实践

#### 1. 延迟创建子菜单

为提升性能，子菜单应在鼠标悬停时创建，而非构建主菜单时：

```cpp
rootAction->registerHovered([this](DFMExtAction *action) {
    if (!action->menu()->actions().empty())
        return; // 已创建，直接返回

    auto subAction1 = m_proxy->createAction();
    subAction1->setText("Sub Menu 1");
    action->menu()->addAction(subAction1);

    auto subAction2 = m_proxy->createAction();
    subAction2->setText("Sub Menu 2");
    action->menu()->addAction(subAction2);
});
```

#### 2. URL 与路径转换

V6 版本传入的是文件路径，如需兼容 URL 格式：

```cpp
std::string removeScheme(const std::string &url) {
    std::string result = url;
    size_t startPos = result.find("://");
    if (startPos != std::string::npos) {
        startPos = result.find('/', startPos + 3);
        if (startPos != std::string::npos)
            result = result.substr(startPos);
    }
    return result;
}
```

#### 3. 菜单项定位

在指定位置插入菜单项：

```cpp
// 查找"刷新"菜单项
auto actions = main->actions();
auto it = std::find_if(actions.cbegin(), actions.cend(),
    [](const DFMExtAction *action) {
        return action->text().find("刷新") == 0;
    });

if (it != actions.cend()) {
    // 在"刷新"前插入分隔线和自定义项
    auto separator = m_proxy->createAction();
    separator->setSeparator(true);
    main->insertAction(*it, separator);
    main->insertAction(*it, myAction);
}
```

- **注：**当前不满足国际化场景，后续提供action id

#### 4. 多选文件处理

遍历选中的文件列表：

```cpp
action->registerTriggered([pathList](DFMExtAction*, bool) {
    for (const auto &path : pathList) {
        // 处理每个文件
        processFile(path);
    }
});
```

## 部署说明

### 安装位置

插件应安装到以下目录之一：

| 优先级 | 路径 | 说明 |
|-------|------|------|
| 系统 | `/usr/lib/<arch>/dde-file-manager/plugins/extensions` | 全局可用，需要 root 权限 |
| 用户（待实现） |  | 仅当前用户可用 |

其中 `<arch>` 为系统架构，如 `x86_64-linux-gnu`。

### 安装方式

#### 方式一：直接安装

```bash
# 编译并安装
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build

sudo cmake --build build --target install
```

#### 方式二：打包安装

创建 Debian 包：

```bash
# 构建 Debian 包
dpkg-buildpackage -uc -us -nc -b

# 安装
sudo dpkg -i ../my-dfm-plugin_1.0.0_amd64.deb
```

#### 方式三：手动复制

```bash
# 复制插件到目标目录
sudo cp my-dfm-plugin.so \
    /usr/lib/<arch>/dde-file-manager/plugins/extensions/
```

### 激活机制

1. 文件管理器启动时自动扫描插件目录
2. 加载所有 `.so` 文件
3. 调用 `dfm_extension_initialize()` 验证插件
4. 调用类型获取函数，注册有效的插件
5. 无需额外配置或重启

### 卸载方式

```bash
# 删除插件文件
sudo rm /usr/lib/<arch>/dde-file-manager/plugins/extensions/my-dfm-plugin.so

# 如果是 Debian 包
sudo apt remove my-dfm-plugin
```

卸载后需重启文件管理器才能生效。


## 参考示例

### 完整示例项目

示例代码位于：
[dfm-extension-example](https://github.com/linuxdeepin/dde-file-manager/tree/master/examples/dfm-extension-example)

该示例包含所有四种机制的完整实现。

- **注：**示例以代码为准，文档仅供参考

### Menu Plugin 示例

```cpp
// mymenuplugin.h
class MyMenuPlugin : public DFMEXT::DFMExtMenuPlugin {
public:
    MyMenuPlugin();
    ~MyMenuPlugin();

    void initialize(DFMEXT::DFMExtMenuProxy *proxy) DFM_FAKE_OVERRIDE;
    bool buildNormalMenu(DFMExtMenu *main,
                         const std::string &currentPath,
                         const std::string &focusPath,
                         const std::list<std::string> &pathList,
                         bool onDesktop) DFM_FAKE_OVERRIDE;
    bool buildEmptyAreaMenu(DFMExtMenu *main,
                            const std::string &currentPath,
                            bool onDesktop) DFM_FAKE_OVERRIDE;

private:
    DFMEXT::DFMExtMenuProxy *m_proxy{nullptr};
};

// mymenuplugin.cpp
MyMenuPlugin::MyMenuPlugin() {
    registerInitialize([this](DFMExtMenuProxy *proxy) {
        initialize(proxy);
    });
    registerBuildNormalMenu([this](DFMExtMenu *main,
                                   const std::string &currentPath,
                                   const std::string &focusPath,
                                   const std::list<std::string> &pathList,
                                   bool onDesktop) {
        return buildNormalMenu(main, currentPath, focusPath,
                              pathList, onDesktop);
    });
    registerBuildEmptyAreaMenu([this](DFMExtMenu *main,
                                      const std::string &currentPath,
                                      bool onDesktop) {
        return buildEmptyAreaMenu(main, currentPath, onDesktop);
    });
}

void MyMenuPlugin::initialize(DFMExtMenuProxy *proxy) {
    m_proxy = proxy;
}

bool MyMenuPlugin::buildNormalMenu(DFMExtMenu *main,
                                   const std::string &currentPath,
                                   const std::string &focusPath,
                                   const std::list<std::string> &pathList,
                                   bool onDesktop) {
    (void)currentPath;
    (void)focusPath;
    (void)onDesktop;

    // 创建根菜单项
    auto rootAction = m_proxy->createAction();
    rootAction->setText("我的工具");

    // 创建子菜单
    auto menu = m_proxy->createMenu();
    rootAction->setMenu(menu);

    // 延迟创建子菜单项
    rootAction->registerHovered([this, pathList](DFMExtAction *action) {
        if (!action->menu()->actions().empty())
            return;

        auto item1 = m_proxy->createAction();
        item1->setText("处理文件");
        item1->setIcon("document-open");
        item1->registerTriggered([pathList](DFMExtAction*, bool) {
            for (const auto &path : pathList) {
                std::cout << "Processing: " << path << std::endl;
            }
        });
        action->menu()->addAction(item1);
    });

    main->addAction(rootAction);
    return true;
}

bool MyMenuPlugin::buildEmptyAreaMenu(DFMExtMenu *main,
                                      const std::string &currentPath,
                                      bool onDesktop) {
    auto action = m_proxy->createAction();
    action->setText(onDesktop ? "在文件管理器中打开" : "刷新当前目录");
    action->setIcon("view-refresh");

    action->registerTriggered([currentPath](DFMExtAction*, bool) {
        pid_t pid = fork();
        if (pid == 0) {
            char *argv[] = {
                (char*)"/usr/bin/dde-file-manager",
                (char*)"-n",
                const_cast<char*>(currentPath.c_str()),
                nullptr
            };
            execvp(argv[0], argv);
        }
    });

    main->addAction(action);
    return true;
}
```

### EmblemIcon Plugin 示例

```cpp
// myemblemiconplugin.h
class MyEmblemIconPlugin : public DFMEXT::DFMExtEmblemIconPlugin {
public:
    MyEmblemIconPlugin();
    ~MyEmblemIconPlugin();

    DFMExtEmblem locationEmblemIcons(const std::string &filePath,
                                     int systemIconCount) const
        DFM_FAKE_OVERRIDE;
};

// myemblemiconplugin.cpp
MyEmblemIconPlugin::MyEmblemIconPlugin() {
    registerLocationEmblemIcons([this](const std::string &filePath,
                                       int systemIconCount) {
        return locationEmblemIcons(filePath, systemIconCount);
    });
}

DFMExtEmblem MyEmblemIconPlugin::locationEmblemIcons(
    const std::string &filePath,
    int systemIconCount) const
{
    DFMExtEmblem emblem;

    // 系统角标已满，无法添加扩展角标
    if (systemIconCount >= 4)
        return emblem;

    // 从文件扩展属性读取角标信息
    char buffer[256] = {0};
    ssize_t result = getxattr(filePath.c_str(), "user.icon",
                              buffer, sizeof(buffer));
    if (result == -1)
        return emblem;

    // 创建角标布局
    std::string iconName(buffer);
    if (!iconName.empty()) {
        std::vector<DFMExtEmblemIconLayout> layouts;
        layouts.push_back(
            DFMExtEmblemIconLayout(
                DFMExtEmblemIconLayout::LocationType::BottomLeft,
                iconName
            )
        );
        emblem.setEmblem(layouts);
    }

    return emblem;
}
```

### 元接口实现示例

```cpp
// my-plugin.cpp
#include <dfm-extension/dfm-extension.h>
#include "mymenuplugin.h"
#include "myemblemiconplugin.h"

static DFMEXT::DFMExtMenuPlugin *g_menuPlugin = nullptr;
static DFMEXT::DFMExtEmblemIconPlugin *g_emblemPlugin = nullptr;

extern "C" void dfm_extension_initialize() {
    g_menuPlugin = new MyMenuPlugin();
    g_emblemPlugin = new MyEmblemIconPlugin();
}

extern "C" void dfm_extension_shutdown() {
    delete g_menuPlugin;
    delete g_emblemPlugin;
    g_menuPlugin = nullptr;
    g_emblemPlugin = nullptr;
}

extern "C" DFMEXT::DFMExtMenuPlugin *dfm_extension_menu() {
    return g_menuPlugin;
}

extern "C" DFMEXT::DFMExtEmblemIconPlugin *dfm_extension_emblem() {
    return g_emblemPlugin;
}

// 如果不需要 Window 或 File 插件，可以不实现对应的函数
```

## 注意事项

### 兼容性

#### 版本差异

| 特性 | V5 | V6 |
|------|----|----|
| 路径格式 | URL 字符串（file://） | 本地文件路径 |
| 角标接口 | `emblemIcons()` | `locationEmblemIcons()` |
| 命名空间 | `DFMEXT` | `DFMEXT` |

#### 适配建议

如需同时支持 V5 和 V6，可进行路径转换：

```cpp
std::string urlToPath(const std::string &url) {
    if (url.find("file://") == 0) {
        return url.substr(7); // 移除 file:// 前缀
    }
    return url;
}
```

- **注：**不兼容早期 V20 无需考虑

### 安全性

#### 权限控制

- 插件运行在文件管理器进程空间，拥有相同权限
- 避免在插件中执行危险操作（如 `system()`、`eval()`）
- 对用户输入进行验证和清理

#### 进程隔离

- 复杂业务建议启动外部程序，避免阻塞主进程

### 性能优化

#### 延迟加载

- 子菜单在 `hovered` 事件中创建，而非构建主菜单时
- 大量数据处理使用异步操作

#### 缓存策略

- 由于早期设计缺陷，提供角标的接口会在主线程高频调用，插件务必在此接口使用缓存，否则导致整个应用卡顿
- 避免重复、高频的系统调用

### 常见问题

#### 1. 插件未被加载

**症状**：插件功能未生效

**排查步骤**：
1. 检查插件是否安装在正确目录
2. 检查导出符号是否正确：`nm -D plugin.so | grep dfm_extension`
3. 查看文件管理器输出：`/usr/libexec/dde-file-manager`
4. 检查依赖库是否满足：`ldd plugin.so`

#### 2. 菜单不显示

**症状**：右键菜单中没有自定义项

**可能原因**：

- `buildNormalMenu()` 或 `buildEmptyAreaMenu()` 返回 `false`
- 菜单项未添加到 `main`：`main->addAction(action)`
- 创建对象后未正确注册回调

#### 3. 内存泄露

**症状**：长时间运行后内存占用持续增长

**解决方法**：
- 确保 `deleted` 回调中释放自定义资源
- 未添加到菜单的对象手动调用 `deleteAction()`
- 使用工具检测（如 valgrind）

#### 4. 角标不显示

**症状**：设置了角标但文件图标上未显示

**可能原因**：
- 系统角标数量已达到 4 个
- 位置冲突，被系统角标覆盖
- 角标图标未安装到主题目录
- 图标命名不符合规范

### 调试技巧

#### 符号验证

```bash
# 检查必需符号
nm -D my-plugin.so | grep dfm_extension

# 预期输出
# dfm_extension_initialize
# dfm_extension_shutdown
# dfm_extension_menu
```

#### 依赖检查

```bash
# 检查依赖库
ldd my-plugin.so

# 预期应包含
# libdfm-extension.so
# libstdc++.so.6
# libc.so.6
```

### 最佳实践总结

1. **内存管理**：优先使用文件管理器的自动管理，必要时手动释放
2. **延迟加载**：子菜单在 `hovered` 中创建，提升性能
3. **错误处理**：检查返回值，处理异常情况
4. **资源清理**：在 `deleted` 回调中释放自定义资源
5. **进程隔离**：耗时操作使用子进程，避免阻塞主线程
6. **版本兼容**：考虑 V5 和 V6 的差异（不兼容早期 V20 无需考虑），提供兼容层
7. **安全意识**：避免危险操作，验证用户输入
8. **日志输出**：使用标准输出流进行调试，便于问题定位

## 附录

### 相关文档

- [XDG Desktop Entry 规范](https://specifications.freedesktop.org/desktop-entry-spec/)
- [XDG Icon Theme 规范](https://specifications.freedesktop.org/icon-theme-spec/)
- [Linux Extended Attributes](https://man7.org/linux/man-pages/man7/xattr.7.html)

### 参考资源

- **项目仓库**：https://github.com/linuxdeepin/dde-file-manager
- **示例代码**：`examples/dfm-extension-example/`
- **接口定义**：`include/dfm-extension/`

### 版本历史

| 版本 | 日期 | 变更说明 |
|------|------|---------|
| 6.0.0 | 2024 | 移除 URL 前缀，新增 Window 和 File 插件接口 |
| 5.x | 2020-2023 | 初始版本，提供 Menu 和 Emblem 插件接口 |

