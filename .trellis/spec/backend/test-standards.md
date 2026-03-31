# 测试规范

> DDE 应用的单元测试框架规范 (基于 Google Test + cpp-stub)。

---

## 概览

本指南涵盖使用 Google Test (gtest) 框架编写 DDE 应用测试的模式和最佳实践。项目使用 gtest 作为主要测试框架，配合 cpp-stub 进行函数打桩。

**测试规则文档**: `autotests/UT_RULES.md`

---

## Google Test 框架

### 基本测试结构

```cpp
#include <gtest/gtest.h>
#include <QCoreApplication>
#include "stubext.h"

class SettingsTest : public ::testing::Test
{
protected:
    void SetUp() override {
        stub.clear();
        // 初始化测试环境
        if (!qApp) {
            static int argc = 1;
            static char *argv[] = { const_cast<char*>("test_settings") };
            app = std::make_unique<QCoreApplication>(argc, argv);
        }
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

### 常见断言

**Google Test 断言** (优先使用 `EXPECT_*` 避免 `ASSERT_*`):

```cpp
// 布尔检查
EXPECT_TRUE(condition);               // 期望条件为真
EXPECT_FALSE(condition);              // 期望条件为假

// 相等性检查
EXPECT_EQ(actual, expected);          // 期望 a == b
EXPECT_NE(actual, expected);          // 期望 a != b
EXPECT_LT(actual, expected);          // 期望 a < b
EXPECT_LE(actual, expected);          // 期望 a <= b
EXPECT_GT(actual, expected);          // 期望 a > b
EXPECT_GE(actual, expected);          // 期望 a >= b

// 浮点数比较 (使用近似比较)
EXPECT_FLOAT_EQ(expected, actual);
EXPECT_DOUBLE_EQ(expected, actual);
EXPECT_NEAR(val1, val2, abs_error);

// 字符串比较
EXPECT_STREQ(str1, str2);             // C 字符串相等
EXPECT_STRNE(str1, str2);             // C 字符串不等
EXPECT_EQ(qstr1, qstr2);              // QString 相等

// 指针检查
EXPECT_EQ(ptr, nullptr);
EXPECT_NE(ptr, nullptr);
}
```

**注意**:
- `EXPECT_*` 失败后继续执行后续测试
- `ASSERT_*` 失败后立即终止当前测试函数
- 优先使用 `EXPECT_*` 避免内存泄漏风险

---

### 数据驱动测试

```cpp
// 使用参数化测试
class ParametrizedTest : public ::testing::TestWithParam<std::tuple<QString, bool, QString>> {
};

INSTANTIATE_TEST_SUITE_P(
    ParameterizedTests,
    ParametrizedTest,
    ::testing::Values(
        std::make_tuple("test@example.com", true, ""),
        std::make_tuple("invalid", false, "Invalid format"),
        std::make_tuple("", false, "Required field")
    )
);

TEST_P(ParametrizedTest, Validation_Tests) {
    QString input = std::get<0>(GetParam());
    bool expectedValid = std::get<1>(GetParam());
    QString expectedError = std::get<2>(GetParam());

    bool isValid = m_widget->validate(input);
    EXPECT_EQ(isValid, expectedValid);

    if (!expectedValid) {
        EXPECT_EQ(m_widget->errorString(), expectedError);
    }
}
```

---

### 信号测试

```cpp
#include <QSignalSpy>

TEST_F(MyWidgetTest, Signal_Tests) {
    QSignalSpy spy(m_widget, &MyWidget::valueChanged);

    // 触发信号
    m_widget->setValue(42);

    // 验证信号已发出
    EXPECT_EQ(spy.count(), 1);

    // 验证信号参数
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.at(0).toInt(), 42);
}

TEST_F(MyWidgetTest, Multiple_Signals) {
    QSignalSpy valueSpy(m_widget, &MyWidget::valueChanged);
    QSignalSpy errorSpy(m_widget, &MyWidget::errorOccurred);

    m_widget->setValue(100);

    EXPECT_EQ(valueSpy.count(), 1);
    EXPECT_EQ(errorSpy.count(), 0);

    m_widget->setInvalidValue();

    EXPECT_EQ(valueSpy.count(), 1);  // 没有新信号
    EXPECT_EQ(errorSpy.count(), 1);
}
```

---

### 打桩测试 (cpp-stub)

```cpp
#include "stubext.h"

TEST_F(FileOperationTest, With_Stub) {
    stub_ext::StubExt stub;

    // 桩成员函数 (使用 ADDR)
    stub.set_lamda(ADDR(DeviceProxyManager, queryBlockInfo), [] {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map[DeviceProperty::kOpticalBlank] = true;
        return map;
    });

    // 桩虚函数 (使用 VADDR)
    stub.set_lamda(VADDR(DDialog, exec), [&trigger] {
        __DBG_STUB_INVOKE__
        trigger = true;
        return 0;
    });

    // 桩普通函数
    stub.set_lamda(&DConfigManager::value, [&] {
        __DBG_STUB_INVOKE__
        return ret;
    });

    // 执行测试...
}
```

**完整的打桩教程**: 参考 `autotests/UT_RULES.md`

---

### 测试异步操作

```cpp
TEST_F(AsyncOperationTest, Async_Operation) {
    QEventLoop loop;
    QTimer::singleShot(100, &loop, &QEventLoop::quit);

    connect(m_worker, &Worker::finished, &loop, &QEventLoop::quit);

    m_worker->startAsync();
    loop.exec();  // 等待完成

    EXPECT_TRUE(m_worker->isComplete());
    EXPECT_EQ(m_worker->result(), expectedResult);
}
```

---

## 测试组织

### 目录结构

```
autotests/
├── CMakeLists.txt          # 测试构建配置
├── libs/                   # 库测试 (dfm-base, dfm-framework, dfm-extension)
│   ├── dfm-base/
│   │   └── base/
│   │       └── test_settings.cpp
│   └── dfm-framework/
│       └── test_pluginmanager.cpp
├── plugins/                # 插件测试
├── services/               # 服务测试
├── tools/                  # 工具测试
└── 3rdparty/               # 第三方库 (cpp-stub, gmock等)
```

**测试文件命名**: `test_<classname>.cpp`

**示例位置**:
- `autotests/libs/dfm-base/base/test_settings.cpp`

---

### 测试命名规范

| 测试类型 | 命名格式 | 示例 |
|----------|----------|------|
| 测试类 | `{ClassName}Test` | `SettingsTest` |
| 测试方法 | `[MethodUnderTest]_[Scenario]_[ExpectedResult]` | `Value_BasicOperations` |

---

## AIR 原则 (必须遵守)

- **Automatic（自动化）**: 测试必须全自动执行，无需人工干预
- **Independent（独立性）**: 测试用例之间不能有依赖关系或执行顺序要求
- **Repeatable（可重复）**: 测试结果不受外部环境影响，可重复执行

---

## 打桩策略

### 工具选择
- **cpp-stub**: 函数级别打桩，适用于普通函数和成员函数
- **gmock**: 接口级别 mock，适用于虚函数接口
- **QTest**: Qt 特有功能测试（信号槽、GUI 交互）

### 打桩原则
- 对所有外部依赖进行打桩（网络、界面 show/exec、DBus、QProcess等）
- 对于简单的 IO 操作可以使用临时文件模拟
- 在 `SetUp()` 中初始化打桩，在 `TearDown()` 中清理资源
- 如果需要对第三方接口打桩，需要参考源码引入对应的头文件和命名空间
- **不要对测试的接口本身打桩！**

**完整的打桩教程**: 参考项目根目录 `autotests/UT_RULES.md`

---

## 覆盖率分析

### 覆盖率要求
- 使用 lcov 工具生成代码覆盖率报告
- 目标覆盖率: **>80%**
- 关注场景覆盖而非单纯的代码行覆盖率
- 排除测试代码和第三方库的覆盖率统计

### 性能考虑
- 测试执行速度要快，避免长时间运行的操作
- 不要编写性能测试用例
- 避免测试中有 sleep 或延时操作
- 及时清理测试过程中创建的资源，防止内存泄漏

### 环境隔离
- 测试不能修改系统环境或删除系统资源
- 测试创建的临时文件和数据必须在测试结束后清理
- 避免测试对生产环境造成任何影响

---

## Qt 特定测试指导

### 信号槽测试
- 使用 QSignalSpy 监控信号发射
- 通过打桩验证信号槽的调用关系
- 测试信号参数的正确性
- 原则上测试代码不用 QObject::connect，应该使用 QSignalSpy

---

## 常见问题避免

### 测试稳定性
- 不要在测试中使用随机数或当前时间
- 避免依赖网络连接或外部服务
- 不要使用 sleep 或延时操作
- 有重大逻辑缺陷的代码可以跳过单元测试

### 测试独立性
- 每个测试用例都应该能够单独运行
- 测试用例之间不能共享状态或数据
- 避免使用全局变量或静态变量传递测试数据

### 资源管理
- 在 SetUp() 中创建的资源必须在 TearDown() 中释放
- 使用 RAII 原则管理动态分配的内存
- 确保异常情况下资源也能正确释放

---

## 最佳实践

1. **每个类一个测试文件**
2. **测试用例命名**: `[MethodUnderTest]_[Scenario]_[ExpectedResult]`
3. **使用 `EXPECT_*` 系列断言**，避免 `ASSERT_*` 防止内存泄漏
4. **测试边界情况**: null、空、最大值
5. **保持测试独立**: 没有共享状态
6. **Mock 外部依赖**: 使用 cpp-stub 打桩
7. **目标覆盖率 >80%**: 尽可能覆盖业务代码
8. **优先使用 `TEST_F` 宏**: 通过 `SetUp()` 和 `TearDown()` 管理资源
9. **重载函数必须按要求打桩**: 这是常见的错误来源

---

## 测试用例模板

```cpp
TEST_F(MyClassTest, MethodName_Scenario_ExpectedResult) {
    // given - 准备测试数据
    QString input = "test input";
    QString expected = "expected output";

    // when - 执行被测试的方法
    QString actual = m_object->methodName(input);

    // then - 验证结果
    EXPECT_EQ(actual, expected);
}
```

---

## 参考文档

- [单元测试规则](../../autotests/UT_RULES.md) - 完整的测试规则和打桩教程
- [代码质量指南](quality-guidelines.md) - 测试标准和覆盖率要求
