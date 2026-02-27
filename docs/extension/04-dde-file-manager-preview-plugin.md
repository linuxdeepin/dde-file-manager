# DDE 文件管理器预览插件规范

## 概述

预览插件是 DDE 文件管理器提供的一套基于 Qt 插件机制的扩展接口，允许开发者为特定文件类型实现**空格键快速预览**功能。用户在文件管理器中选中文件后按下空格键，文件管理器将启动独立的预览进程，并根据文件的 MIME 类型加载对应的预览插件进行展示。通过实现标准化的接口，插件可以自定义任意文件类型的预览 UI、媒体播放控制以及状态栏信息。

### 适用场景

| 场景 | 示例 |
|------|------|
| 自定义文件类型预览 | 公司内部格式、CAD 文件、3D 模型 |
| 增强已有类型的预览体验 | 为 PDF 增加批注展示、为图片增加 EXIF 信息 |
| 媒体文件播放控制 | 自定义音视频格式的播放器 |
| 文档摘要提取 | 在预览中直接展示文档大纲、元数据 |

### 架构概览

预览插件模块由以下几个部分组成：

| 组件 | 路径 | 说明 |
|------|------|------|
| 预览进程主程序 | `dde-file-manager-preview` | 独立进程，负责加载插件并展示预览对话框 |
| 插件加载框架 | `libdfm-preview` | 提供插件扫描、加载和管理能力 |
| 内置预览插件 | `pluginpreviews/` | 框架内置的各类型文件预览插件 |
| 插件基础接口 | `dfm-base/interfaces/` | 定义插件工厂和预览视图的抽象基类 |

### 内置预览插件

框架已内置以下预览插件，可作为开发参考：

| 插件 | MIME 类型 | 说明 |
|------|---------|------|
| `dde-image-preview-plugin` | `image/*` | 图片文件预览 |
| `dde-music-preview-plugin` | `audio/*` | 音频文件预览，含播放控制 |
| `dde-text-preview-plugin` | `text/*` | 文本文件预览 |
| `dde-markdown-preview-plugin` | `text/markdown` | Markdown 文件预览 |
| `dde-pdf-preview-plugin` | `application/pdf` | PDF 文件预览 |
| `dde-video-preview-plugin` | `video/*` | 视频文件预览，含播放控制（部分架构） |
| `dde-dciicon-preview-plugin` | `image/dci` | DCI 图标文件预览 |

### 设计原则

1. **进程隔离**：预览插件运行在独立进程 `dde-file-manager-preview` 内，与文件管理器主进程隔离
2. **Qt 插件机制**：基于 `QPluginLoader` 和 `Q_PLUGIN_METADATA` 实现标准 Qt 插件
3. **MIME 类型路由**：通过 JSON 元数据文件声明支持的 MIME 类型，框架自动匹配和分发
4. **工厂模式**：插件工厂负责根据 MIME 类型创建具体预览实例

## 前置条件

### 开发环境

#### 操作系统
- UOS 20 或更高版本
- Deepin V23 或更高版本

#### 编译工具链
- GCC 7.5 或更高版本
- CMake 3.10 或更高版本

#### 开发库依赖
- `dde-file-manager-dev`（dfm-base 开发包，提供抽象基类头文件）
- Qt6 Core、Widgets 模块

### 安装开发包

```bash
sudo apt install dde-file-manager-dev
```

开发包包含以下关键头文件：

| 头文件 | 说明 |
|--------|------|
| `dfm-base/interfaces/abstractbasepreview.h` | 预览视图抽象基类 |
| `dfm-base/interfaces/abstractfilepreviewplugin.h` | 插件工厂抽象基类 |

### 编程要求

- **C++ 标准**：C++17 或更高
- **Qt 版本**：Qt6
- **链接方式**：动态共享库（.so）
- **插件接口 IID**：`com.deepin.filemanager.FilePreviewFactoryInterface_iid`

## 实现规范

### 插件生命周期

```
用户按空格键
     │
     ▼
文件管理器主进程
     │  通过 D-Bus 启动预览进程
     ▼
dde-file-manager-preview 进程
     │
     ├─ PreviewPluginLoader 扫描并加载插件目录
     │
     ├─ 对每个 .so 文件：读取 JSON 元数据 → 匹配 MIME 类型
     │
     ├─ 找到匹配插件 → 调用工厂 create(key) 创建预览实例
     │
     ├─ 调用 initialize(window, statusBar)
     │
     ├─ 调用 setFileUrl(url) 加载文件内容
     │
     ├─ 将 contentWidget() 嵌入 FilePreviewDialog 展示
     │
     └─ 用户关闭预览 → 框架调用 handleBeforDestroy() → 销毁实例
```

### 插件工厂接口：AbstractFilePreviewPlugin

头文件：`<dfm-base/interfaces/abstractfilepreviewplugin.h>`

```cpp
namespace dfmbase {

#define FilePreviewFactoryInterface_iid \
    "com.deepin.filemanager.FilePreviewFactoryInterface_iid"

class AbstractBasePreview;

class AbstractFilePreviewPlugin : public QObject
{
    Q_OBJECT
public:
    explicit AbstractFilePreviewPlugin(QObject *parent = nullptr);

    /**
     * @brief 根据 MIME 类型键创建预览实例
     * @param key 文件的 MIME 类型字符串，如 "image/jpeg"
     * @return 新创建的预览实例，所有权转交给调用方；不支持则返回 nullptr
     */
    virtual AbstractBasePreview *create(const QString &key) = 0;
};

} // namespace dfmbase
```

**实现要求**：

- 必须使用 `Q_PLUGIN_METADATA(IID FilePreviewFactoryInterface_iid FILE "xxx.json")` 宏声明元数据
- `create()` 应根据 `key`（MIME 类型）决定是否创建并返回预览实例
- 返回的指针所有权由框架接管，插件不应持有或再次使用该指针

### 预览视图接口：AbstractBasePreview

头文件：`<dfm-base/interfaces/abstractbasepreview.h>`

```cpp
namespace dfmbase {

class AbstractBasePreview : public QObject
{
    Q_OBJECT
public:
    explicit AbstractBasePreview(QObject *parent = nullptr);

    // ── 必须实现 ─────────────────────────────────────────────────
    virtual bool setFileUrl(const QUrl &url) = 0;
    virtual QUrl fileUrl() const = 0;
    virtual QWidget *contentWidget() const = 0;

    // ── 可选实现 ─────────────────────────────────────────────────
    virtual void initialize(QWidget *window, QWidget *statusBar);
    virtual QWidget *statusBarWidget() const;
    virtual Qt::Alignment statusBarWidgetAlignment() const;
    virtual QString title() const;
    virtual bool showStatusBarSeparator() const;
    virtual void play();   // 播放媒体（音视频插件实现）
    virtual void pause();  // 暂停播放（音视频插件实现）
    virtual void stop();   // 停止播放（音视频插件实现）
    virtual void handleBeforDestroy(); // 销毁前回调

Q_SIGNALS:
    void titleChanged(); // 标题更新时发射，通知对话框刷新标题栏
};

} // namespace dfmbase
```

#### 接口说明

**必须实现的接口**

| 接口 | 说明 |
|------|------|
| `setFileUrl(url)` | 加载指定 URL 对应的文件内容，成功返回 `true`，失败返回 `false` |
| `fileUrl()` | 返回当前已加载的文件 URL |
| `contentWidget()` | 返回用于展示预览内容的主 Widget，不能返回 `nullptr` |

**可选实现的接口**

| 接口 | 说明 |
|------|------|
| `initialize(window, statusBar)` | 在 `setFileUrl` 调用前执行一次性初始化，`window` 为父窗口，`statusBar` 为状态栏容器 |
| `statusBarWidget()` | 返回自定义状态栏 Widget（如播放控制条），默认返回 `nullptr` |
| `statusBarWidgetAlignment()` | 状态栏 Widget 的对齐方式，默认居中 |
| `title()` | 返回预览标题，默认返回文件名 |
| `showStatusBarSeparator()` | 是否在状态栏上方显示分隔线，默认 `false` |
| `play()` / `pause()` / `stop()` | 音视频类插件需实现，框架按需调用 |
| `handleBeforDestroy()` | 对象销毁前的清理回调，可在此释放与 UI 相关的资源 |

### MIME 类型元数据文件

每个插件必须附带一个 JSON 元数据文件，声明该插件所支持的文件类型键（MIME 类型）。

**文件格式**：

```json
{
    "Keys" : ["image/*"]
}
```

**说明**：

- `Keys` 数组中的每一项是一个 MIME 类型模式字符串
- 支持通配符形式，如 `image/*` 匹配全部图片类型
- 也可指定精确 MIME 类型，如 `application/pdf`
- 同一插件可声明多个键，如 `["audio/mpeg", "audio/ogg"]`
- 文件名通常与插件库名保持一致，如 `dde-image-preview-plugin.json`

**常见 MIME 类型参考**：

| 文件类型 | MIME 类型模式 |
|---------|------------|
| 所有图片 | `image/*` |
| 所有音频 | `audio/*` |
| 所有视频 | `video/*` |
| 所有文本 | `text/*` |
| PDF | `application/pdf` |
| Markdown | `text/markdown` |

## 开发指南

### 项目结构

推荐的预览插件项目结构：

```
my-preview-plugin/
├── CMakeLists.txt
├── my-preview-plugin.json        # MIME 类型元数据
├── mypreviewplugin.h             # 插件工厂类声明
├── mypreviewplugin.cpp           # 插件工厂类实现
├── mypreview.h                   # 预览视图类声明
└── mypreview.cpp                 # 预览视图类实现
```

### CMakeLists.txt 配置

```cmake
cmake_minimum_required(VERSION 3.10)
project(my-preview-plugin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(dfm6-base REQUIRED)   # 提供 AbstractBasePreview 等头文件

file(GLOB PLUGIN_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

add_library(${PROJECT_NAME} SHARED ${PLUGIN_SOURCES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt6::Core
    Qt6::Widgets
    dfm6-base
)

# 插件安装目录（release 构建请对应调整）
set(DFM_PLUGIN_PREVIEW_DIR
    ${CMAKE_INSTALL_FULL_LIBDIR}/dde-file-manager/plugins/previews
)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${DFM_PLUGIN_PREVIEW_DIR}
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/my-preview-plugin.json
    DESTINATION ${DFM_PLUGIN_PREVIEW_DIR}
)
```

### 插件工厂类实现

**头文件 `mypreviewplugin.h`**：

```cpp
#pragma once

#include <dfm-base/interfaces/abstractfilepreviewplugin.h>
#include <QObject>

class MyPreviewPlugin : public DFMBASE_NAMESPACE::AbstractFilePreviewPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FilePreviewFactoryInterface_iid FILE "my-preview-plugin.json")

public:
    explicit MyPreviewPlugin(QObject *parent = nullptr);

    /**
     * @brief Creates a preview instance for the given MIME type key.
     * @param key The MIME type string, e.g. "application/x-mytype"
     * @return A new MyPreview instance, or nullptr if the key is not supported.
     */
    DFMBASE_NAMESPACE::AbstractBasePreview *create(const QString &key) override;
};
```

**实现文件 `mypreviewplugin.cpp`**：

```cpp
#include "mypreviewplugin.h"
#include "mypreview.h"

MyPreviewPlugin::MyPreviewPlugin(QObject *parent)
    : AbstractFilePreviewPlugin(parent)
{
}

DFMBASE_NAMESPACE::AbstractBasePreview *MyPreviewPlugin::create(const QString &key)
{
    Q_UNUSED(key)
    return new MyPreview(this);
}
```

### 预览视图类实现

**头文件 `mypreview.h`**：

```cpp
#pragma once

#include <dfm-base/interfaces/abstractbasepreview.h>
#include <QPointer>
#include <QUrl>

class QWidget;
class QLabel;

class MyPreview : public DFMBASE_NAMESPACE::AbstractBasePreview
{
    Q_OBJECT

public:
    explicit MyPreview(QObject *parent = nullptr);
    ~MyPreview() override;

    // ── 必须实现 ──────────────────────────────────────────────
    bool setFileUrl(const QUrl &url) override;
    QUrl fileUrl() const override;
    QWidget *contentWidget() const override;

    // ── 可选实现 ──────────────────────────────────────────────
    void initialize(QWidget *window, QWidget *statusBar) override;
    QString title() const override;

private:
    QUrl m_currentUrl;
    QPointer<QWidget> m_contentWidget;
    QString m_title;
};
```

**实现文件 `mypreview.cpp`**：

```cpp
#include "mypreview.h"

#include <QLabel>
#include <QVBoxLayout>

MyPreview::MyPreview(QObject *parent)
    : AbstractBasePreview(parent)
{
}

MyPreview::~MyPreview()
{
    // contentWidget 由 Qt 父子关系或框架管理，通常无需手动 delete
    if (m_contentWidget)
        m_contentWidget->deleteLater();
}

void MyPreview::initialize(QWidget *window, QWidget *statusBar)
{
    Q_UNUSED(window)
    Q_UNUSED(statusBar)

    // 创建主内容 Widget（懒加载，在此一次性创建）
    auto *widget = new QWidget;
    auto *layout = new QVBoxLayout(widget);
    // ... 组装 UI ...
    m_contentWidget = widget;
}

bool MyPreview::setFileUrl(const QUrl &url)
{
    if (!url.isValid())
        return false;

    m_currentUrl = url;
    m_title = url.fileName();

    // ... 加载并渲染文件内容 ...

    emit titleChanged();
    return true;
}

QUrl MyPreview::fileUrl() const
{
    return m_currentUrl;
}

QWidget *MyPreview::contentWidget() const
{
    return m_contentWidget;
}

QString MyPreview::title() const
{
    return m_title;
}
```

### 音视频插件的播放控制

如果插件需要支持媒体播放，需额外实现 `play()`、`pause()`、`stop()` 以及可选的 `statusBarWidget()`：

```cpp
// mypreview.h 中新增
QWidget *statusBarWidget() const override;
Qt::Alignment statusBarWidgetAlignment() const override;
void play() override;
void pause() override;
void stop() override;
void handleBeforDestroy() override;

// mypreview.cpp 中实现
QWidget *MyPreview::statusBarWidget() const
{
    return m_controlBar; // 自定义播放控制条 Widget
}

Qt::Alignment MyPreview::statusBarWidgetAlignment() const
{
    return Qt::AlignLeft;
}

void MyPreview::play()
{
    // 开始播放逻辑
}

void MyPreview::pause()
{
    // 暂停播放逻辑
}

void MyPreview::stop()
{
    // 停止播放，释放解码资源
}

void MyPreview::handleBeforDestroy()
{
    stop();
    // 释放其他非 Qt 父子树管理的资源
}
```

### 构建流程

```bash
# 常规的 CMake 构建步骤如下：
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build

# 构建完成后，您可以通过以下命令安装：
sudo cmake --build build --target install

# 项目提供了 debian 文件夹，用于在 deepin Linux 桌面发行版下构建软件包。要构建软件包，请使用以下命令：
sudo apt build-dep . # 安装构建依赖
dpkg-buildpackage -uc -us -nc -b # 构建二进制包
```

### 调试方法

#### 日志输出

插件运行在 `dde-file-manager-preview` 独立进程中，可从终端启动以查看输出：

```bash
# 通过 journalctl 实时过滤预览进程日志
journalctl -f | grep dde-file-manager-preview
```

在插件代码中使用 dfm-base 提供的日志宏：

```cpp
#include <dfm-base/dfm_log_defines.h>

fmDebug()    << "Plugin loaded, key:" << key;
fmInfo()     << "File loaded successfully:" << url;
fmWarning()  << "Unsupported file format:" << url;
fmCritical() << "Failed to open file:" << url;
```

日志级别使用规范：
- `fmDebug()`：详细流程和中间状态，仅在开发阶段开启
- `fmInfo()`：关键操作成功（插件加载、文件加载完成）
- `fmWarning()`：不影响功能的潜在问题（格式不支持、降级处理）
- `fmCritical()`：功能性错误（文件无法打开、Widget 创建失败）

#### 符号验证

安装插件后，可验证导出符号是否正确：

```bash
# 检查插件是否有正确的 Qt 插件元数据符号
nm -D /usr/lib/x86_64-linux-gnu/dde-file-manager/plugins/previews/my-preview-plugin.so \
    | grep -i "qt_plugin"
```

#### 依赖检查

```bash
# 确认插件所有依赖库均已满足
ldd my-preview-plugin.so

# 预期应包含
# libDFM6Base.so
# libQt6Core.so.6
# libQt6Widgets.so.6
```

#### 加载验证

若插件未被框架识别，逐步检查以下几点：

1. JSON 文件名与 `Q_PLUGIN_METADATA(IID ... FILE "xxx.json")` 中的文件名一致
2. JSON 文件与 `.so` 安装在同一目录下
3. `.so` 文件使用 Qt6 编译（Qt5 与 Qt6 不兼容）
4. IID 字符串与框架中定义的 `FilePreviewFactoryInterface_iid` 完全一致

### 最佳实践

#### 1. contentWidget 的懒加载

`contentWidget()` 会在 `setFileUrl()` 之前由框架调用以获取 Widget 指针，因此推荐在 `initialize()` 阶段完成 Widget 创建，而非构造函数中：

```cpp
void MyPreview::initialize(QWidget *window, QWidget *statusBar)
{
    Q_UNUSED(window)
    Q_UNUSED(statusBar)

    // 一次性创建 Widget，initialize 只调用一次
    auto *widget = new QWidget;
    // ... 组装 UI ...
    m_contentWidget = widget;
}
```

#### 2. 异步文件加载

对于耗时的文件解析（如大型 PDF、高分辨率图片），应在后台线程加载，通过信号槽切回主线程更新 UI：

```cpp
bool MyPreview::setFileUrl(const QUrl &url)
{
    m_currentUrl = url;

    // 启动后台加载，不阻塞 UI 线程
    auto future = QtConcurrent::run([this, url]() {
        // 在工作线程中解析文件
        auto data = parseFile(url.toLocalFile());
        // 通过 QMetaObject::invokeMethod 切回主线程更新 UI
        QMetaObject::invokeMethod(this, [this, data]() {
            m_contentWidget->update(data);
            emit titleChanged();
        }, Qt::QueuedConnection);
    });

    return true;
}
```

#### 3. 正确实现 sizeHint()

框架通过 `contentWidget()->adjustSize()` 后读取 `size()` 来决定预览窗口大小。若不覆写 `sizeHint()`，窗口可能过小。推荐为内容 Widget 提供合理的默认尺寸：

```cpp
QSize MyContentWidget::sizeHint() const
{
    return QSize(720, 540);
}

QSize MyContentWidget::minimumSizeHint() const
{
    return QSize(480, 360);
}
```

#### 4. handleBeforDestroy() 的正确使用

框架在销毁预览实例前会调用 `handleBeforDestroy()`，应在此停止所有异步操作，避免 Widget 销毁后仍有回调尝试访问它：

```cpp
void MyPreview::handleBeforDestroy()
{
    // 停止播放、取消网络请求、cancel 异步任务
    if (m_futureWatcher.isRunning()) {
        m_futureWatcher.cancel();
        m_futureWatcher.waitForFinished();
    }
    stop();
}
```

## 部署说明

### 安装位置

预览插件（`.so` 文件）及其元数据 JSON 文件均需安装到以下目录：

```
/usr/lib/<arch>/dde-file-manager/plugins/previews/
```

其中 `<arch>` 为系统架构标识，如 `x86_64-linux-gnu` 或 `aarch64-linux-gnu`。

**示例**：

```
/usr/lib/x86_64-linux-gnu/dde-file-manager/plugins/previews/
├── dde-image-preview-plugin.so
├── dde-image-preview-plugin.json
├── my-preview-plugin.so
└── my-preview-plugin.json
```

### 安装方式

#### 方式一：CMake 直接安装

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build -j$(nproc)
sudo cmake --build build --target install
```

#### 方式二：Debian 包安装（推荐用于分发）

```bash
# 安装构建依赖
sudo apt build-dep .
# 构建二进制包
dpkg-buildpackage -uc -us -nc -b
# 安装产物
sudo dpkg -i ../my-preview-plugin_1.0.0_amd64.deb
```

#### 方式三：手动复制

```bash
sudo cp my-preview-plugin.so \
    /usr/lib/$(dpkg-architecture -qDEB_HOST_MULTIARCH)/dde-file-manager/plugins/previews/
sudo cp my-preview-plugin.json \
    /usr/lib/$(dpkg-architecture -qDEB_HOST_MULTIARCH)/dde-file-manager/plugins/previews/
```

### 激活机制

插件安装后自动生效，无需额外配置。文件管理器进程每次触发预览时，`PreviewPluginLoader` 会：

1. 扫描插件目录下所有 `.so` 文件
2. 解析**同名** `.json` 元数据文件中的 `Keys` 字段
3. 将文件的 MIME 类型与 `Keys` 中声明的类型进行匹配
4. 若匹配成功，加载对应 `.so`，调用工厂的 `create()` 方法创建预览实例

### 卸载方式

```bash
# 手动删除
sudo rm /usr/lib/<arch>/dde-file-manager/plugins/previews/my-preview-plugin.so
sudo rm /usr/lib/<arch>/dde-file-manager/plugins/previews/my-preview-plugin.json

# 若通过 Debian 包安装
sudo apt remove my-preview-plugin
```

卸载后无需重启，下次触发预览时自动不再加载该插件。

## 参考示例

### 完整示例项目

完整的文件夹预览插件示例位于：
[folder-preview-example](https://github.com/linuxdeepin/dde-file-manager/tree/master/examples/folder-preview-example)

该示例实现了对 `inode/directory`（文件夹）类型的预览，展示文件数量、子文件夹数量、递归总大小及文件列表，可作为开发新插件的起点。

### 插件工厂类示例

```cpp
// folderpreviewplugin.h
#pragma once
#include <dfm-base/interfaces/abstractfilepreviewplugin.h>

class FolderPreviewPlugin : public DFMBASE_NAMESPACE::AbstractFilePreviewPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FilePreviewFactoryInterface_iid
                      FILE "dde-folder-preview-plugin.json")
public:
    explicit FolderPreviewPlugin(QObject *parent = nullptr);

    /**
     * @brief Creates a FolderPreview for the given MIME key.
     * @param key  MIME type string, expected "inode/directory"
     * @return New FolderPreview instance; the caller takes ownership.
     */
    DFMBASE_NAMESPACE::AbstractBasePreview *create(const QString &key) override;
};

// folderpreviewplugin.cpp
#include "folderpreviewplugin.h"
#include "folderpreview.h"

FolderPreviewPlugin::FolderPreviewPlugin(QObject *parent)
    : AbstractFilePreviewPlugin(parent) {}

DFMBASE_NAMESPACE::AbstractBasePreview *FolderPreviewPlugin::create(const QString &key)
{
    Q_UNUSED(key)
    return new FolderPreview(this);
}
```

### 预览视图类示例

```cpp
// folderpreview.h
#pragma once
#include <dfm-base/interfaces/abstractbasepreview.h>
#include <QPointer>
#include <QUrl>

class FolderContentWidget;

class FolderPreview : public DFMBASE_NAMESPACE::AbstractBasePreview
{
    Q_OBJECT
public:
    explicit FolderPreview(QObject *parent = nullptr);
    ~FolderPreview() override;

    bool setFileUrl(const QUrl &url) override;
    QUrl fileUrl() const override;
    QWidget *contentWidget() const override;
    QString title() const override;
    bool showStatusBarSeparator() const override;

private:
    QUrl m_currentUrl;
    QString m_title;
    QPointer<FolderContentWidget> m_widget;
};

// folderpreview.cpp
#include "folderpreview.h"
#include "foldercontentwidget.h"
#include <QFileInfo>
#include <QDebug>

DFMBASE_USE_NAMESPACE

FolderPreview::FolderPreview(QObject *parent)
    : AbstractBasePreview(parent)
{
    fmInfo() << "FolderPreview: instance created";
}

FolderPreview::~FolderPreview()
{
    if (m_widget)
        m_widget->deleteLater();
}

bool FolderPreview::setFileUrl(const QUrl &url)
{
    if (!url.isValid() || !url.isLocalFile()) {
        fmWarning() << "FolderPreview: invalid or non-local URL:" << url;
        return false;
    }
    if (!QFileInfo(url.toLocalFile()).isDir()) {
        fmWarning() << "FolderPreview: path is not a directory:" << url;
        return false;
    }
    if (m_currentUrl == url)
        return true;

    m_currentUrl = url;
    m_title = QFileInfo(url.toLocalFile()).fileName();
    if (m_title.isEmpty())
        m_title = url.toLocalFile();

    if (m_widget)
        m_widget->loadFolder(url);

    emit titleChanged();
    return true;
}

QUrl FolderPreview::fileUrl() const { return m_currentUrl; }

QWidget *FolderPreview::contentWidget() const
{
    if (!m_widget) {
        auto *self = const_cast<FolderPreview *>(this);
        self->m_widget = new FolderContentWidget;
        if (m_currentUrl.isValid())
            self->m_widget->loadFolder(m_currentUrl);
    }
    return m_widget;
}

QString FolderPreview::title() const { return m_title; }
bool FolderPreview::showStatusBarSeparator() const { return true; }
```

### MIME 类型元数据文件示例

```json
{
    "Keys" : ["inode/directory"]
}
```

### CMakeLists.txt 完整示例

```cmake
cmake_minimum_required(VERSION 3.10)
project(dde-folder-preview-plugin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Concurrent)
find_package(dfm6-base REQUIRED)

file(GLOB PLUGIN_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

add_library(${PROJECT_NAME} SHARED ${PLUGIN_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core Qt6::Widgets Qt6::Concurrent dfm6-base
)

include(GNUInstallDirs)
set(DFM_PLUGIN_PREVIEW_DIR
    ${CMAKE_INSTALL_FULL_LIBDIR}/dde-file-manager/plugins/previews
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${DFM_PLUGIN_PREVIEW_DIR})
install(FILES dde-folder-preview-plugin.json DESTINATION ${DFM_PLUGIN_PREVIEW_DIR})
```

### 内置插件参考

框架内置的以下插件均可作为开发参考：

| 参考插件 | 路径 | 适合学习 |
|---------|------|---------|
| image-preview | `pluginpreviews/image-preview/` | 基础静态内容预览，`canPreview()` 按格式判断 |
| music-preview | `pluginpreviews/music-preview/` | 含播放控制的媒体预览，自定义 `statusBarWidget()` |
| text-preview  | `pluginpreviews/text-preview/`  | 纯文本渲染，流式加载大文件 |
| markdown-preview | `pluginpreviews/markdown-preview/` | 富文本渲染，WebView 嵌入 |
| pdf-preview   | `pluginpreviews/pdf-preview/`   | 复杂多页渲染，异步加载 |

## 注意事项

### 安全性

预览插件运行在文件管理器**主进程**空间内（通过 `QPluginLoader` 动态加载），需注意：

1. **避免阻塞主线程**：文件读取、网络请求、数据库查询等耗时操作必须在后台线程中完成，否则会导致整个文件管理器 UI 冻结。
2. **输入校验**：在 `setFileUrl()` 中验证 URL 合法性（`isLocalFile()`、`QFileInfo::exists()`）再进行文件操作，防止路径注入。
3. **资源释放**：在 `handleBeforDestroy()` 中取消所有后台任务（`QFutureWatcher::cancel()`、`QProcess::kill()`），避免野指针访问。

### 常见问题排查

| 问题现象 | 可能原因 | 解决方法 |
|---------|---------|---------|
| 插件未被加载 | JSON 文件路径或名称不正确 | 检查 `Q_PLUGIN_METADATA FILE` 与实际文件名一致，并确保两者安装在同一目录 |
| 预览窗口过小 / 显示截断 | `contentWidget()` 未提供合适的 `sizeHint()` | 重写 `sizeHint()` — 推荐至少返回 `QSize(600, 480)` |
| MIME 未匹配 | 系统 MIME 数据库未包含该扩展名 | 使用 `file --mime-type <文件>` 验证目标文件的实际 MIME 类型 |
| 构建时找不到头文件 | 未正确查找 DFM 包 | CMakeLists.txt 中添加 `find_package(dfm6-base REQUIRED)` 并链接 `dfm6-base` |
| 预览切换后 UI 无更新 | `setFileUrl()` 未触发数据加载 | 确保每次 `setFileUrl()` 都调用了加载逻辑，不要因 URL 相同而直接跳过 |

### 内存管理

1. `contentWidget()` 返回的 Widget 不能是栈对象，必须在堆上创建。
2. 推荐在 `contentWidget()` 首次调用时（懒加载）创建 Widget，并用 `QPointer` 持有，以便安全判断是否已销毁。
3. 若 Widget 设置了 Qt 父对象，由 Qt 父子树管理，无需手动 `delete`；若未设置父对象，在析构函数中调用 `deleteLater()`。
4. **禁止** 在同一 `QObject` 上混用 Qt 父子关系和 `std::unique_ptr` / `std::shared_ptr` 管理，避免 double free。
5. 后台线程回调触发时 `this` 可能已被销毁——使用 `QPointer` 守护，或在 `handleBeforDestroy()` 中取消 `QFutureWatcher`。

### 线程安全

1. 所有 UI 创建和操作必须在主线程完成。
2. 若需要在后台线程加载文件内容，应通过信号槽（`Qt::QueuedConnection`）将结果传回主线程后再更新 UI。
3. `setFileUrl()` 通常在主线程调用，如需异步加载，务必确保 UI 操作回到主线程。
4. 不要在后台线程直接操作 `QWidget` 或调用 `update()`。

### MIME 类型匹配

1. 框架使用 `QMimeDatabase` 检测文件类型，JSON 中的 `Keys` 必须与 Qt 识别的 MIME 字符串一致。
2. 通配符 `image/*` 能匹配所有 `image/` 前缀的类型，但精确匹配优先于通配匹配。
3. 若同一类型有多个插件匹配，框架选择第一个成功 `create()` 的插件；需要覆盖内置插件时，可将 `.so` 放到相同目录并用更高优先级的名称排序。

### 兼容性

1. 插件必须使用 **Qt6** 编译，Qt5 编译产物与当前框架不兼容。
2. `Q_PLUGIN_METADATA` 中的 IID `com.deepin.filemanager.FilePreviewFactoryInterface_iid` 必须与框架保持一致，否则插件无法被识别。
3. 插件不应假设日志框架、D-Bus 等全局服务已初始化完成，应在实际使用时进行延迟初始化。
4. 插件的 ABI 与 `libdde-file-manager.so` 版本绑定，主库升级后需重新构建。

### 最佳实践总结

1. **懒加载** `contentWidget()`：首次调用时创建 UI，`setFileUrl()` 只更新数据，避免每次预览切换都重建控件。
2. **异步 I/O**：使用 `QtConcurrent::run + QFutureWatcher` 或 `QProcess` 在后台完成文件操作，主线程只做渲染。
3. **提供合理的 `sizeHint()`**：建议至少 `QSize(600, 480)`，让预览窗口在第一次显示时即呈现完整内容。
4. **在 `handleBeforDestroy()` 中清理资源**：取消所有后台任务，与框架析构时序完全解耦。
5. **英文日志覆盖关键路径**：插件加载、文件切换、错误捕获位置均应有 `fmInfo`/`fmWarning`/`fmCritical` 记录。
6. **最小化对外依赖**：若使用额外三方库，需在 `debian/control` 中添加对应 `Depends`。

## 附录

### 相关文档

| 文档 | 链接 |
|-----|------|
| Qt Plugin System | <https://doc.qt.io/qt-6/plugins-howto.html> |
| QMimeDatabase | <https://doc.qt.io/qt-6/qmimedatabase.html> |
| Freedesktop MIME 规范 | <https://specifications.freedesktop.org/shared-mime-info-spec/latest/> |
| dde-file-manager 主仓库 | <https://github.com/linuxdeepin/dde-file-manager> |
| dfm-extension 开发文档 | `docs/extension/05-dfm-extension-plugin.md` |

### 参考资源

| 资源 | 路径 |
|-----|------|
| 完整示例插件 | `examples/folder-preview-example` |
| 内置预览插件 | `src/plugins/pluginpreviews/` |
| 核心接口定义 | `src/dfm-base/interfaces/abstractbasepreview.h` |
| 插件加载器 | `src/dfm-base/base/application/filepreviewfactory.h` |

### 版本历史

| 版本 | 日期 | 说明 |
|-----|------|------|
| 5.x | 2020-2023 | 初始版本，涵盖核心接口与开发指南 |
| 6.0.0 | 2024 | 新增完整示例插件（folder-preview-example）及 Debian 打包说明 |
