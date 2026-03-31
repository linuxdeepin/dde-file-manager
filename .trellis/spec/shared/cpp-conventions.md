# C++ 编码约定

> DDE 应用的 C++17 编码规范和最佳实践。

---

## 概览

本指南定义 DDE 应用的 C++ 编码标准，遵循 C++17 特性和 Qt/DTK 集成。

---

## C++17 特性使用

### 结构化绑定

```cpp
// ✅ 推荐
auto [success, error] = parseResult();

// ✅ 推荐
for (const auto &[key, value] : map) {
    use(key, value);
}
```

### If 初始化

```cpp
// ✅ 推荐
if (auto result = function(); result.isValid()) {
    process(result);
}

// 等价于
auto result = function();
if (result.isValid()) {
    process(result);
}
```

### Optional

```cpp
// ✅ 推荐
std::optional<int> parseNumber(const QString &str) {
    bool ok;
    int value = str.toInt(&ok);
    return ok ? std::optional<int>(value) : std::nullopt;
}

// 使用
if (auto value = parseNumber(input)) {
    process(*value);
}
```

---

## RAII 资源管理

### 智能指针选择

```cpp
// ✅ 推荐: QSharedPointer 共享所有权
QSharedPointer<Resource> resource = QSharedPointer<Resource>::create();

// ✅ 推荐: QScopedPointer 独占所有权
QScopedPointer<Worker> worker(new Worker());

// ✅ 推荐: QWeakPointer 非拥有引用
QWeakPointer<Resource> weakRef = resource;

// ❌ 避免: 除非特殊需要，不用 std::unique_ptr 与 Qt 对象混用
```

### Qt 对象父子关系

```cpp
// ✅ 推荐: 父子关系自动管理
DWidget *parent = new DWindow;
DLabel *child = new DLabel("Text", parent);

// ✅ 推荐: 使用 deleteLater 安全删除
widget->deleteLater();
```

### 文件资源

```cpp
// ✅ 推荐: 使用 QFile 的 RAII
void processFile(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    // 文件自动关闭
}

// ✅ 推荐: 临时文件
QTemporaryFile tempFile;
if (tempFile.open()) {
    // 自动删除
}
```

---

## Pimpl 模式

### 标准 Pimpl

```cpp
// MyClass.h
class MyClass : public QObject {
    Q_OBJECT
public:
    explicit MyClass(QObject *parent = nullptr);
    ~MyClass();

    void setValue(int value);
    int value() const;

private:
    class Private;
    QScopedPointer<Private> d;
};

// MyClass.cpp
class MyClass::Private {
public:
    int value = 0;
    DLabel *label = nullptr;
    QTimer *timer = nullptr;
};

MyClass::MyClass(QObject *parent)
    : QObject(parent)
    , d(new Private())
{
}

MyClass::~MyClass() = default;
```

### 何时使用 Pimpl

- **需要 ABI 稳定性** - 公共 API
- **降低编译依赖** - 减少头文件包含
- **隐藏实现细节** - 不暴露内部结构

---

## 异常处理

### 策略

DDE 应用优先使用错误码而非异常，因为 Qt 主要使用错误码模式。

```cpp
// ✅ 推荐: 使用 Result 模式
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

### 异常捕获

```cpp
// ✅ 推荐: 捕获所有异常
try {
    // 可能抛出异常的代码
} catch (const std::exception &e) {
    qWarning() << "Exception:" << e.what();
    showError(tr("Operation failed"));
} catch (...) {
    qWarning() << "Unknown exception";
    showError(tr("Unknown error"));
}
```

---

## 命名约定

> 本规范遵循 Google C++ 风格指南，deepin/DDE 特例外见下文。

### 文件命名

**遵循 deepin-styleguide 规范：**

```cpp
// ✅ 正确 - 文件名全小写，使用下划线连接
class MyWidget;      // mywidget.h .cpp
class FileManager;  // file_manager.h .cpp
class DMainWindow;  // d_main_window.h .cpp

// ✅ 正确 - 头文件使用 .h，源文件使用 .cpp
mywidget.h / mywidget.cpp

// ✅ 备选 - 只有在下划线无法使用时才考虑连字符
my-useful-class.cpp  // 适用于某些构建系统不支持下划线的情况

// ❌ 错误 - 无连接符，难以阅读
mywidget.cpp         // 不适用
myusefulclass.cpp

// ❌ 错误 - 名称不清晰
MyWidget.h           // 大驼峰
```

### 类和函数

```cpp
// 类: 大驼峰
class MyPlugin : public QObject { };

// 函数: 小驼峰
void initializePlugin();
QString filePath();
bool isValid();

// 成员变量: 见下方 "D-Pointer 模式特殊命名"

// 静态常量: 下划线分隔 + k 前缀
static const int kMaxRetries = 3;
static const QString kDefaultPath = "/tmp";
```

### D-Pointer 模式特殊命名

> 基于 deepin-styleguide Qt 命名规范。

**保留名称**：`d_ptr`, `q_ptr`, `dd_ptr`, `qq_ptr` 保留给 D-Pointer 模式使用。

**成员变量命名规则**：

```cpp
// ✅ 正确 - Private 类中成员变量不加任何前缀
class MyClassPrivate {
public:
    QString name;        // 不加 m_ 前缀
    int value;           // 不加 m_ 前缀
    DLabel *label;       // 不加 m_ 前缀
    static int count;    // 不加 m_ 前缀
};

// ✅ 正确 - 普通类中的成员变量使用 m_ 前缀
class MyClass : public QObject {
private:
    QString m_name;      // 普通类使用 m_ 前缀
    int m_value;         // 普通类使用 m_ 前缀
};

// ✅ 正确 - 结构体成员变量不加前缀（与 Private 类风格一致）
struct ConfigData {
    QString name;        // 不加前缀
    int value;           // 不加前缀
};

// ❌ 错误 - Private 类中错误地使用了 m_ 前缀
class MyClassPrivate {
public:
    QString m_name;      // 不应该在 Private 类中使用 m_ 前缀
};

// ❌ 错误 - 普通类中错误地没有使用 m_ 前缀
class MyClass {
private:
    QString name;        // 应该使用 m_ 前缀
};

// ❌ 错误 - 使用了保留名称
QObject *d_ptr;         // d_ptr 是保留名称
QObject *q_ptr;         // q_ptr 是保留名称
```

**何时使用 Private 类**：

```cpp
// 使用 D-Pointer 模式（通过 d-> 访问 Private 类成员）
class MyClass : public QObject {
    Q_OBJECT
public:
    MyClass(QObject *parent = nullptr);
    ~MyClass();

    QString name() const;
    void setName(const QString &name);

private:
    class Private;
    QScopedPointer<Private> d;
};

// 优势：
// 1. 封装：隐藏实现细节
// 2. ABI 稳定性：不暴露内部成员
// 3. 编译速度：减少头文件依赖

// 注意：在 D-Pointer 模式中访问 Private 类成员：
// d->name（不加 m_ 前缀）
```

---

## Const 正确性

### 常量正确

```cpp
// ✅ 推荐: 不修改成员的函数标记为 const
int value() const {
    return m_value;
}

// ✅ 推荐: 参数可以是 const 的就标记为 const
void process(const QString &text);
void processData(const QByteArray &data);

// ✅ 推荐: 优先使用 const 引用避免拷贝
QString processLargeString(const QString &input);
// 而不是
QString processLargeString(QString input);
```

### 避免 mutable

```cpp
// ❌ 避免: 除非必要，不用 mutable
// mutable 破坏了 const 的语义
// 如果需要，考虑重新设计
int value() const {
    return ++m_cachedValue;  // ❌ mutable
}
```

---

## 头文件管理

> 基于 deepin-styleguide Qt 头文件规范。

### 自包含头文件

**每个 `.h` 文件必须能够自给自足（self-contained）。**

```cpp
// ✅ 正确 - 自包含头文件
// myclass.h
#ifndef MYCLASS_H_
#define MYCLASS_H_

#include <QObject>  // 包含所有需要的头文件

class MyClass : public QObject {
    Q_OBJECT
public:
    explicit MyClass(QObject *parent = nullptr);
};

#endif // MYCLASS_H_

// myclass.cpp
#include "myclass.h"  // 可以直接使用，无需额外的 include

// ❌ 错误 - 头文件依赖其他头文件
// myclass.h（假设没有 #include <QObject>）
class MyClass : public QObject {  // 编译失败！QObject 未定义
    // ...
};

// myclass.cpp
#include <QObject>     // 需要先包含才能使用
#include "myclass.h"
```

### 头文件保护宏

**格式：`<PROJECT>_<PATH>_<FILE>_H_`**

```cpp
// ✅ 正确 - 基于项目路径生成
// 项目: myapp
// 路径: src/core
// 文件: myclass

#ifndef MYAPP_CORE_MYCLASS_H_
#define MYAPP_CORE_MYCLASS_H_

// 代码

#endif // MYAPP_CORE_MYCLASS_H_

// ✅ 正确 - 更简单的格式
#ifndef MYCLASS_H_
#define MYCLASS_H_

// 代码

#endif // MYCLASS_H_
```

### 前置声明策略

**避免使用前置声明，直接使用 `#include`。**

```cpp
// ❌ 避免 - 前置声明
class MyClass;  // 前置声明

void process(MyClass *obj);  // 使用前置声明

// ✅ 推荐 - 直接 include
#include "myclass.h"

void process(MyClass *obj);  // 直接使用
```

**原因**：
1. 避免隐藏依赖关系
2. 减少重构时的编译错误
3. 保持代码可维护性

### 内联函数限制

**只有不超过 10 行的函数才应定义为内联函数。**

```cpp
// ✅ 推荐 - 简短函数可以内联
inline int square(int x) {
    return x * x;
}

// ✅ 推荐 - getter/setter 可以内联
class MyClass {
public:
    int value() const { return m_value; }         // 简短，可以内联
    void setValue(int value) { m_value = value; }
};

// ❌ 避免 - 复杂函数不应内联
inline void complexFunction() {  // 太长了！
    // ... 10+ 行代码 ...
}
```

### #include 顺序

**使用标准顺序：相关头文件 → C 库 → C++ 库 → 其他库 → 本项目。**

```cpp
// myclass.cpp

// 1. 相关头文件（优先位置）
#include "myclass.h"

// 2. C 系统库
#include <sys/types.h>
#include <unistd.h>

// 3. C++ 标准库
#include <memory>
#include <vector>
#include <algorithm>

// 4. 其他库（Qt 等）
#include <QObject>
#include <QList>
#include <QDebug>

// 5. 本项目内头文件
#include "otherclass.h"
#include "utils.h"
```

**好处**：
1. 相关头文件在前，如果缺少依赖会立即发现
2. 按类别分组，易于阅读和维护
3. 避免隐藏依赖

---

## 禁用特性

### 禁止使用

```cpp
// ❌ 禁止: 原始指针直接 new 没有父对象
QObject *obj = new QObject();

// ❌ 禁止: 裸指针 delete
delete widget;
delete[] array;

// ❌ 禁止: malloc/free
void *ptr = malloc(100);
free(ptr);

// ❌ 禁止: C 风格转换
QObject *obj = (QObject*)ptr;

// ❌ 禁止: 数组作为原始数组
char buffer[1024];
```

### 推荐替代

```cpp
// ✅ 推荐: 智能指针
QSharedPointer<Object> obj = QSharedPointer<Object>::create();

// ✅ 推荐: Qt 容器
QVector<int> numbers;
QMap<QString, int> mapping;
QList<QObject*> objects;

// ✅ 推荐: QByteArray/QByteArray
QByteArray data = QByteArray::fromRawData(ptr, size);
QDataBuffer buffer;
```

---

## 内存管理

### Parent-Child 模式

```cpp
// ✅ 推荐: 始终为没有明确所有者的对象设置父对象
class MyWidget : public DWidget {
public:
    explicit MyWidget(QWidget *parent = nullptr) : DWidget(parent) {
        m_label = new DLabel(this);      // 自动删除
        m_button = new DPushButton(this); // 自动删除
    }
private:
    DLabel *m_label;
    DPushButton *m_button;
};
```

### 避免循环引用

```cpp
// ❌ 避免: Parent 和 Child 相互引用
class Parent : public QObject {
    Q_OBJECT
public:
    void setChild(Child *child) {
        m_child = child;
        child->setParent(this);  // 可能导致问题
    }
private:
    Child *m_child;
};

// ✅ 推荐: 只由 Parent 拥有 Child
class Parent : public QObject {
    Q_OBJECT
public:
    void setChild(Child *child) {
        child->setParent(this);
        // 不保存指针
    }
};

// 或使用 QWeakPointer
```

---

## 快速参考

| 特性 | 使用 |
|------|------|
| 结构化绑定 | `auto [a, b] = pair;` |
| Optional | `std::optional<int>` |
| 共享所有权 | `QSharedPointer` |
| 独占所有权 | `QScopedPointer` |
| 非拥有引用 | `QWeakPointer` |
| 文件资源 | `QFile` RAII |
| 错误处理 | `Result` 模式 |
| 类命名 | 大驼峰 |
| 函数命名 | 小驼峰 |
| 成员变量 | 普通类 `m_` 前缀，Private 类无前缀 |
| 文件命名 | 小写 + 下划线（`my_widget.h`） |
| 头文件保护 | `<PROJECT>_<PATH>_<FILE>_H_` |

---

## 参考资源

- [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) - 通用 C++ 规范
- [deepin-styleguide](https://gitlab.deepin.org/styleguide) - deepin/DDE 特有规范
