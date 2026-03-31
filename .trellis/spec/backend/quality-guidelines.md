# 后端代码质量指南

> DDE File Manager 后端代码质量标准，包括测试、代码审查、命名规范等。

---

## 概述

项目使用多种工具和规范确保代码质量，包括单元测试、静态分析和规范约定。

---

## 单元测试

### 测试框架

使用 **Google Test (gtest)** 框架进行单元测试，配合 `stubext` 进行函数桩。

---

### 测试文件结构

```cpp
test_settings.cpp
```

```cpp
#include <gtest/gtest.h>
#include "stubext.h"

class SettingsTest : public ::testing::Test
{
protected:
    void SetUp() override {
        stub.clear();
        // 初始化测试环境
    }

    void TearDown() override {
        stub.clear();
        // 清理测试环境
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QCoreApplication> app;
};

TEST_F(SettingsTest, Creation_Basic) {
    Settings *settings = createTestSettings();
    EXPECT_NE(settings, nullptr);
    delete settings;
}

TEST_F(SettingsTest, Value_BasicOperations) {
    Settings *settings = createTestSettings();
    QVariant value1 = settings->value("TestGroup", "key1");
    EXPECT_EQ(value1.toString(), "settings_value1");
    delete settings;
}
```

**示例位置**: `autotests/libs/dfm-base/base/test_settings.cpp`

---

### CMake 测试配置

```cmake
add_subdirectory(libs)
add_subdirectory(plugins)
add_subdirectory(services)
add_subdirectory(tools)
```

**示例位置**: `autotests/CMakeLists.txt`

---

### 测试命名规范

| 测试类型 | 命名格式 | 示例 |
|----------|----------|------|
| 测试类 | `{ClassName}Test` | `SettingsTest` |
| 测试方法 | `{TestMethod}_{TestCase}` | `Value_BasicOperations` |

---

### Stubext 使用

```cpp
stub_ext::StubExt stub;

// 桩函数
stubs.set(&QFile::open, [](QIODevice::OpenMode) { return true; });

// 恢复原始函数
stub.clear();
```

**用途**: 模拟外部依赖，隔离测试单元。

---

## 静态分析工具

### CPPLINT

**Google C++ 风格检查工具**。

**使用** (如果配置):
```bash
cpplint --filter=-build/include_what_you_use src/**/*.cpp
```

**常见检查项**:
- 命名规范
- 指针/引用使用
- 头文件包含顺序

---

### Clang-Tidy

**代码静态分析工具**。

**使用** (如果配置):
```bash
clang-tidy src/*.cpp -- -Iinclude/ -std=c++17
```

**常见检查项**:
- 内存泄漏
- 未初始化变量
- 悬空指针

---

## 命名空间管理

### 命名空间宏

```cpp
#define DFMBASE_NAMESPACE dfmbase

#define DFMBASE_BEGIN_NAMESPACE namespace DFMBASE_NAMESPACE {
#define DFMBASE_END_NAMESPACE }
#define DFMBASE_USE_NAMESPACE using namespace DFMBASE_NAMESPACE;
```

**示例位置**: `include/dfm-base/dfm_base_global.h`

---

### 使用方式

```cpp
DFMBASE_BEGIN_NAMESPACE

class Settings : public QObject
{
    Q_OBJECT
};

DFMBASE_END_NAMESPACE
```

---

### 顶层文件命名空间

| 库 | 命名空间 | 宏 |
|----|----------|-----|
| dfm-base | `dfmbase` | `DFMBASE_NAMESPACE` |
| dfm-framework | `dfmframework` | `DFMFRAMEWORK_NAMESPACE` |
| dfm-extension | `dfmextension` | `DFMEXTENSION_NAMESPACE` |

---

## 编译选项控制

### 禁用调试宏

```cmake
# 禁用 qDebug(), qInfo(), qWarning(), qCritical()
# 强制使用 qCDebug() 分类日志
if(OPT_DISABLE_QDEBUG)
    add_definitions(-DDFM_DISABLE_DEBUG_MACRO)
endif()
```

---

### Qt 信号槽关键字禁用

```cmake
# signals and slots keyword is disable, use Q_SIGNALS and Q_SLOTS instead
add_compile_definitions(QT_NO_SIGNALS_SLOTS_KEYWORDS)
```

**原因**: 与 Qt3/MOC 兼容性，`signals`/`slots` 可能与其他系统宏冲突。

**正确用法**:
```cpp
// 使用 Qt 关键字宏
class MyClass : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    void slot();
Q_SIGNALS:
    void signal();
};
```

---

### 自定义编译定义

```cmake
add_compile_definitions(THUMBNAIL_TOOL_DIR="${DFM_THUMBNAIL_TOOL}")
```

**用途**: 在编译时注入配置值，如工具路径、版本号等。

---

## 代码风格规范

### 文件命名

| 类型 | 约定 | 示例 |
|------|------|------|
| 头文件 | 小写+下划线 | `sqliteconnectionpool.h` |
| 源文件 | 小写+下划线 | `sqliteconnectionpool.cpp` |
| 私有头文件 | 小写+下划线+_p | `fileselectionmodel_p.h` |

---

### 类命名

| 类型 | 约定 | 示例 |
|------|------|------|
| 公共类 | PascalCase | `TagDbHandler` |
| 私有类 | PascalCase | `SettingsPrivate` (无 m_ 前缀) |
| 结构体 | PascalCase | `TagProperty` |

---

### D-Pointer 命名

**Private 类无前缀，普通成员变量用 m_ 前缀**:

```cpp
// 公共类头文件
class SettingsPrivate;

class Settings : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Settings)
};

// Private 头文件
class SettingsPrivate
{
    Q_DECLARE_PUBLIC(Settings)
public:
    // 无 m_ 前缀
    QString settingFile;
    QVariantHash values;
};
```

**详细说明**: 参考 `shared/cpp-conventions.md`

---

### 变量命名

| 类型 | 约定 | 示例 |
|------|------|------|
| 局部变量 | 小驼峰 | `fileName`, `isReady` |
| 成员变量 | m_ + 小驼峰 | `m_data`, `m_isValid` |
| 成员变量 (Private 类) | m_ + 小驼峰 | `m_values`, `m_privateValues` |
| 常量 | 小驼峰 | `kDefaultTimeout` |
| 宏定义 | 全大写+下划线 | `MAX_RETRY_COUNT` |

---

### 函数命名

| 类型 | 约定 | 示例 |
|------|------|------|
| 成员函数 | 小驼峰 | `getValue()`, `setData()` |
| 虚函数 | 小驼峰 | `initialize()` |
| 信号 | 小驼峰 (Q_SIGNALS) | `valueChanged()` |
| 槽 | 小驼峰 (Q_SLOTS) | `onValueChanged()` |

---

### Getter/Setter 命名

```cpp
// Getter: get + 字段名
QString getFileName() const;

// Setter: set + 字段名
void setFileName(const QString &name);

// Boolean getter: is + 形容词
bool isValid() const;
bool hasData() const;
```

---

## 代码组织

### 头文件结构

```cpp
// 1. 版权声明 (可选)
// 2. 头文件保护
#ifndef DFMBASE_SETTINGS_H
#define DFMBASE_SETTINGS_H

// 3. 包含顺序: Qt 头 → 项目头
#include <QObject>
#include <QString>
#include <QVariant>

#include "dfm-base/dfm_base_global.h"

// 4. 前置声明
class SettingsPrivate;
class QFile;

// 5. 类型别名
using DataMap = QMap<QString, QVariant>;

// 6. 类声明
DFMBASE_BEGIN_NAMESPACE

class Settings : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Settings)
};

DFMBASE_END_NAMESPACE

#endif // DFMBASE_SETTINGS_H
```

---

### 源文件结构

```cpp
// 1. 包含对应的头文件
#include "settings.h"

// 2. 包含私有头文件
#include "settings_p.h"

// 3. 其他头文件
#include <QSaveFile>

// 4. 命名空间
DFMBASE_BEGIN_NAMESPACE

// 5. 实现
Settings::Settings(QObject *parent)
    : QObject(parent)
    , d_ptr(new SettingsPrivate(this))
{
}

Settings::~Settings()
{
}

// 6. 结束命名空间
DFMBASE_END_NAMESPACE
```

---

## 代码审查标准

### 审查清单

**功能性**:
- [ ] 代码是否实现预期功能
- [ ] 边界条件是否处理
- [ ] 错误处理是否完善

**安全性**:
- [ ] 输入参数是否验证
- [ ] 资源是否正确释放
- [ ] 是否有潜在的内存泄漏

**性能**:
- [ ] 是否有不必要的拷贝
- [ ] 是否在热点路径进行耗时操作
- [ ] 是否使用 Q_UNLIKELY 优化分支

**可读性**:
- [ ] 命名是否清晰
- [ ] 代码是否简洁
- [ ] 注释是否合理

---

### 常见问题

| 问题 | 检测方法 | 修复建议 |
|------|----------|----------|
| 内存泄漏 | `grep "new"` 检查父对象 | 设置父对象或使用智能指针 |
| 硬编码颜色 | `grep -E "#[0-9a-fA-F]{6}"` | 使用 DTK 主题颜色 |
| 原生 Qt 组件 | `grep "QMainWindow"` | 替换为 DTK 组件 |
| 异常处理 | 检查返回值处理 | 添加错误处理逻辑 |

---

## 覆盖率要求

**目标覆盖率**: >80%

**优先测试**:
- 公共 API 函数
- 核心业务逻辑
- 复杂算法
- 边界条件

---

## 最佳实践

### Do
- 使用 Google Test 编写单元测试
- 遵循命名空间规范
- 使用 Q_SIGNALS/Q_SLOTS 宏
- 正确使用 D-Pointer 模式
- 覆盖边界条件和错误场景

### Don't
- 不要跳过代码审查
- 不要提交未通过静态分析的代码
- 不要使用 Qt 原生 signals/slots 关键字 (使用 Q_SIGNALS/Q_SLOTS)
- 不要硬编码颜色和路径
- 不要忽略警告

---

## 参考文档

- [C++ 编码规范](../shared/cpp-conventions.md) - C++ 详细规范
- [测试标准](test-standards.md) - 测试详细标准
- [代码审查标准](../review/code-review-standards.md) - 审查流程
