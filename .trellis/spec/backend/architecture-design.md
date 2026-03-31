# 架构设计规范

> DDE 应用架构设计的规范和约束。

---

## 概览

本指南定义功能开发阶段的架构分析要求，遵循 DDE 约定和最佳实践。

---

## 架构设计原则

### 1. 分层架构

```
┌─────────────────────────────────────┐
│        UI Layer (DTK Widgets)       │
│  - DMainWindow, DDialog, DListView  │
└─────────────────────────────────────┘
               ↕ Signals/Slots
┌─────────────────────────────────────┐
│      Business Logic Layer           │
│  - Controllers, Services, Models    │
└─────────────────────────────────────┘
               ↕
┌─────────────────────────────────────┐
│      Data Access Layer              │
│  - GIO/GVfs, DBus, QSettings       │
└─────────────────────────────────────┘
```

### 2. 命名空间约定

```cpp
// 基础层 - dfmbase
namespace dfmbase {
    namespace interfaces { ... }
    namespace widgets { ... }
    namespace utils { ... }
}

// 框架层 - dpf (Deepin Plugin Framework)
namespace dpf {
    namespace event { ... }
    namespace lifecycle { ... }
}

// 扩展层 - Extension
namespace Extension {
    namespace menu { ... }
    namespace emblemicon { ... }
}
```

### 3. 插件化架构

参考 `plugin-architecture.md` 获取完整的插件系统规范。

---

## DDE 约定遵循

### 必须使用的组件

| 场景 | 必须使用 | 禁止使用 |
|------|---------|---------|
| 主窗口 | `DMainWindow` | `QMainWindow` |
| 对话框 | `DDialog` | `QDialog` |
| 消息框 | `DMessageBox` | `QMessageBox` |
| 基础组件 | `DWidget` | `QWidget` |
| 列表视图 | `DListView` | `QListView` |
| 输入框 | `DLineEdit` | `QLineEdit` |

### 主题适配

```cpp
// ✅ 正确
QPalette palette = DGuiApplicationHelper::instance()->applicationPalette();
widget->setPalette(palette);

// ❌ 错误
widget->setStyleSheet("color: #333333;");
```

### 国际化

```cpp
// 所有面向用户的文本必须使用 tr()
button->setText(tr("Save"));
dialog->setTitle(tr("Confirm"));

// CMake 添加 TS 文件
find_package(Qt6LinguistTools REQUIRED)
qt_add_translations(app TS_FILES translations/app_zh_CN.ts)
```

---

## 接口定义

### 模型接口

```cpp
// 优先使用 QAbstractListModel/QAbstractTableModel
class FileListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        FilePathRole = Qt::UserRole + 1,
        FileNameRole,
        FileSizeRole
    };

    explicit FileListModel(QObject *parent = nullptr);

    // 必须实现的虚函数
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
};
```

### 服务接口

```cpp
// 定义清晰的服务接口
class IFileService {
public:
    virtual ~IFileService() = default;

    virtual Result copyFile(const QString &source, const QString &dest) = 0;
    virtual Result moveFile(const QString &source, const QString &dest) = 0;
    virtual Result deleteFile(const QString &path) = 0;
};
```

---

## 模块结构

### 标准模块目录

```
src/
├── widgets/          # UI 组件
├── controllers/      # 控制器
├── models/           # 数据模型
├── services/         # 服务
├── utils/            # 工具函数
└── interfaces/       # 接口定义
```

### Pimpl 模式实现

```cpp
// interface.h
class MyWidget : public DWidget {
    Q_OBJECT
public:
    explicit MyWidget(QWidget *parent = nullptr);
    ~MyWidget() override;

    void setValue(int value);
    int value() const;

private:
    class Private;
    QScopedPointer<Private> d;
};

// impl.cpp
class MyWidget::Private {
public:
    int value = 0;
    DLabel *label = nullptr;
    DLineEdit *lineEdit = nullptr;
};

MyWidget::MyWidget(QWidget *parent)
    : DWidget(parent)
    , d(new Private())
{
    d->label = new DLabel(this);
    d->lineEdit = new DLineEdit(this);
}
```

---

## 并发和线程

### 后台任务模式

```cpp
// 使用 QtConcurrent 执行耗时操作
void Controller::loadLargeFile(const QString &path) {
    QFuture<Result> future = QtConcurrent::run([path]() {
        return FileService::loadFile(path);
    });

    QFutureWatcher<Result> *watcher = new QFutureWatcher<Result>(this);
    connect(watcher, &QFutureWatcher<Result>::finished, this, [this, watcher]() {
        Result result = watcher->result();
        handleLoadComplete(result);
        watcher->deleteLater();
    });
    watcher->setFuture(future);
}
```

### 线程安全信号

```cpp
// 跨线程通信必须使用 QueuedConnection
connect(worker, &Worker::progress, ui, &UI::updateProgress,
        Qt::QueuedConnection);
```

---

## 错误处理

### Result 模式

```cpp
struct Result {
    bool success = false;
    QString error;
    QVariant data;

    static Result ok(const QVariant &data = {}) {
        return {true, QString(), data};
    }

    static Result fail(const QString &error) {
        return {false, error, QVariant()};
    }
};

// 使用
Result result = service.copyFile(source, dest);
if (!result.success) {
    DDialog dialog;
    dialog.setTitle(tr("Error"));
    dialog.setMessage(result.error);
    dialog.exec();
}
```

### 异常处理

```cpp
// DDE 应用优先使用错误码而非异常
// 如果使用异常，确保捕获所有路径
try {
    // 操作
} catch (const std::exception &e) {
    qWarning() << "Exception:" << e.what();
    // 显示错误对话框
}
```

---

## 实施计划 (WBS)

### 1. 需求分析
- [ ] 收集功能需求
- [ ] 识别 DDE 相关约束
- [ ] 评估技术可行性

### 2. 架构设计
- [ ] 设计模块结构
- [ ] 定义接口契约
- [ ] 规划数据流

### 3. 原型设计
- [ ] 创建 UI 原型
- [ ] 验证 DTK 组件选择
- [ ] 用户确认

### 4. 详细设计
- [ ] 设计数据库/存储
- [ ] 设计 API/DBus 接口
- [ ] 设计测试策略

### 5. 实施计划
- [ ] 任务分解
- [ ] 依赖关系
- [ ] 时间估算
