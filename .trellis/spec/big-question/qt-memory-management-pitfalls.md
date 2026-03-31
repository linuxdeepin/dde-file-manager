# Qt 内存管理陷阱

> Qt 对象生命周期和内存管理的常见陷阱及解决方案。

---

## 问题描述 (P1)

开发 DDE 应用时，经常出现内存泄漏或崩溃，通常与 Qt Parent-Child 机制、信号槽连接和 deleteLater 使用不当有关。

---

## 根本原因

Qt 的自动内存管理依赖于 Parent-Child 关系和信号槽的生命周期绑定，但不当使用会导致：
1. 对象被删除后仍然被访问
2. 跨线程删除导致崩溃
3. 循环引用导致内存泄漏

---

## 错误示例

### 1. 没有父对象的临时对象泄漏

```cpp
// ❌ 错误: 没有父对象，泄漏
void showDialog() {
    DDialog *dialog = new DDialog();
    dialog->exec();
    // 泄漏！从未删除
}

// ✅ 正确: 设置父对象或使用栈对象
void showDialog() {
    DDialog dialog;  // 栈对象自动析构
    dialog.exec();
}

// ✅ 正确: 或者正确管理生命周期
void showDialog() {
    DDialog *dialog = new DDialog(parentWidget);
    connect(dialog, &DDialog::finished, dialog, &DDialog::deleteLater);
    dialog->exec();
}
```

### 2. 跨线程删除崩溃

```cpp
// ❌ 错误: 在错误线程中删除
class Manager : public QObject {
public:
    void stopWorker() {
        delete worker;  // 崩溃！worker 在不同线程
    }
    Worker *worker;
};

// ✅ 正确: 使用 deleteLater
void stopWorker() {
    worker->deleteLater();  // 安全删除
}
```

### 3. 信号槽连接导致的循环引用

```cpp
// ❌ 错误: 循环引用
class Parent : public QObject {
    void setChild(Child *child) {
        m_child = child;
        connect(child, &Child::signal, this, &Parent::handler);
        connect(this, &Parent::signal, child, &Child::handler);
        // 互相引用，可能泄漏
    }
};

// ✅ 正确: 明确所有权或使用 QWeakPointer
class Parent : public QObject {
    void setChild(Child *child) {
        child->setParent(this);  // 明确 Parent 拥有 Child
        connect(child, &Child::signal, this, &Parent::handler);
        // 连接到 Parent 的槽，Parent 拥有 Child
    }
};
```

### 4. deleteLater 后立即使用

```cpp
// ❌ 错误: 计划删除后继续使用
void cleanup() {
    widget->deleteLater();
    widget->show();  // 未定义行为！
}

// ✅ 正确: 释放引用
void cleanup() {
    widget->deleteLater();
    widget = nullptr;  // 立即释放引用
}

// ✅ 正确: 使用信号处理
void cleanup() {
    connect(widget, &QObject::destroyed, this, [this]() {
        // widget 已完全删除
    });
    widget->deleteLater();
}
```

---

## 检测方法

### Valgrind

```bash
valgrind --leak-check=full --show-leak-kinds=all ./myapp
```

### Qt 内存分析

```cpp
// Qt 内存调试模式
#include <QtCore>
QLoggingCategory::setFilterRules("qt.core.memory.debug=true");
```

### ASAN (AddressSanitizer)

```cmake
# CMake
target_compile_options(myapp PRIVATE -fsanitize=address)
target_link_options(myapp PRIVATE -fsanitize=address)
```

---

## 预防措施

### 1. 始终设置父对象

```cpp
// 规则: 所有没有明确所有权的对象都应该设置父对象
class MyWidget : public DWidget {
    MyWidget(QWidget *parent) : DWidget(parent) {
        m_label = new DLabel(this);      // 自动删除
        m_button = new DPushButton(this); // 自动删除
    }
};
```

### 2. 使用智能指针

```cpp
// 规则: 需要共享所有权时使用 QSharedPointer
QSharedPointer<Resource> resource = QSharedPointer<Resource>::create();

// 规则: 非拥有引用使用 QWeakPointer
QWeakPointer<Resource> weakRef = resource;
if (auto strong = weakRef.lock()) {
    strong->use();
}
```

### 3. 正确的信号槽连接

```cpp
// 规则: 跨线程使用 QueuedConnection
connect(worker, &Worker::signal, ui, &UI::handler,
        Qt::QueuedConnection);

// 规则: 连接时使用上下文对象
connect(sender, &Sender::signal, receiver, &Receiver::slot);
// receiver 被销毁时连接自动断开
```

### 4. 对象生命周期明确

```cpp
// 规则: 在头文件的注释中说明对象所有权
class Controller : public QObject {
    // 以下对象由 Controller 拥有
    QScopedPointer<Worker> m_worker;
    QSharedPointer<DataCache> m_cache;

    // 以下为非拥有引用
    QWeakPointer<Config> m_config;
};
```

---

## 代码检查清单

- [ ] 所有 `new` 创建的对象都有父对象或所有者
- [ ] 跨线程操作使用 `deleteLater` 而非直接 `delete`
- [ ] 信号槽连接使用正确的连接类型 (Direct/Queued)
- [ ] 检查潜在的循环引用
- [ ] 在析构函数中断开信号槽
- [ ] 使用 RAII 模式管理资源
- [ ] 定期运行 Valgrind/ASAN 检测泄漏
