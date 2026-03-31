# GVfs/GIO 文件操作指南

## 核心原则

**访问网络协议（smb://, mtp://, dav://）必须使用 GIO API**，Qt 的 `QFile` 无法处理 GVfs 挂载的路径。

## 何时使用 GIO

| 场景 | 使用 | 原因 |
|------|------|------|
| `smb://` 网络共享 | ✅ GIO | Qt 不支持 SMB 协议 |
| `mtp://` 移动设备 | ✅ GIO | Qt 不支持 MTP 协议 |
| `dav://` WebDAV | ✅ GIO | Qt 不支持 WebDAV 协议 |
| `ftp://` FTP | ✅ GIO | Qt 不支持 GVfs FTP 挂载 |
| `file:///` 本地文件 | ⚠️ QFile 或 GIO | 两者都可以，QFile 更简单 |

## GVfs 架构

```
用户空间
├── GVfs 服务 (gvfsd)
│   ├── gvfsd-smb    → SMB/CIFS
│   ├── gvfsd-mtp    → MTP 设备
│   ├── gvfsd-dav    → WebDAV
│   └── gvfsd-ftp    → FTP
│
├── 挂载点
│   └── /run/user/1000/gvfs/
│       ├── smb-share:server=SERVER,share=SHARE/
│       ├── mtp=host_[device_id]/
│       └── dav:host=server.com,ssl=true/
│
└── 应用层
    ├── GIO API (推荐) → 直接 URI 访问
    └── QFile (仅本地) → 无法访问网络协议
```

## 基本用法

### 读取文件

```cpp
#include <gio/gio.h>
#include <QByteArray>

QByteArray readFile(const QString &uri) {
    GFile *file = g_file_new_for_uri(uri.toUtf8().constData());
    GError *error = nullptr;
    
    // 打开文件
    GFileInputStream *input = g_file_read(file, nullptr, &error);
    if (error) {
        qWarning() << "Failed to open:" << error->message;
        g_error_free(error);
        g_object_unref(file);
        return QByteArray();
    }
    
    // 读取内容
    QByteArray buffer;
    char buf[4096];
    gssize bytes_read;
    
    while ((bytes_read = g_input_stream_read(
        G_INPUT_STREAM(input), buf, sizeof(buf), nullptr, &error)) > 0) {
        buffer.append(buf, bytes_read);
    }
    
    if (error) {
        qWarning() << "Read error:" << error->message;
        g_error_free(error);
    }
    
    // 清理
    g_object_unref(input);
    g_object_unref(file);
    
    return buffer;
}

// 使用示例
QByteArray content = readFile("smb://server/share/file.txt");
```

### 写入文件

```cpp
bool writeFile(const QString &uri, const QByteArray &data) {
    GFile *file = g_file_new_for_uri(uri.toUtf8().constData());
    GError *error = nullptr;
    
    // 创建输出流
    GFileOutputStream *output = g_file_replace(
        file, nullptr, false, G_FILE_CREATE_NONE, nullptr, &error
    );
    
    if (error) {
        qWarning() << "Failed to create output:" << error->message;
        g_error_free(error);
        g_object_unref(file);
        return false;
    }
    
    // 写入数据
    gsize bytes_written;
    gboolean success = g_output_stream_write_all(
        G_OUTPUT_STREAM(output),
        data.constData(),
        data.size(),
        &bytes_written,
        nullptr,
        &error
    );
    
    if (!success) {
        qWarning() << "Write failed:" << error->message;
        g_error_free(error);
    }
    
    // 关闭流
    g_output_stream_close(G_OUTPUT_STREAM(output), nullptr, nullptr);
    g_object_unref(output);
    g_object_unref(file);
    
    return success;
}
```

### 获取文件信息

```cpp
struct FileInfo {
    QString displayName;
    qint64 size;
    QString mimeType;
    QDateTime modified;
};

FileInfo getFileInfo(const QString &uri) {
    GFile *file = g_file_new_for_uri(uri.toUtf8().constData());
    GError *error = nullptr;
    
    FileInfo info;
    
    GFileInfo *fileInfo = g_file_query_info(
        file,
        G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME ","
        G_FILE_ATTRIBUTE_STANDARD_SIZE ","
        G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE ","
        G_FILE_ATTRIBUTE_TIME_MODIFIED,
        G_FILE_QUERY_INFO_NONE,
        nullptr,
        &error
    );
    
    if (error) {
        qWarning() << "Query failed:" << error->message;
        g_error_free(error);
        g_object_unref(file);
        return info;
    }
    
    info.displayName = g_file_info_get_display_name(fileInfo);
    info.size = g_file_info_get_size(fileInfo);
    
    const char *contentType = g_file_info_get_content_type(fileInfo);
    if (contentType) {
        info.mimeType = contentType;
    }
    
    GTimeVal mtime;
    if (g_file_info_get_modification_time(fileInfo, &mtime)) {
        info.modified = QDateTime::fromSecsSinceEpoch(mtime.tv_sec);
    }
    
    g_object_unref(fileInfo);
    g_object_unref(file);
    
    return info;
}
```

## 异步操作

**文件 I/O 必须在后台线程执行**，避免阻塞 UI：

```cpp
#include <QFuture>
#include <QtConcurrent>

class GVfsFileHandler : public QObject {
    Q_OBJECT
    
public slots:
    void readFileAsync(const QString &uri) {
        QFutureWatcher<QByteArray> *watcher = 
            new QFutureWatcher<QByteArray>(this);
        
        connect(watcher, &QFutureWatcher<QByteArray>::finished,
                this, [this, watcher, uri]() {
            try {
                QByteArray data = watcher->result();
                emit fileReadComplete(uri, data);
            } catch (...) {
                emit fileReadError(uri, "Unknown error");
            }
            watcher->deleteLater();
        });
        
        QFuture<QByteArray> future = QtConcurrent::run([uri]() {
            return readFile(uri);  // 同步 GIO 操作在后台线程
        });
        
        watcher->setFuture(future);
    }
    
signals:
    void fileReadComplete(const QString &uri, const QByteArray &data);
    void fileReadError(const QString &uri, const QString &error);
};
```

## 错误处理

```cpp
GError *error = nullptr;
GFile *file = g_file_new_for_uri(uri);

GFileInputStream *input = g_file_read(file, nullptr, &error);
if (error) {
    switch (error->code) {
    case G_IO_ERROR_NOT_FOUND:
        qWarning() << "File not found:" << uri;
        break;
    case G_IO_ERROR_PERMISSION_DENIED:
        qWarning() << "Permission denied:" << uri;
        break;
    case G_IO_ERROR_HOST_NOT_FOUND:
        qWarning() << "Host not found:" << uri;
        break;
    case G_IO_ERROR_NOT_MOUNTED:
        qWarning() << "Volume not mounted:" << uri;
        break;
    default:
        qWarning() << "GIO error:" << error->message;
    }
    g_error_free(error);
}
```

## 常见错误

| 错误 | 后果 | 修复 |
|------|------|------|
| `QFile("smb://...")` | 打开失败 | → `g_file_new_for_uri()` |
| 主线程 GIO 操作 | UI 冻结 | → `QtConcurrent::run()` |
| 忽略 `GError` | 无法诊断问题 | → 检查 error 参数 |
| 忘记 `g_object_unref` | 内存泄漏 | → 使用 RAII 包装 |
| 硬编码 GVfs 挂载点 | 路径可能变化 | → 使用 URI API |

## GVfs 挂载点参考

**不要硬编码这些路径**，优先使用 GIO URI API：

```bash
# GVfs 挂载点位置
/run/user/[UID]/gvfs/

# SMB 挂载点格式
smb-share:server=SERVER,share=SHARE_NAME/

# MTP 挂载点格式
mtp=host_[device_id]/storage_[id]/

# WebDAV 挂载点格式
dav:host=server.com,ssl=true,port=443,user=username/

# FTP 挂载点格式
ftp:host=server.com,user=username/
```

## CMake 配置

```cmake
find_package(PkgConfig REQUIRED)
pkg_check_modules(GIO REQUIRED gio-2.0)

target_include_directories(app PRIVATE ${GIO_INCLUDE_DIRS})
target_link_libraries(app PRIVATE ${GIO_LIBRARIES})
```

## 相关资源

- [GIO API 文档](https://docs.gtk.org/gio/)
- [GVfs 文档](https://wiki.gnome.org/Projects/GVfs)
