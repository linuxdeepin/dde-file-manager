# Qt/DTK 编程指南

> Qt6/DTK Widget 开发的编码规范和最佳实践。

---

## 概览

本指南涵盖 DDE 桌面应用开发中的 Qt/DTK 编码模式、DTK 组件使用和最佳实践。

---

## 信号槽模式

### 连接类型

```cpp
// Auto connection (default) - 根据上下文自动选择
connect(sender, &Sender::signal, receiver, &Receiver::slot);

// Direct connection - 同线程，立即执行
connect(sender, &Sender::signal, receiver, &Receiver::slot, Qt::DirectConnection);

// Queued connection - 跨线程，投递到事件循环
connect(sender, &Sender::signal, receiver, &Receiver::slot, Qt::QueuedConnection);

// Blocking queued - 跨线程，阻塞直到处理完成
connect(sender, &Sender::signal, receiver, &Receiver::slot, Qt::BlockingQueuedConnection);
```

### Lambda 连接

```cpp
// 简单 lambda
connect(button, &DPushButton::clicked, this, [this]() {
    handleButtonClick();
});

// 捕获变量
const QString id = "button-001";
connect(button, &DPushButton::clicked, this, [this, id]() {
    qDebug() << "Clicked:" << id;
});

// 带上下文追踪（上下文销毁时自动断开）
connect(timer, &QTimer::timeout, this, [this]() {
    updateProgress();
});
```

### 断开连接模式

```cpp
// 断开发送者的所有连接
disconnect(sender, nullptr, nullptr, nullptr);

// 断开特定信号
disconnect(sender, &Sender::signal, nullptr, nullptr);

// 断开特定连接（使用 QMetaObject::Connection）
QMetaObject::Connection conn = connect(...);
disconnect(conn);
```

---

## 内存管理模式

### Parent-Child 模式

```cpp
// Qt 在父对象删除时自动处理子对象
class MyWidget : public DWidget {
public:
    MyWidget(QWidget *parent = nullptr) : DWidget(parent) {
        // 子组件自动删除
        m_label = new DLabel(this);
        m_button = new DPushButton(this);
    }
private:
    DLabel *m_label;
    DPushButton *m_button;
};
```

### Pimpl 模式（推荐用于 ABI 稳定性）

```cpp
// myclass.h
class MyClass : public DWidget {
    Q_OBJECT
public:
    explicit MyClass(QWidget *parent = nullptr);
    ~MyClass() override;

    void setValue(int value);
    int value() const;

private:
    class Private;
    QSharedPointer<Private> d;
};

// myclass.cpp
class MyClass::Private {
public:
    int value = 0;
    QString name;
};

MyClass::MyClass(QWidget *parent)
    : DWidget(parent)
    , d(new Private())
{
}

MyClass::~MyClass() = default;
```

### 智能指针模式

```cpp
// QSharedPointer 用于共享所有权
QSharedPointer<Resource> resource = QSharedPointer<Resource>::create();

// QScopedPointer 用于独占所有权
QScopedPointer<Worker> worker(new Worker());

// QWeakPointer 用于非拥有引用
QWeakPointer<Resource> weakRef = resource;
if (auto strong = weakRef.lock()) {
    strong->doSomething();
}
```

### Delete Later 模式

```cpp
// 在事件循环中安全删除
widget->deleteLater();

// 在槽处理中
void MyClass::onWorkComplete() {
    m_worker->deleteLater();
    m_worker = nullptr;
}
```

---

## 线程模式

### QThread Worker 模式

```cpp
// Worker 类
class Worker : public QObject {
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);

public slots:
    void doWork();

signals:
    void workComplete(const Result &result);
};

// 使用
QThread *thread = new QThread;
Worker *worker = new Worker;
worker->moveToThread(thread);

connect(thread, &QThread::started, worker, &Worker::doWork);
connect(worker, &Worker::workComplete, this, &MyClass::handleResult);
connect(worker, &Worker::workComplete, thread, &QThread::quit);
connect(thread, &QThread::finished, thread, &QThread::deleteLater);
connect(thread, &QThread::finished, worker, &Worker::deleteLater);

thread->start();
```

### Qt 并发模式

```cpp
#include <QtConcurrent>

// 在线程池中运行
QFuture<Result> future = QtConcurrent::run([this]() {
    return expensiveComputation();
});

// 监听完成
QFutureWatcher<Result> *watcher = new QFutureWatcher<Result>(this);
connect(watcher, &QFutureWatcher<Result>::finished, this, [this, watcher]() {
    Result result = watcher->result();
    handleResult(result);
    watcher->deleteLater();
});
watcher->setFuture(future);
```

---

## 错误处理模式

### DDialog 错误显示

```cpp
void showError(const QString &title, const QString &message) {
    DDialog dialog;
    dialog.setIcon(QIcon::fromTheme("dialog-error"));
    dialog.setTitle(title);
    dialog.setMessage(message);
    dialog.addButton(tr("OK"), true, DDialog::ButtonRecommend);
    dialog.exec();
}
```

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
```

---

## DTK 组件指南

### DDialog

```cpp
#include <DDialog>
#include <DApplication>

// 简单对话框
void showSimpleDialog() {
    DDialog dialog;
    dialog.setTitle(tr("Confirm Action"));
    dialog.setMessage(tr("Are you sure you want to proceed?"));
    dialog.setIcon(QIcon::fromTheme("dialog-question"));

    dialog.addButton(tr("Cancel"), false, DDialog::ButtonNormal);
    dialog.addButton(tr("OK"), true, DDialog::ButtonRecommend);

    if (dialog.exec() == DDialog::Accepted) {
        // 用户点击了 OK
    }
}

// 带自定义内容的对话框
void showCustomDialog() {
    DDialog dialog;
    dialog.setTitle(tr("Settings"));

    QWidget *content = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(content);

    DLineEdit *lineEdit = new DLineEdit;
    lineEdit->setPlaceholderText(tr("Enter value"));
    layout->addWidget(lineEdit);

    dialog.addContent(content);
    dialog.addButton(tr("Cancel"), false, DDialog::ButtonNormal);
    dialog.addButton(tr("Apply"), true, DDialog::ButtonRecommend);

    if (dialog.exec() == DDialog::Accepted) {
        QString value = lineEdit->text();
    }
}
```

### DMainWindow

```cpp
#include <DMainWindow>
#include <DTitlebar>
#include <DWidgetUtil>

class MainWindow : public DMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr)
        : DMainWindow(parent)
    {
        // 设置标题栏
        titlebar()->setTitle("My Application");
        titlebar()->setMenu(new QMenu(this));

        // 添加菜单动作
        QMenu *menu = titlebar()->menu();
        menu->addAction(tr("Settings"), this, &MainWindow::showSettings);
        menu->addSeparator();
        menu->addAction(tr("About"), this, &MainWindow::showAbout);

        // 设置中心组件
        setCentralWidget(createCentralWidget());

        // 窗口属性
        setMinimumSize(800, 600);
        setWindowIcon(QIcon::fromTheme("preferences-system"));
    }

private:
    QWidget* createCentralWidget() {
        QWidget *widget = new QWidget(this);
        QVBoxLayout *layout = new QVBoxLayout(widget);
        return widget;
    }
};
```

### DLineEdit

```cpp
#include <DLineEdit>

// 基本使用
DLineEdit *lineEdit = new DLineEdit;
lineEdit->setPlaceholderText(tr("Enter text"));
lineEdit->setClearButtonEnabled(true);

// 带警告
lineEdit->setAlert(true);
lineEdit->showAlertMessage(tr("Invalid input"));

// 密码模式
DLineEdit *passwordEdit = new DLineEdit;
passwordEdit->setEchoMode(QLineEdit::Password);

// 信号连接
connect(lineEdit, &DLineEdit::textChanged, this, [this](const QString &text) {
    // 处理文本变化
});

connect(lineEdit, &DLineEdit::returnPressed, this, [this]() {
    // 处理回车键
});
```

---

## DTK 工具类

### DApplication

```cpp
#include <DApplication>
#include <DWidgetUtil>

int main(int argc, char *argv[]) {
    DApplication a(argc, argv);

    // 应用信息
    a.setApplicationName("myapp");
    a.setApplicationVersion("1.0.0");
    a.setProductName(QObject::tr("My Application"));
    a.setProductIcon(QIcon::fromTheme("myapp"));

    // 加载翻译
    a.loadTranslator();

    MainWindow w;
    w.show();

    return a.exec();
}
```

### DGuiApplicationHelper (主题)

```cpp
#include <DGuiApplicationHelper>

// 获取当前主题类型
DGuiApplicationHelper::ColorType themeType =
    DGuiApplicationHelper::instance()->themeType();

if (themeType == DGuiApplicationHelper::DarkType) {
    // 深色主题
} else {
    // 浅色主题
}

// 监听主题变化
connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
    this, [this](DGuiApplicationHelper::ColorType type) {
        // 处理主题变化
    });

// 获取调色板（从不硬编码颜色）
QPalette palette = DGuiApplicationHelper::instance()->applicationPalette();
QColor textColor = palette.color(QPalette::WindowText);
```

---

## 设置模式

```cpp
#include <QSettings>

void saveSettings() {
    QSettings settings("org.deepin", "myapp");
    settings.setValue("window/size", size());
    settings.setValue("window/position", pos());
    settings.setValue("value", m_value);
}

void loadSettings() {
    QSettings settings("org.deepin", "myapp");
    resize(settings.value("window/size", QSize(800, 600)).toSize());
    move(settings.value("window/position", QPoint(100, 100)).toPoint());
    m_value = settings.value("value", 0).toInt();
}
```

---

## 最佳实践

### 始终使用 DTK 组件

| 推荐使用 | 禁止使用 |
|---------|---------|
| `DMainWindow` | `QMainWindow` |
| `DDialog` | `QDialog` |
| `DMessageBox` | `QMessageBox` |
| `DWidget` | `QWidget` |
| `DLabel` | `QLabel` |
| `DPushButton` | `QPushButton` |
| `DLineEdit` | `QLineEdit` |
| `DListView` | `QListView` |

### 主题适配

```cpp
// ❌ 错误: 硬编码颜色
label->setStyleSheet("color: #333333;");

// ✅ 正确: 使用调色板
QPalette palette = DGuiApplicationHelper::instance()->applicationPalette();
label->setPalette(palette);
```

### 国际化

```cpp
// 始终对面向用户的文本使用 tr()
button->setText(tr("Save"));
label->setText(tr("Hello World"));

// 代码注释使用英文
// Initialize the widget
m_widget = new DWidget(this);
```

---

## 反模式

```cpp
// ❌ 错误: 没有父对象的原始指针（内存泄漏）
QObject *obj = new QObject();

// ❌ 错误: 阻塞 UI 线程
QThread::sleep(5); // 阻塞 UI!

// ❌ 错误: 跨线程连接没有使用 queued connection
connect(threadWorker, &Worker::signal, uiWidget, &Widget::slot); // 可能崩溃!

// ❌ 错误: 在错误线程中直接删除 QObject
delete worker; // 如果 worker 在不同线程中会崩溃!

// ✅ 正确: 对跨线程删除使用 deleteLater
worker->deleteLater();

// ✅ 正确: 对跨线程信号使用 QueuedConnection
connect(threadWorker, &Worker::signal, uiWidget, &Widget::slot, Qt::QueuedConnection);
```

---

## 快速参考

| 任务 | 解决方案 |
|------|----------|
| 显示对话框 | `DDialog` + `exec()` |
| 创建主窗口 | `DMainWindow` + `titlebar()` |
| 获取用户输入 | `DLineEdit` + `textChanged` 信号 |
| 显示列表 | `DListView` + `QStandardItemModel` |
| 获取主题颜色 | `DGuiApplicationHelper::instance()->applicationPalette()` |
| 线程安全信号 | `Qt::QueuedConnection` |
| 安全删除 | `deleteLater()` |
| 设置存储 | `QSettings("org.deepin", "app")` |
