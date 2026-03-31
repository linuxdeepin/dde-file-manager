# GThread UI 线程安全陷阱

> 在 DDE 应用中跨线程更新 UI 常见的安全问题。

---

## 问题描述 (P0)

从非主线程直接更新 UI 组件导致应用崩溃或随机出现显示异常。

---

## 根本原因

Qt UI 组件不是线程安全的，只能在主线程（GUI 线程）中创建和操作。直接从工作线程访问或修改 UI 会导致未定义行为。

---

## 错误示例

### 1. 工作线程直接更新 UI

```cpp
// ❌ 错误: 从工作线程更新 UI
class Worker : public QObject {
    Q_OBJECT
public:
    void doWork() {
        // 崩溃！不在主线程
        label->setText("Processing...");
        progressBar->setValue(50);
    }
    QLabel *label;
    QProgressBar *progressBar;
};
```

### 2. AutoConnection 跨线程陷阱

```cpp
// ❌ 错误: 默认 AutoConnection 可能导致 DirectConnection
class Controller : public QObject {
public:
    void startWorker() {
        Worker *worker = new Worker;
        worker->moveToThread(&workerThread);

        // AutoConnection 在跨线程时使用 QueuedConnection
        // 但如果 worker 和 controller 在同一线程，使用 DirectConnection
        // 然后如果 worker 移动到其他线程，问题就出现了
        connect(worker, &Worker::progress, this, &Controller::updateUI);
    }
};
```

### 3. 直接访问 QMetaObject::invokeMethod

```cpp
// ❌ 错误: DirectConnection 在错误线程
void Worker::notifyProgress(int value) {
    QMetaObject::invokeMethod(ui, "setProgress",
                             Qt::DirectConnection,  // 错误！
                             Q_ARG(int, value));
}

// ✅ 正确: 使用 Qt::QueuedConnection
void Worker::notifyProgress(int value) {
    QMetaObject::invokeMethod(ui, "setProgress",
                             Qt::QueuedConnection,
                             Q_ARG(int, value));
}
```

---

## 正确做法

### 使用信号槽 + QueuedConnection

```cpp
class Worker : public QObject {
    Q_OBJECT
signals:
    void progressChanged(int value);
    void workComplete(const QString &result);
};

class Controller : public QObject {
    Q_OBJECT
public:
    void startWorker() {
        Worker *worker = new Worker;
        worker->moveToThread(&workerThread);

        // ✅ 正确: 明确指定 QueuedConnection
        connect(worker, &Worker::progressChanged,
                this, &Controller::updateProgress,
                Qt::QueuedConnection);

        connect(worker, &Worker::workComplete,
                this, &Controller::handleComplete,
                Qt::QueuedConnection);
    }

private slots:
    void updateProgress(int value) {
        // 在主线程中安全执行
        progressBar->setValue(value);
    }
};
```

### 检查当前线程

```cpp
// 检查是否在主线程
void setSafeText(const QString &text) {
    if (QThread::currentThread() == qApp->thread()) {
        label->setText(text);
    } else {
        // 通过信号在主线程更新
        emit textChanged(text);
    }
}
```

### QMetaObject::invokeMethod 线程安全

```cpp
// ✅ 正确: 使用 QueuedConnection
// 确保在主线程执行
QThread *mainThread = qApp->thread();
if (QThread::currentThread() != mainThread) {
    QMetaObject::invokeMethod(ui, "setProgress",
                             Qt::QueuedConnection,
                             Q_ARG(int, value));
}
```

---

## 连接类型选择

| 连接类型 | 说明 | 使用场景 |
|---------|------|---------|
| `AutoConnection` | 根据上下文自动选择 | 同线程用 Direct，跨线程用 Queued |
| `DirectConnection` | 立即调用 | 同线程，或确定安全的情况 |
| `QueuedConnection` | 投递到事件循环 | 跨线程，确保线程安全 |
| `BlockingQueuedConnection` | 阻塞等待 | 需要同步结果时（谨慎使用） |

---

## 检测方法

### 静态分析

```cpp
// 断言检查
Q_ASSERT_X(QThread::currentThread() == qApp->thread(),
           "UI update must be on main thread");
```

### 编译时检查

```cpp
// 可以使用 C++11 static_assert 检查线程
constexpr bool IsMainThread() {
    return true;  // 运行时检查
}
```

---

## 预防措施

### 1. 设计原则

- 所有 UI 操作必须在主线程
- 工作线程只做计算，不访问 UI
- 通过信号槽通信

### 代码模式

```cpp
// Worker 类 - 无 UI 依赖
class Worker : public QObject {
    Q_OBJECT
public:
    void process(const QString &data);
signals:
    void finished(const QString &result);
    void error(const QString &message);
};

// Controller 类 - 负责桥接
class Controller : public QObject {
    Q_OBJECT
public:
    void start(const QString &data);

private slots:
    void onFinished(const QString &result);
    void onError(const QString &message);
};
```

### 3. 信号槽约定

```cpp
// ✅ 推荐: 跨线程信号总是指定 QueuedConnection
connect(worker, &Worker::signal, this, &Controller::slot,
        Qt::QueuedConnection);

// ✅ 推荐: 在头文件注释中说明线程要求
/*
 * @brief Update progress bar
 * @note Must be called from main thread
 */
void updateProgress(int value);
```

---

## 代码检查清单

- [ ] 所有 UI 更新在主线程执行
- [ ] 工作线程中设置 moveToThread
- [ ] 跨线程信号使用 Qt::QueuedConnection
- [ ] 在调试模式添加线程检查断言
- [ ] 避免直接从工作线程访问 UI
- [ ] 考虑使用 QtConcurrent 替代手动线程管理
- [ ] 测试多线程场景下 UI 更新
