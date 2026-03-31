# DBus 接口命名规范

> DDE 桌面环境 DBus 接口的命名和设计规范。

---

## 概览

本规范定义 DDE 组件间 DBus 通信的标准命名格式，确保接口的一致性和可维护性。

遵循 [DBus API Design Guidelines](https://dbus.freedesktop.org/doc/dbus-api-design.html) 并在此基础上定义 deepin/DDE 特有的约定。

---

## 命名结构

### 标准格式

DBus 接口由四个部分组成：**服务名**、**对象路径**、**接口**、**方法/属性/信号**。

```
服务名:    org.deepin.DDE1.Accounts
对象路径:  /org/deepin/dde/accounts      # 注意路径中的转换
接口:      org.deepin.DDE1.Accounts
方法:      SetBrightness
属性:      Volume
信号:      BrightnessChanged
```

### 命名规则分解

| 部分 | 格式 | 示例 |
|------|------|------|
| **域名** | 倒置域名 | `org.deepin`, `org.desktopspec` |
| **项目名** | 大小写混合 + 版本号 | `DDE1`, `Manual1` |
| **组件名** | 大小写混合 | `Accounts`, `Search` |

---

## 服务命名规范

### 新格式（推荐）

```cpp
// ✅ 正确 - 明确的版本号
org.deepin.DDE1.Accounts
org.deepin.Manual1.Search
org.desktopspec.ConfigManager

// ✅ 正确 - 多级组件
org.deepin.DDE1.Calendar.Event
org.deepin.DDE1.FileManager.Mount
```

### 旧格式（已废弃）

```cpp
// ❌ 废弃 - daemon 意义不明确
com.deepin.daemon.Accounts
com.deepin.daemon.Display

// ⚠️ 迁移路径
// 旧: com.deepin.daemon.Accounts
// 新: org.deepin.DDE1.Accounts
```

---

## 对象路径规范

### 命名转换

对象路径将服务名中的点号`.`替换为斜杠`/`，并将大写字母转换为小写（除路径中的单词边界）。

```cpp
// ✅ 正确 - 标准转换规则
// 服务名: org.deepin.DDE1.Accounts
// 路径:   /org/deepin/dde/accounts

// 服务名: org.deepin.DDE1.FileManager.Mount
// 路径:   /org/deepin/dde/file-manager/mount

// 服务名: org.deepin.Manual1.Search
// 路径:   /org/deepin/manual1/search
```

### 路径变体

对于有多个实例的对象，可以在路径末尾添加唯一标识。

```cpp
// ✅ 正确 - 单个实例
/org/deepin/dde/accounts

// ✅ 正确 - 多个实例
/org/deepin/dde/accounts/user1
/org/deepin/dde/accounts/user2

// ✅ 正确 - 使用 ID
/org/deepin/dde/mount/sd-1234
```

---

## 接口命名规范

### 基本规则

接口名与服务名使用相同的命名格式（大小写混合 + 版本号）。

```cpp
// ✅ 正确
org.deepin.DDE1.Accounts
org.desktopspec.ConfigManager

// ❌ 错误 - 缺少版本号
org.deepin.dde.Accounts       // 应该是 DDE1
org.freedesktop.portal        // 缺少版本号
```

### 接口分段

一个服务可以提供多个接口，接口应该按照功能进行分段。

```cpp
// ✅ 正确 - 多接口服务
org.deepin.DDE1.Calendar.Event        // 事件相关
org.deepin.DDE1.Calendar.Schedule     // 日程相关
org.deepin.DDE1.Calendar.Widget       // 窗口部件
```

---

## 方法命名规范

### 基本规则

方法名使用**大小写混合**格式，每个单词首字母大写（PascalCase）。

```cpp
// ✅ 正确
SetBrightness(int brightness)
GetUserInfo(const QString &username)
CreateFolder(const QString &path)

// ❌ 错误
set_brightness         // 下划线命名
get_user_info         // 下划线命名
createfolder          // 不区分大小写
```

### 命名模式

| 模式 | 示例 | 说明 |
|------|------|------|
| Get | `GetVolume()` | 获取值 |
| Set | `SetVolume(int)` | 设置值 |
| Is | `IsValid()` | 检查状态（返回 bool） |
| Has | `HasPermission()` | 检查拥有（返回 bool） |
| Create | `CreateFolder()` | 创建对象 |
| Delete | `DeleteFolder()` | 删除对象 |
| Add | `AddItem()` | 添加元素 |
| Remove | `RemoveItem()` | 移除元素 |

### 异步方法

异步方法应使用明确的命名约定。

```cpp
// ✅ 正确 - 异步操作
void CreateFolderAsync(const QString &path)
void RequestPermissionsAsync()

// ✅ 正确 - 信号方式
QDBusPendingCall reply = interface.asyncCall("CreateFolder", path);
```

---

## 属性命名规范

### 基本规则

属性名使用**大小写混合**格式，与方法命名风格一致。

```cpp
// ✅ 正确
Volume            // 音量
Brightness        // 亮度
UserName          // 用户名
FilePath          // 文件路径

// ❌ 错误
volume            // 全小写
file_path         // 下划线
```

### 只读属性

只读属性应明确标记。

```cpp
// ✅ 正确 - DBus XML 中定义
<property name="UserName" type="s" access="read" />

// ✅ 正确 - QDBusAbstractAdaptor 中
Q_PROPERTY(QString UserName READ userName)
```

### 读写属性

读写属性使用标准的 Get/Set 方法。

```cpp
// ✅ 正确
<property name="Volume" type="i" access="readwrite" />

// 对应的方法
int Volume() const;
void SetVolume(int volume);
```

---

## 信号命名规范

### 基本规则

信号名使用**大小写混合**格式，通常表示状态变化。

```cpp
// ✅ 正确
VolumeChanged(int newVolume)
BrightnessChanged(int newBrightness)
StatusChanged(int newStatus)

// ❌ 错误
volume_changed         // 下划线
OnVolumeChanged       // 不要加 On 前缀
```

### 命名模式

| 模式 | 示例 | 说明 |
|------|------|------|
| Changed | `VolumeChanged()` | 值变化 |
| Added | `ItemAdded(QString item)` | 添加元素 |
| Removed | `ItemRemoved(QString item)` | 移除元素 |
| Started | `ProcessStarted()` | 开始 |
| Finished | `ProcessFinished()` | 完成 |
| Failed | `ProcessFailed(QString error)` | 失败 |

### 信号参数

信号应包含变化的相关信息。

```cpp
// ✅ 正确 - 提供变化后的值
void VolumeChanged(int volume)
void ItemAdded(const QString &item, int index)

// ✅ 正确 - 提供完整状态
void StatusChanged(int oldStatus, int newStatus)

// ⚠️ 备选 - 简化版本（适用于频繁触发的信号）
void DataChanged()
```

---

## 完整示例

### 服务端代码

```cpp
// VolumeService.h
class VolumeService : public QObject, public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.DDE1.Volume")
    Q_PROPERTY(int Volume READ volume WRITE setVolume NOTIFY volumeChanged)

public:
    explicit VolumeService(QObject *parent = nullptr);

    // ✅ 正确 - 方法命名
    int volume() const;
    void setVolume(int volume);
    bool isMuted() const;
    void setMuted(bool muted);

signals:
    // ✅ 正确 - 信号命名
    void VolumeChanged(int volume);
    void MutedChanged(bool muted);
};
```

### DBus XML 描述

```xml
<!DOCTYPE node PUBLIC "-//freedesktop//DTD D-BUS Object Introspection 1.0//EN"
"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd">

<node>
  <!-- ✅ 正确 - 接口定义 -->
  <interface name="org.deepin.DDE1.Volume">

    <!-- ✅ 正确 - 属性定义 -->
    <property name="Volume" type="i" access="readwrite" />
    <property name="Muted" type="b" access="readwrite" />

    <!-- ✅ 正确 - 方法定义 -->
    <method name="SetVolume">
      <arg name="volume" type="i" direction="in" />
    </method>

    <method name="GetVolume">
      <arg name="volume" type="i" direction="out" />
    </method>

    <method name="IsMuted">
      <arg name="muted" type="b" direction="out" />
    </method>

    <!-- ✅ 正确 - 信号定义 -->
    <signal name="VolumeChanged">
      <arg name="volume" type="i" />
    </signal>

    <signal name="MutedChanged">
      <arg name="muted" type="b" />
    </signal>

  </interface>
</node>
```

### 客户端代码

```cpp
// 客户端使用
QDBusInterface iface(
    "org.deepin.DDE1.Volume",                    // ✅ 服务名
    "/org/deepin/dde/volume",                    // ✅ 对象路径
    "org.deepin.DDE1.Volume",                    // ✅ 接口
    QDBusConnection::sessionBus()
);

// ✅ 正确 - 方法调用
iface.call("SetVolume", 50);
QDBusReply<int> reply = iface.call("GetVolume");

// ✅ 正确 - 连接信号
QDBusConnection::sessionBus().connect(
    "org.deepin.DDE1.Volume",
    "/org/deepin/dde/volume",
    "org.deepin.DDE1.Volume",
    "VolumeChanged",
    this,
    SLOT(onVolumeChanged(int))
);
```

---

## 迁移指南

### 从旧格式迁移

```cpp
// ❌ 旧格式（废弃）
服务名:    com.deepin.daemon.Accounts
路径:      /com/deepin/daemon/Accounts
接口:      com.deepin.daemon.Accounts
方法:      SetUserName

// ✅ 新格式（推荐）
服务名:    org.deepin.DDE1.Accounts
路径:      /org/deepin/dde/accounts
接口:      org.deepin.DDE1.Accounts
方法:      SetUserName
```

### 迁移步骤

1. **评估影响范围**：确认哪些服务仍在使用旧格式
2. **更新服务实现**：修改服务注册时的名称
3. **更新客户端代码**：修改接口调用
4. **提供兼容层**：在过渡期同时支持新旧格式
5. **通知所有使用者**：发布变更日志

---

## 快速参考

| 元素 | 格式规则 | 正确示例 |
|------|---------|----------|
| 服务名 | `org.domain.Project1.Version` | `org.deepin.DDE1.Accounts` |
| 对象路径 | `/org/domain/project/version/component` | `/org/deepin/dde/accounts` |
| 接口 | 与服务名相同 | `org.deepin.DDE1.Accounts` |
| 方法 | PascalCase | `SetUserName` |
| 属性 | PascalCase | `UserName` |
| 信号 | PascalCase + Changed/Added/Removed | `UserNameChanged` |

---

## 反模式

```cpp
// ❌ 反模式 - 不要这样做

// 1. 混用命名风格
void set_volume(int);        // 不要用下划线
void SetuserName(int);       // 不要混合大小写

// 2. 描述不清的名称
void DoSomething(int x);     // 应该用具体名称
void Method1();              // 应该有明确含义

// 3. 缺少版本号
org.deepin.dde.accounts      // 应该是 DDE1

// 4. 不一致的命名
void SetUserName();          // PascalCase
void get_password();         // 混合了驼峰和下划线

// 5. 不必要的前缀
void OnVolumeChanged();      // 不需要 On 前缀
```

---

## 参考资源

- [DBus API Design Guidelines](https://dbus.freedesktop.org/doc/dbus-api-design.html)
- [dbus-specification](https://dbus.freedesktop.org/doc/dbus-specification.html)
- [Qt D-Bus Documentation](https://doc.qt.io/qt-6/qdbus.html)
