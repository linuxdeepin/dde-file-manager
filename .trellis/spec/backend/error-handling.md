# 错误处理指南

> DDE File Manager 错误处理规范，包括资源清理、事务管理、异常捕获等模式。

---

## 概述

项目使用多种模式处理错误，确保程序稳定性和数据一致性。

**核心原则**:
- 资源必须正确释放 (RAII)
- 数据库操作使用事务保证一致性
- 关键路径防止崩溃 (catch-all)

---

## FinallyUtil 资源清理

### 模式描述

使用 FinallyUtil 实现 RAII 风格的资源清理，确保在函数退出时执行清理操作。

### 基本用法

```cpp
DFMBASE_NAMESPACE::FinallyUtil finally([&]() {
    // 清理代码
    lastErr.clear();
});

// 如果需要取消清理
finally.dismiss();
```

**示例位置**: `include/dfm-base/utils/finallyutil.h`

```cpp
class FinallyUtil
{
    Q_DISABLE_COPY(FinallyUtil)

public:
    explicit FinallyUtil(std::function<void()> onExit);
    ~FinallyUtil();
    void dismiss(bool dismissed = true);

private:
    std::function<void()> exitFunc;
    bool hasDismissed { false };
};
```

---

### 实际应用

**场景 1: 清理错误状态**

```cpp
QVariantMap TagDbHandler::getAllTags()
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });
    finally.dismiss(); // 暂时取消，如果有错误再启用

    const auto &tagPropertyBean = handle->query<TagProperty>().toBeans();
    if (tagPropertyBean.isEmpty()) {
        fmDebug() << "No tags found in database";
        return {};
    }

    QVariantMap tagPropertyMap;
    for (auto &bean : tagPropertyBean)
        tagPropertyMap.insert(bean->getTagName(), QVariant { bean->getTagColor() });

    return tagPropertyMap;
}
```

**示例位置**: `src/plugins/daemon/tag/tagdbhandler.cpp`

**场景 2: 验证返回清理**

```cpp
bool TagDbHandler::addTagsForFiles(const QVariantMap &data)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (data.isEmpty()) {
        lastErr = "input parameter is empty!";
        fmWarning() << "Empty data provided";
        finally.dismiss(); // 取消清理，保留错误信息
        return false;
    }

    // 执行操作...

    finally.dismiss(); // 成功后取消清理
    return true;
}
```

**要点**:
1. 在函数开始创建 FinallyUtil
2. 失败时调用 `dismiss()` 保留状态
3. 成功后调用 `dismiss()` 避免不必要清理

---

## 事务回滚

### 模式描述

数据库操作必须使用事务包装，任一步失败则全部回滚。

### 标准事务模式

```cpp
bool ret = handle->transaction([tmpData, this]() -> bool {
    for (auto dataIt = tmpData.begin(); dataIt != tmpData.end(); ++dataIt) {
        bool ret = tagFile(dataIt.key(), dataIt.value());
        if (!ret) {
            fmCritical() << "Transaction failed for file:" << dataIt.key();
            return ret;
        }
    }
    return true;
});

if (!ret) {
    fmCritical() << "Transaction failed, automatic rollback executed";
} else {
    fmInfo() << "Operation completed successfully";
}
```

**示例位置**: `src/plugins/daemon/tag/tagdbhandler.cpp`

**关键点**:
- 使用 lambda 包装所有操作
- 任一步失败返回 false 触发回滚
- 所有成功返回 true 触发提交

---

### 事务嵌套

```cpp
// 内层事务
handle->transaction([&]() {
    // 操作 1
    if (!operation1()) return false;

    // 嵌套事务 (SQLite 实际上不支持嵌套，需要 savepoint)
    // 建议避免嵌套，使用同一事务处理
});
```

**注意**: SQLite 不支持真正的嵌套事务，建议在同一事务中完成所有操作。

---

## Q_UNLIKELY 分支优化

### 模式描述

使用 Q_UNLIKELY 标记不太可能执行的错误分支，帮助编译器优化热点路径。

### 基本用法

```cpp
if (Q_UNLIKELY(dirs.isEmpty())) {
    // 处理错误情况 (不常执行)
}

if (Q_UNLIKELY(org.isEmpty() || proj.isEmpty())) {
    // 处理参数错误
}
```

**示例位置**:
- `src/dfm-base/base/standardpaths.cpp`
- `src/dfm-base/utils/fileutils.cpp`

---

### 适用场景

**使用 Q_UNLIKELY**:
- 参数验证失败
- 边界条件检查
- 异常情况处理

**不使用 Q_UNLIKELY**:
- 正常的程序流分支
- 频繁发生的条件判断

---

## 异常捕获

### Catch-All 模式

在关键位置使用 catch-all 防止异常传播导致程序崩溃。

```cpp
} catch (...) {
    // 捕获所有异常，防止崩溃
    fmCritical() << "Uncaught exception occurred";
    return false;
}
```

**示例位置**: `src/dfm-base/utils/fileutils.cpp`

**关键点**:
- 仅在无法预料的异常路径使用
- 记录详细的错误日志
- 提供合理的错误处理或恢复

---

### Qt 异常处理

Qt 一般不建议使用 C++ 异常，推荐使用错误码或信号/槽。

```cpp
// 推荐: 使用返回值和错误状态
bool readFile(const QString &path, QByteArray &data) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        fmCritical() << "Failed to open file:" << path;
        return false;
    }
    data = file.readAll();
    return true;
}

// 推荐: 使用信号报告错误
connect(worker, &Worker::errorOccurred, this, [](const QString &msg) {
    fmCritical() << "Worker error:" << msg;
});
```

---

## 验证模式

### 输入验证

```cpp
bool TagDbHandler::addTagProperty(const QVariantMap &data)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    // 参数验证
    if (data.isEmpty()) {
        lastErr = "input parameter is empty!";
        fmWarning() << "TagDbHandler::addTagProperty: Empty data provided";
        finally.dismiss();
        return false;
    }

    // 依赖验证
    auto it = data.begin();
    for (; it != data.end(); ++it) {
        if (!checkTag(it.key())) {
            if (!insertTagProperty(it.key(), it.value())) {
                fmCritical() << "Failed to insert tag property for tag:" << it.key();
                return false;
            }
        }
    }

    emit newTagsAdded(data);
    return true;
}
```

**示例位置**: `src/plugins/daemon/tag/tagdbhandler.cpp`

---

## 断言使用

### Q_ASSERT

仅在调试模式下启用，用于检测不可恢复的编程错误。

```cpp
Q_ASSERT(func);                 // 断言函数指针有效
Q_ASSERT(!databaseName.isEmpty()); // 断言参数有效
```

**示例位置**:
- `src/dfm-base/base/db/sqlitehandle.h`
- `src/dfm-base/base/db/sqliteconnectionpool.cpp`

---

### Qt 断言与断言的区别

| 宏 | 调试模式 | 发布模式 | 用途 |
|----|----------|----------|------|
| `Q_ASSERT` | 启用 | 禁用 | 内部条件检查 |
| `assert` | 启用 | 禁用 | 标准 C++ 断言 |
| `Q_UNLIKELY` | 始终启用 | 始终启用 | 分支提示 |

---

## 错误日志规范

### 日志级别

```cpp
fmDebug()   << "详细调试信息";    // 开发调试
fmInfo()    << "一般信息";        // 正常操作
fmWarning() << "警告信息";        // 异常但可恢复
fmCritical() << "严重错误";       // 需要立即处理
```

### 日志内容规范

```cpp
// 不好的
fmWarning() << "Failed";

// 好的
fmWarning() << "TagDbHandler::addTagsForFiles: Empty data provided";

// 更好的
fmWarning() << "TagDbHandler::addTagsForFiles: Empty data provided, expected at least 1 file";
```

**要点**:
- 包含类名和方法名
- 包含关键参数值
- 描述期望 vs 实际

---

## 常见错误模式

### 1. 忘记清理

```cpp
// 错误
void processFile() {
    QFile file("test.txt");
    file.open(QIODevice::ReadOnly);
    // 如果抛异常，文件可能未关闭
}

// 正确 - FinallyUtil
void processFile() {
    QFile file("test.txt");
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() {
        if (file.isOpen()) file.close();
    });

    file.open(QIODevice::ReadOnly);
    // 即使异常，文件也会被关闭
}
```

---

### 2. 忘记事务

```cpp
// 错误
void transferData() {
    insert1();
    insert2();  // 如果失败，数据不一致
    insert3();
}

// 正确 - 事务
void transferData() {
    handle->transaction([&]() {
        if (!insert1()) return false;
        if (!insert2()) return false;
        if (!insert3()) return false;
        return true;
    });
}
```

---

### 3. 忽略返回值

```cpp
// 错误
file.open(QIODevice::ReadOnly);  // 忽略返回值

// 正确
if (!file.open(QIODevice::ReadOnly)) {
    fmCritical() << "Failed to open file:" << file.errorString();
    return false;
}
```

---

## 最佳实践

### Do
- 使用 FinallyUtil 确保资源清理
- 所有数据库操作使用事务
- 使用 Q_UNLIKELY 优化错误分支
- 记录详细的错误日志 (上下文、参数)
- 验证所有输入参数

### Don't
- 不要在 Qt 中滥用 C++ 异常
- 不要忽略返回值
- 不要在发布代码中保留 Q_ASSERT
- 不要使用空洞的错误消息

---

## 参考文档

- [数据库指南](database-guidelines.md) - 事务模式详情
- [日志指南](logging-guidelines.md) - 日志级别规范
- [Qt 内存管理陷阱](../big-question/qt-memory-management-pitfalls.md) - 资源生命周期
