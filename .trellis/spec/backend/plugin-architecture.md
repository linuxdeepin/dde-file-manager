# 插件架构

> DDE 应用的插件化架构规范。

---

## 概览

本指南定义 DDE 应用的插件化架构，参考 dde-file-manager 插件系统。

---

## 插件系统架构

### 分层架构

```
┌─────────────────────────────────────┐
│         Application Core            │
├─────────────────────────────────────┤
│      Plugin Framework (dpf)        │
├─────────────────────────────────────┤
│      Plugin Interfaces             │
├─────────────────────────────────────┤
│         Plugins                    │
│  ├── Common                        │
│  ├── Specific (filemanager...)    │
│  └── Custom                        │
└─────────────────────────────────────┘
```

### 核心组件

- **Plugin Framework (dpf)** - 插件加载和生命周期管理
- **Plugin Interfaces** - 定义插件契约
- **Plugins** - 实现具体功能

---

## 插件元数据

### JSON 元数据格式

```json
{
  "Name": "plugin-name",
  "Version": "1.0.0",
  "Depends": [
    {
      "Name": "core",
      "Version": "1.0.0"
    }
  ],
  "Description": "Plugin description",
  "Author": "author",
  "Enabled": true
}
```

### 必须字段

| 字段 | 类型 | 必需 | 描述 |
|------|------|------|------|
| `Name` | string | 是 | 插件唯一标识 |
| `Version` | string | 是 | 语义化版本 |
| `Depends` | array | 否 | 依赖的其他插件 |

---

## 插件生命周期

### 状态机

```
Invalid → Reading → Readed → Loading → Loaded → Initialized → Started
   ↓          ↓         ↓                           ↓
  ←─────────←────────←────────←────────────────────────
           Stopped → Shutdown
```

### 状态说明

| 状态 | 描述 |
|------|------|
| `Invalid` | 插件元数据无效 |
| `Reading` | 正在读取元数据 |
| `Readed` | 元数据已读取 |
| `Loading` | 正在加载插件 |
| `Loaded` | 插件已加载（库） |
| `Initialized` | 插件已初始化 |
| `Started` | 插件正在运行 |
| `Stopped` | 插件已停止 |
| `Shutdown` | 插件已关闭 |

---

## 插件接口定义

### 基础插件接口

```cpp
#include <QObject>
#include <dpf/plugin.hpp>

class MyPlugin : public QObject, dpf::PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.MyPlugin")
    Q_INTERFACES(dpf::PluginInterface)

public:
    explicit MyPlugin(QObject *parent = nullptr);
    ~MyPlugin() override;

    // 插件接口
    bool initialize() override;
    void start() override;
    void stop() override;
};
```

### 扩展点接口

```cpp
// 定义扩展点接口
class IFileOperation : public QObject
{
    Q_OBJECT
public:
    virtual ~IFileOperation() = default;

    virtual bool canHandle(const QString &path) = 0;
    virtual bool copy(const QString &source, const QString &dest) = 0;
    virtual bool move(const QString &source, const QString &dest) = 0;
    virtual bool remove(const QString &path) = 0;
};

// 实现扩展点
class LocalFilePlugin : public IFileOperation
{
    Q_OBJECT
public:
    bool canHandle(const QString &path) override;
    bool copy(const QString &source, const QString &dest) override;
    bool move(const QString &source, const QString &dest) override;
    bool remove(const QString &path) override;
};
```

---

## 插件类型

### Common 插件

跨应用通用插件：
- 书签
- 菜单
- 标签
- 图标

### Application 特定插件

特定应用的专用插件：
- filemanager - 文件管理器
- filedialog - 文件对话框
- desktop - 桌面
- daemon - 守护进程

---

## CMake 配置

### 插件 CMakeLists.txt

```cmake
# 插件库
add_library(myplugin SHARED
    src/myplugin.cpp
    src/myplugin.h
)

target_link_libraries(myplugin PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    dtk6::core
    dfm6-framework
)

# 插件元数据
set(PLUGIN_METADATA_FILE "${CMAKE_CURRENT_SOURCE_DIR}/plugin.json")
set_target_properties(myplugin PROPERTIES
    PLUGIN_METADATA "${PLUGIN_METADATA_FILE}"
)

# 安装到插件目录
install(TARGETS myplugin
    LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}/myplugin
)

# 安装元数据
install(FILES ${PLUGIN_METADATA_FILE}
    DESTINATION ${PLUGIN_INSTALL_DIR}/myplugin
)
```

---

## 命名空间约定

### 插件命名空间

```cpp
// 遵循下划线分隔的命名空间
namespace filemanager {
namespace details {
    // 内部实现
}
namespace plugin {
    // 插件接口
}
}

// 或使用完整命名空间
namespace MyApplication {
namespace Plugins {
namespace MyPlugin {
    // 插件实现
}
}
}
```

### 避免命名冲突

- 使用应用前缀
- 使用功能后缀
- 避免通用名称（如 `Plugin`、`Manager`）

---

## 事件系统

### 插件间通信

```cpp
// 发布事件
dpf::EventDispatcherManager::instance()
    .publish(Events::EventType::FileSelected, fileUrl);

// 订阅事件
dpf::EventDispatcherManager::instance()
    .subscribe(Events::EventType::FileSelected,
              this, &MyPlugin::handleFileSelected);
```

### 定义事件

```cpp
// events/eventtypes.h
namespace Events {
Q_NAMESPACE enum class EventType {
    FileSelected = 1,
    FileCreated,
    FileDeleted,
    FileModified
};
Q_ENUM_NS(EventType)
}
```

---

## 最佳实践

1. **最小依赖** - 插件应该最小化依赖
2. **清晰契约** - 通过接口明确定义契约
3. **版本兼容** - 元数据中声明版本依赖
4. **错误处理** - 妥善处理插件加载失败
5. **资源清理** - 在 stop() 中清理资源
6. **线程安全** - 确保插件跨线程安全

---

## 快速参考

| 任务 | 方法 |
|------|------|
| 定义插件 | 继承 `PluginInterface` + `Q_PLUGIN_METADATA` |
| 插件元数据 | `plugin.json` |
| 插件状态 | Invalid → Reading → Readed → Loading → Loaded |
| 事件发布 | `dpf::EventDispatcherManager::instance().publish()` |
| 事件订阅 | `dpf::EventDispatcherManager::instance().subscribe()` |
| 命名约定 | `{app}::{features}::{Plugin}` |
