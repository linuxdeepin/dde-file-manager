# GVfs 挂载路径问题

> 使用硬编码的 GVfs 挂载路径导致应用在不同会话间失效。

---

## 问题描述 (P1)

应用中硬编码 GVfs 挂载路径 `/run/user/UID/gvfs/`，导致路径在不同会话或重启后失效。

---

## 根本原因

GVfs 挂载点：
1. 路径包含会话 ID，随每次登录而变化
2. `/run/user/UID/gvfs/smb-share:server=...` 会话敏感
3. 重启或重新登录后会重新生成

---

## 错误示例

### 硬编码 GVfs 路径

```cpp
// ❌ 错误: 硬编码 GVfs 挂载路径
QString getSambaPath(const QString &server, const QString &share) {
    // 重启后会失效！
    return QString("/run/user/%1/gvfs/smb-share:server=%2,share=%3")
        .arg(getuid())
        .arg(server)
        .arg(share);
}

// ❌ 错误: 保存到配置文件
settings.setValue("lastFile", "/run/user/1000/gvfs/smb...");
// 重启后路径无效
```

---

## 正确做法

### 使用 GIO URI API

```cpp
// ✅ 正确: 始终使用 GIO URI
#include <gio/gio.h>

void openSmbFile(const QString &server, const QString &share) {
    // 构建 URI
    QString uri = QString("smb://%1/%2").arg(server, share);

    // 使用 GFile
    GFile *file = g_file_new_for_uri(uri.toUtf8().constData());
    GFileInputStream *input = g_file_read(file, nullptr, &error);

    if (input) {
        // 读取数据
        g_object_unref(input);
    }
    g_object_unref(file);
}
```

### 使用 gio-qt

```cpp
// ✅ 正确: 使用 gio-qt 提供的 C++ 接口
#include <QUrl>
#include <QCoreApplication>
#include <giomm/file.h>

void openFile(const QUrl &uri) {
    auto file = Gio::File::create_for_uri(uri.toString().toStdString());

    // 异步读取
    file->read_async(sigc::mem_fun(*this, &MyClass::onFileRead),
                     Glib::PRIORITY_DEFAULT);
}
```

### 配置存储 URI 而非路径

```cpp
// ✅ 正确: 存储 URI
settings.setValue("lastFile", "smb://server/share/file.txt");

// ✅ 当需要时动态解析
QString getLastFilePath() {
    QString uri = settings.value("lastFile").toString();
    // 使用 GIO API 访问 UR，而非转换为路径
    return uri;  // 返回 URI 或使用 GIO API 处理
}
```

---

## 挂载点路径解析

### 获取当前挂载信息

```cpp
// 使用 gio mount 命令
QProcess process;
process.start("gio", QStringList() << "mount" << "-l");
process.waitForFinished();
QString mounts = process.readAllStandardOutput();

// 解析挂载点
```

### 使用 GMountMonitor 监听挂载变化

```cpp
#include <gio/gio.h>

void setupMountMonitor() {
    GVolumeMonitor *monitor = g_volume_monitor_get();

    g_signal_connect(monitor, "mount-added",
                     G_CALLBACK(onMountAdded), this);
    g_signal_connect(monitor, "mount-removed",
                     G_CALLBACK(onMountRemoved), this);
}
```

---

## 检测方法

### 测试路径有效性

```cpp
bool isPathValid(const QString &path) {
    GFile *file = g_file_new_for_path(path.toUtf8().constData());
    bool exists = g_file_query_exists(file, nullptr);
    g_object_unref(file);
    return exists;
}
```

### 验证 URI

```cpp
bool isUriValid(const QString &uri) {
    GFile *file = g_file_new_for_uri(uri.toUtf8().constData());
    bool exists = g_file_query_exists(file, nullptr);
    g_object_unref(file);
    return exists;
}
```

---

## 预防措施

### 1. 始终使用 URI

```cpp
// ✅ 错误: 使用路径
QString path = "/run/user/1000/gvfs/smb-share:...";

// ✅ 正确: 使用 URI
QUrl uri("smb://server/share/file.txt");
```

### 2. 配置文件存储 URI

```cpp
// 配置文件示例
{
    "recentFiles": [
        "smb://server/file1.txt",
        "file:///home/user/file2.txt",
        "mtp://device/file3.mp3"
    ]
}
```

### 3. 运行时路径解析

```cpp
// 只有在显示给用户时才解析路径
QString getDisplayName(const QString &uri) {
    GFile *file = g_file_new_for_uri(uri.toUtf8().constData());
    char *path = g_file_get_path(file);
    QString displayPath = QString::fromUtf8(path);
    g_free(path);
    g_object_unref(file);
    return displayPath;
}
```

### 4. 挂载状态检查

```cpp
// 检查远程位置是否已挂载
bool isMounted(const QString &uri) {
    GVolumeMonitor *monitor = g_volume_monitor_get();
    GMount *mount = g_volume_monitor_get_mount_for_uuid(
        monitor, uri.toUtf8().constData());

    if (mount) {
        g_object_unref(mount);
        return true;
    }
    return false;
}
```

---

## 代码检查清单

- [ ] 所有远程文件访问使用 GIO URI API
- [ ] 配置文件中存储 URI 而非路径
- [ ] 避免硬编码 `/run/user/UID/gvfs/` 路径
- [ ] 使用 gio-qt 或 C API 处理远程文件
- [ ] 添加挂载状态检查
- [ ] 提供用户挂载失败的提示
- [ ] 监听挂载/卸载事件
