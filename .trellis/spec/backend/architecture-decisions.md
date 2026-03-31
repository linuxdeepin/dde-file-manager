# DDE 架构决策

> 基于 DDE 系统集成的架构决策和核心规则。

---

## 概览

本指南定义 DDE 桌面应用开发中的关键架构决策和核心规则。

---

## 核心原则

1. **DTK 优先于 Qt 原生**
2. **GIO 优先于 QFile（网络协议）**
3. **DBus 事件驱动优先于轮询**
4. **Polkit 鉴权优先于 sudo**
5. **Qt6 优先，V20 回退到 Qt5**

---

## 决策表

| 场景 | 决策 | 参考文档 |
|------|------|----------|
| 显示主窗口 | `DMainWindow` 而非 `QMainWindow` | [DTK 组件指南](reference/dtk-widgets-guide.md) |
| 访问 `smb://` 路径 | GIO `GFile` 而非 `QFile` | [GVfs/GIO 集成](reference/gvfs-gio-integration.md) |
| 获取电池/网络状态 | DBus 信号监听而非轮询 | [DBus 服务使用](reference/dbus-service-usage.md) |
| 修改 `/etc` 配置 | Polkit + Helper 而非 sudo | [Polkit 认证流程](reference/polkit-auth-workflow.md) |
| 深色/浅色主题适配 | `applicationPalette()` 而非硬编码 | [DTK 组件指南](reference/dtk-widgets-guide.md) |
| 大文件 I/O | QtConcurrent 后台线程 | [GVfs/GIO 集成](reference/gvfs-gio-integration.md) |

---

## 核心规则

### 规则 1: DTK 组件（强制性）

| 推荐使用 | 禁止使用 |
|---------|---------|
| `DMainWindow` | `QMainWindow` |
| `DDialog` | `QDialog` |
| `DMessageBox` | `QMessageBox` |
| `DWidget` | `QWidget` |

```cpp
// ✅ 正确
#include <DMainWindow>
#include <DMessageBox>
DWIDGET_USE_NAMESPACE

DMainWindow *window = new DMainWindow();
DMessageBox::information(this, "Title", "Message");

// ❌ 禁止
QMainWindow *window = new QMainWindow();
```

**主题**: 使用 `DGuiApplicationHelper::instance()->applicationPalette()`，从不硬编码颜色。

📖 **详情**: [reference/dtk-widgets-guide.md](reference/dtk-widgets-guide.md)

### 规则 2: GVfs/GIO for 网络协议

```cpp
// ✅ 正确: smb://, mtp://, dav://
#include <gio/gio.h>
GFile *file = g_file_new_for_uri("smb://server/share/file.pdf");
GFileInputStream *input = g_file_read(file, nullptr, &error);

// ❌ 禁止
QFile file("smb://server/share/file.pdf"); // 失败
```

**异步 I/O**: 始终对文件操作使用 `QtConcurrent::run()`。

📖 **详情**: [reference/gvfs-gio-integration.md](reference/gvfs-gio-integration.md)

### 规则 3: DBus 事件驱动

```cpp
// ✅ 正确: 监听信号
QDBusConnection::systemBus().connect(
    "org.freedesktop.UPower",
    "/org/freedesktop/UPower",
    "org.freedesktop.DBus.Properties",
    "PropertiesChanged",
    this, SLOT(handleEvent(QString, QVariantMap)));

// ❌ 禁止: 轮询
QTimer *timer = new QTimer();
connect(timer, &QTimer::timeout, this, [](){
    QFile::read("/sys/class/power_supply/BAT0/capacity");
});
```

📖 **详情**: [reference/dbus-service-usage.md](reference/dbus-service-usage.md)

### 规则 4: Polkit 认证

```cpp
// ✅ 正确
PolkitQt1::Authority::instance()->checkAuthorizationSync(
    "org.deepin.dde.policy.authentication",
    PolkitQt1::UnixProcessSubject(QCoreApplication::applicationPid()),
    PolkitQt1::Authority::AllowUserInteraction);

// ❌ 禁止
sudo ./application  // 安全风险
```

📖 **详情**: [reference/polkit-auth-workflow.md](reference/polkit-auth-workflow.md)

---

## CMake 配置

参考 `cmake-conventions.md` 获取完整的 Qt6/Qt5 双版本支持。

```cmake
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
if(QT_VERSION_MAJOR EQUAL 6)
    set(DTK_VERSION_MAJOR 6)
else()
    set(DTK_VERSION_MAJOR "")
endif()

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets)
find_package(Dtk${DTK_VERSION_MAJOR}Widget REQUIRED)

target_link_libraries(app PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Dtk${DTK_VERSION_MAJOR}::Widget
)
```

---

## Debian Control 文件

参考 `git-conventions.md` 获取文件约定。

| 文件 | Deepin | Qt |
|------|--------|-----|
| `debian/control` | V25 | Qt6 |
| `debian/control.1` | V20 | Qt5 |

**依赖 (V25/Qt6)**:
```
Build-Depends:
 qt6-base-dev | qtbase5-dev,
 libdtk6widget-dev | libdtkwidget-dev,
 libpolkit-qt6-1-dev | libpolkit-qt5-1-dev
```

---

## 常见陷阱

| 陷阱 | 后果 | 修复 |
|------|------|------|
| GVfs 挂载点 `/run/user/UID/gvfs/` | 路径随会话变化 | → 使用 GIO URI API |
| DBus 总线类型错误 | 连接失败 | → System vs Session |
| Polkit helper 未验证输入 | 命令注入风险 | → 严格验证路径 |
| 非主线程更新 UI | 崩溃 | → 信号槽 (QueuedConnection) |
| 忽略 `GError` | 无法诊断问题 | → 检查 error 参数 |

---

## 代码审查清单

📖 **完整清单**: [reference/code-review-checklist.md](reference/code-review-checklist.md)

**快速检查**:
```bash
# 检查 DTK 组件使用
grep -r "QMainWindow\|QDialog\|QMessageBox" src/

# 检查硬编码颜色
grep -rE "#[0-9a-fA-F]{6}" src/

# 检查 GIO 使用
grep -r "g_file_new_for_uri" src/
```

---

## 参考文档

- [DTK 组件指南](reference/dtk-widgets-guide.md) - 控件使用、主题适配
- [GVfs/GIO 集成](reference/gvfs-gio-integration.md) - 文件操作、异步 I/O
- [DBus 服务使用](reference/dbus-service-usage.md) - 系统服务集成
- [Polkit 认证流程](reference/polkit-auth-workflow.md) - 权限控制、Helper 模式
- [代码审查清单](reference/code-review-checklist.md) - 审查清单
