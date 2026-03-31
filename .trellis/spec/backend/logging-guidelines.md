# 日志使用指南

> DDE File Manager 日志系统规范，包括分类日志、日志级别和日志注册。

---

## 概述

项目使用自定义分类日志系统替代 Qt 原生日志，支持按模块控制日志级别。

**特点**:
- 运行时检查日志级别，避免不必要的字符串格式化
- 按插件/模块注册独立日志类别
- 编译时可选择性禁用调试日志

---

## 日志宏

### 可用日志宏

```cpp
fmDebug()    << 调试信息
fmInfo()     << 一般信息
fmWarning()  << 警告信息
fmCritical() << 严重错误
```

**注意**: 不要使用 Qt 原生日志宏 (qDebug, qInfo 等)，应使用 `fmDebug` 等分类日志。

---

### 日志宏定义

```cpp
#undef fmDebug
#define fmDebug(...)                                                                                                           \
    for (bool qt_category_enabled = __getLogCategoryName().isDebugEnabled(); qt_category_enabled; qt_category_enabled = false) \
    QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC, __getLogCategoryName().categoryName()).debug(__VA_ARGS__)

#undef fmInfo
#define fmInfo(...)                                                                                                           \
    for (bool qt_category_enabled = __getLogCategoryName().isInfoEnabled(); qt_category_enabled; qt_category_enabled = false) \
    QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC, __getLogCategoryName().categoryName()).info(__VA_ARGS__)

#undef fmWarning
#define fmWarning(...)                                                                                                           \
    for (bool qt_category_enabled = __getLogCategoryName().isWarningEnabled(); qt_category_enabled = qt_category_enabled = false) \
    QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC, __getLogCategoryName().categoryName()).warning(__VA_ARGS__)

#undef fmCritical
#define fmCritical(...)                                                                                                           \
    for (bool qt_category_enabled = __getLogCategoryName().isCriticalEnabled(); qt_category_enabled = qt_category_enabled = false) \
    QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC, __getLogCategoryName().categoryName()).critical(__VA_ARGS__)
```

**示例位置**: `include/dfm-base/dfm_log_defines.h`

**关键点**:
- 使用 for 循环实现惰性求值
- 日志级别未启用时，完全不执行格式化代码
- 自动包含文件名、行号、函数名

---

## 日志类别注册

### 注册宏

```cpp
#define DFM_LOG_REGISTER_CATEGORY2(name)                                                    \
    const QLoggingCategory &__log##name()                                                   \
    {                                                                                       \
        static const QLoggingCategory category("org.deepin.dde.filemanager.plugin." #name); \
        return category;                                                                    \
    }

#define DFM_LOG_USE_CATEGORY2(name)                       \
    extern const QLoggingCategory &__log##name();         \
    inline const QLoggingCategory &__getLogCategoryName() \
    {                                                     \
        return __log##name();                             \
    }
```

**示例位置**: `include/dfm-base/dfm_log_defines.h`

---

### 在插件中注册

#### 步骤 1: 在 `.h` 文件中声明

```cpp
// myplugin.h
#include "dfm-base/dfm_log_defines.h"

class MyPlugin : public QObject
{
    Q_OBJECT
};
```

#### 步骤 2: 在 `.cpp` 文件中注册

```cpp
// myplugin.cpp
#include "myplugin.h"

DFM_LOG_USE_CATEGORY2(MyPlugin)
```

**生成的日志类别**: `org.deepin.dde.filemanager.plugin.MyPlugin`

---

### 命名约定

| 组件类型 | 命名格式 | 示例 |
|----------|----------|------|
| 公共插件 | `dfmplugin-*` | `dfmplugin-bookmark` |
| 桌面插件 | `ddplugin-*` | `ddplugin-core` |
| 桌面壁纸 | `plugin-name` | `background`, `wallpapersetting` |

**完整日志类别名称**:
```
org.deepin.dde.filemanager.plugin.{plugin-name}
```

---

## 日志级别

### 级别说明

| 级别 | 宏 | 用途 | 生产环境 |
|------|-----|------|----------|
| Debug | fmDebug() | 详细的程序流程信息 | 关闭 |
| Info | fmInfo() | 一般操作信息 | 可选 |
| Warning | fmWarning() | 异常但可恢复 | 开启 |
| Critical | fmCritical() | 严重错误导致功能失败 | 开启 |

---

### 运行时控制

通过环境变量或日志规则文件控制日志级别:

```bash
# 启用所有 debug 日志
QT_LOGGING_RULES="*.debug=true"

# 启用特定插件的 debug 日志
QT_LOGGING_RULES="org.deepin.dde.filemanager.plugin.*.debug=true"

# 禁用特定日志
QT_LOGGING_RULES="org.deepin.dde.filemanager.plugin.MyPlugin.debug=false"
```

---

### 编译时控制

在 CMakeLists.txt 中禁用调试宏:

```cmake
# 禁用 qDebug(), qInfo(), qWarning(), qCritical()
# 强制使用 qCDebug() 分类日志
if(OPT_DISABLE_QDEBUG)
    add_definitions(-DDFM_DISABLE_DEBUG_MACRO)
endif()
```

**示例位置**: `src/dfm-base/CMakeLists.txt`

---

## 日志内容规范

### 格式要求

```cpp
// 不好的格式
fmWarning() << "Failed to open file";

// 好的格式
fmWarning() << "TagDbHandler::addTagsForFiles: failed to open file:" << filePath;

// 更好的格式
fmWarning() << "TagDbHandler::addTagsForFiles: failed to open file:" << filePath
            << ", error:" << file.errorString();
```

---

### 日志内容要素

**必需元素**:
1. 类名和方法名
2. 操作描述
3. 关键参数值
4. 错误原因 (如果有)

**示例**:

```cpp
// 信息日志
fmInfo() << "TagDbHandler::addTagsForFiles: Adding tags for" << data.size() << "files";

// 警告日志
fmWarning() << "TagDbHandler::getTagsColor: Empty tag list provided, expected at least 1 tag";

// 错误日志
fmCritical() << "TagDbHandler::addTagProperty: Failed to insert tag property for tag:" << it.key()
             << ", error:" << db.lastError().text();
```

---

## 日志使用场景

### Debug 日志

**用途**: 开发和调试时的详细信息

```cpp
fmDebug() << "Starting file operation:" << operation << "on file:" << filePath;
fmDebug() << "Database query result count:" << results.size();
fmDebug() << "Processing item" << index << "of" << total;
```

**注意**: 生产环境默认关闭

---

### Info 日志

**用途**: 正常操作的记录信息

```cpp
fmInfo() << "TagDbHandler::addTagsForFiles: Adding tags for" << data.size() << "files";
fmInfo() << "Database initialized successfully";
fmInfo() << "Service started on port" << port;
```

**建议**: 关键操作使用 Info 日志

---

### Warning 日志

**用途**: 异常但可恢复的情况

```cpp
fmWarning() << "TagDbHandler::addTagsForFiles: Empty data provided";
fmWarning() << "Retry attempt" << retryCount << "after" << retryDelay << "ms";
fmWarning() << "Using fallback value for missing config";
```

**必须包含**: 问题描述和上下文

---

### Critical 日志

**用途**: 严重错误导致功能失败

```cpp
fmCritical() << "TagDbHandler::addTagProperty: Failed to insert tag property for tag:" << it.key();
fmCritical() << "Database connection failed:" << db.lastError().text();
fmCritical() << "Worker thread crashed, attempting restart";
```

**必须包含**:
1. 错误描述
2. 关键参数
3. 错误详情 (如果有)

---

## 性能考虑

### 避免不必要的日志

```cpp
// 假设这是热点代码，每秒调用 1000 次
for (int i = 0; i < 1000000; ++i) {
    // 不好的 - 每次都构建字符串
    fmDebug() << "Processing item" << i << "of" << totalItems;
}

// 好的 - 减少 debug 日志频率
if (i % 1000 == 0) {
    fmDebug() << "Progress:" << i << "of" << totalItems;
}
```

---

### 使用 fmt 格式化 (如果可用)

```cpp
// 如果项目使用 fmt 库
fmDebug() << fmt("Processing item {} of {}", i, totalItems);
```

---

## 禁用原生日志宏

在 `dfm-base/dfm_log_defines.h` 中:

```cpp
#if defined(DFM_DISABLE_DEBUG_MACRO)
#    undef qDebug
#    define qDebug DFM_NO_QDEBUG_MACRO
#    undef qInfo
#    define qInfo DFM_NO_QDEBUG_MACRO
#    undef qWarning
#    define qWarning DFM_NO_QDEBUG_MACRO
#    undef qCritical
#    define qCritical DFM_NO_QDEBUG_MACRO
#endif
```

**强制使用分类日志**，便于日志过滤和级别控制。

---

## 日志输出示例

### 普通使用

```cpp
// tagdbhandler.cpp
QVariantMap TagDbHandler::getAllTags()
{
    const auto &tagPropertyBean = handle->query<TagProperty>().toBeans();
    if (tagPropertyBean.isEmpty()) {
        fmDebug() << "TagDbHandler::getAllTags: No tags found in database";
        return {};
    }

    QVariantMap tagPropertyMap;
    for (auto &bean : tagPropertyBean)
        tagPropertyMap.insert(bean->getTagName(), QVariant { bean->getTagColor() });

    fmDebug() << "TagDbHandler::getAllTags: Retrieved" << tagPropertyMap.size() << "tags";
    return tagPropertyMap;
}
```

---

### 服务日志

```cpp
// commonhelper.cpp (diskencrypt 服务)
if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    qCritical() << "[common_helper::createDFMDesktopEntry] Failed to open desktop file for writing";
    return;
}

qWarning() << "[common_helper::encryptCipher] Unsupported cipher algorithm, using default:" << cipher << "-> aes";
```

---

## 最佳实践

### Do
- 使用 `fmDebug` / `fmInfo` / `fmWarning` / `fmCritical`
- 在插件中注册日志类别
- 日志包含类名、方法名和上下文
- 使用运行时环境变量控制日志级别
- 热点代码减少日志频率

### Don't
- 不要使用 Qt 原生日志宏 (qDebug, qInfo 等)
- 不要在日志中进行耗时操作
- 不要忽略 Warning 和 Critical 日志
- 不要在生产环境启用大量 Debug 日志

---

## 参考文档

- [错误处理指南](error-handling.md) - 错误日志用法
- [测试标准](test-standards.md) - 测试中的日志规范
