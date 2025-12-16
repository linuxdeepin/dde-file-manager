# DDE File Manager 单元测试框架学习指南

## 1. 测试框架概述

### 1.1 测试框架架构
- **测试框架**: Google Test (GTest)
- **构建系统**: CMake + CTest
- **Qt 应用支持**: 自动选择 QApplication/QCoreApplication
- **代码覆盖率**: lcov + genhtml
- **Mock/Stub 工具**: stub_ext (自定义 stub 扩展)

### 1.2 项目结构
```
autotests/
├── CMakeLists.txt           # 主构建配置
├── dfm_test_main.h          # 测试主入口宏定义
├── dfm_asan_helper.h        # ASAN 报告辅助
├── run-ut.sh                # 一键测试脚本
├── generate-report.py       # 测试报告生成器
├── libs/                    # 库单元测试
│   ├── dfm-base/           # dfm-base 库测试
│   ├── dfm-extension/       # dfm-extension 库测试
│   └── dfm-framework/       # dfm-framework 库测试
├── plugins/                 # 插件单元测试
├── services/                # 服务单元测试
└── tools/                   # 工具单元测试
```

## 2. 测试框架核心组件

### 2.1 测试主入口 (`dfm_test_main.h`)

统一的测试主入口宏，自动处理：
- Qt 应用类型选择（QApplication/QCoreApplication）
- Google Test 初始化
- ASAN 报告配置

**使用方法**:
```cpp
#include "dfm_test_main.h"

DFM_TEST_MAIN(my_unit_test)
```

**宏定义详解**:
- 自动根据 `QT_WIDGETS_LIB` 选择应用类型
- 初始化 Google Test 框架
- 配置 ASAN 报告路径

### 2.2 CMake 测试工具 (`DFMTestUtils.cmake`)

提供了完整的测试构建支持：

**核心函数**:
- `dfm_init_test_environment()` - 初始化测试环境
- `dfm_create_test_executable()` - 创建测试可执行文件
- `dfm_create_library_test()` - 创建库测试
- `dfm_create_plugin_test_enhanced()` - 创建插件测试（增强版）
- `dfm_create_service_test_enhanced()` - 创建服务测试（增强版）

**测试环境配置**:
- 自动查找 GTest 和 Qt6 Test
- 配置测试 stub 工具
- 设置代码覆盖率编译选项
- 配置 ASAN 报告目录

## 3. 如何编写单元测试

### 3.1 基本测试类结构

```cpp
#include <gtest/gtest.h>
#include "dfm_test_main.h"

// 包含测试目标类
#include <dfm-base/base/application/application.h>

// 包含 stub 工具
#include "stubext.h"

class ApplicationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 设置测试数据
    }

    void TearDown() override
    {
        // 清理测试环境
        stub.clear();
    }

    // Stub 工具实例
    stub_ext::StubExt stub;
};

// 测试用例
TEST_F(ApplicationTest, SingletonCreation_Basic)
{
    // 测试代码
    EXPECT_EQ(Application::instance(), nullptr);
}
```

### 3.2 测试文件结构

**标准测试文件组成**:
1. **Header 注释**: SPDX 版权和许可证信息
2. **Includes**: 测试框架、目标类、stub 工具
3. **Test Fixture**: 继承 `::testing::Test` 的测试类
4. **测试用例**: 使用 `TEST_F` 或 `TEST` 宏定义
5. **Main 函数**: 使用 `DFM_TEST_MAIN` 宏

**示例** (`test_application.cpp`):
```cpp
// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later


#include <dfm-base/base/application/application.h>

#include "stubext.h"
#include <gtest/gtest.h>

using namespace dfmbase;

class ApplicationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 测试前置设置
    }
    
    void TearDown() override {
        stub.clear();
        // 测试后置清理
    }
    
    stub_ext::StubExt stub;
};

TEST_F(ApplicationTest, SingletonCreation_Basic) {
    EXPECT_EQ(Application::instance(), nullptr);
    // 更多测试逻辑...
}
```

### 3.3 使用 Stub 工具进行 Mock

**stub_ext 使用方式**:

```cpp
#include "stubext.h"

class MyTest : public ::testing::Test {
protected:
    stub_ext::StubExt stub;
};

TEST_F(MyTest, StubExample) {
    // 1. Stub 成员函数
    bool setValueCalled = false;
    stub.set_lamda(static_cast<void (Settings::*)(const QString&, const QString&, const QVariant&)>(&Settings::setValue),
                   [&setValueCalled](Settings *self, const QString &group, const QString &key, const QVariant &value) {
                       setValueCalled = true;
                       EXPECT_EQ(group, "TestGroup");
                   });
    
    // 2. 调用被测试代码
    Settings settings;
    settings.setValue("TestGroup", "TestKey", QVariant(123));
    
    // 3. 验证 stub 被调用
    EXPECT_TRUE(setValueCalled);
}
```

**Stub 常用场景**:
- Mock 外部依赖（文件系统、数据库、网络）
- Mock Qt 信号/槽
- Mock 静态函数调用
- 控制函数返回值

#### 3.3.1 UI 显示和对话框的插桩方式

在 GUI 单元测试中，需要避免实际的 UI 显示和用户交互，使用以下标准模式：

**1. 对话框执行插桩**:
```cpp
// Mock QDialog::exec to avoid actual dialog display
stub.set_lamda(VADDR(QDialog, exec), [] {
    __DBG_STUB_INVOKE__
    return QDialog::Accepted;  // 或 QDialog::Rejected
});
```

**2. UI 显示/隐藏插桩**:
```cpp
// Mock the UI show
stub.set_lamda(&QWidget::show, [](QWidget *) {
    __DBG_STUB_INVOKE__
});

stub.set_lamda(&QWidget::hide, [](QWidget *) {
    __DBG_STUB_INVOKE__
});
```

**3. 菜单执行插桩**:
```cpp
// Mock menu execution with specific overload
bool isCall = false;
stub.set_lamda((QAction * (QMenu::*)(const QPoint &, QAction *)) ADDR(QMenu, exec), [&]() {
    isCall = true;
    return nullptr;
});
```

**4. 完整 UI 测试示例**:
```cpp
class UT_DialogTest : public testing::Test {
protected:
    void SetUp() override {
        stub.clear();
        dialog = new TestDialog();
        
        // Standard UI stub setup
        stub.set_lamda(VADDR(QDialog, exec), [] {
            __DBG_STUB_INVOKE__
            return QDialog::Accepted;
        });
        stub.set_lamda(&QWidget::show, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&QWidget::hide, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
    }
    
    void TearDown() override {
        delete dialog;
        stub.clear();
    }
    
    stub_ext::StubExt stub;
    TestDialog *dialog = nullptr;
};
```

#### 3.3.2 事件发送的插桩方式

对于事件发布和信号分发，使用以下标准模式：

**1. EventDispatcherManager 事件发布插桩**:
```cpp
// Mock dpfSignalDispatcher->publish
bool eventPublished = false;
QUrl capturedUrl;

// Use a specific overload of publish method
using PublishFunc = bool (EventDispatcherManager::*)(const QString&, const QString&, const QUrl&);
auto publish = static_cast<PublishFunc>(&EventDispatcherManager::publish);
stub.set_lamda(publish, [&](EventDispatcherManager *, const QString &, const QString &, const QUrl &url) -> bool {
    __DBG_STUB_INVOKE__
    eventPublished = true;
    capturedUrl = url;
    EXPECT_EQ(url, testUrl);
    return true;
});
```

**2. EventChannelManager 插槽推送插桩**:
```cpp
// Mock dpfSlotChannel->push
bool slotPushed = false;
typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, QList<QUrl>, QVariantHash &&);
auto push = static_cast<Push>(&EventChannelManager::push);
stub.set_lamda(push, [&](EventChannelManager *, const QString &space, const QString &topic, 
                         const QList<QUrl> &urls, const QVariantHash &properties) -> bool {
    __DBG_STUB_INVOKE__
    slotPushed = true;
    EXPECT_EQ(space, QString("dfmplugin_propertydialog"));
    EXPECT_EQ(topic, QString("slot_PropertyDialog_Show"));
    EXPECT_EQ(urls.size(), 2);
    return true;
});
```

**3. 函数指针地址插桩**:
```cpp
// Mock using function pointer address
stub.set_lamda(ADDR(FileManagerWindowsManager, findWindowId), 
    [expectedId](FileManagerWindowsManager *self, const QWidget *sender) -> quint64 {
        Q_UNUSED(self)
        Q_UNUSED(sender)
        return expectedId;
    });
```

**4. 虚函数地址插桩**:
```cpp
// Mock virtual functions using VADDR
stub.set_lamda(VADDR(DDialog, showEvent), [&parentShowEventCalled] {
    __DBG_STUB_INVOKE__
    parentShowEventCalled = true;
});
```

#### 3.3.3 常用插桩模式总结

**验证函数调用**:
```cpp
bool called = false;
stub.set_lamda(&SomeClass::method, [&called](...) { 
    called = true; 
});
EXPECT_TRUE(called);
```

**控制返回值**:
```cpp
stub.set_lamda(&SomeClass::getValue, []() { return 42; });
EXPECT_EQ(obj.getValue(), 42);
```

**验证参数**:
```cpp
QString capturedParam;
stub.set_lamda(&SomeClass::setValue, [&capturedParam](const QString &s) { 
    capturedParam = s; 
});
EXPECT_EQ(capturedParam, "expected");
```

**处理重载函数**:
```cpp
// 使用 static_cast 指定具体的重载版本
stub.set_lamda(static_cast<void (*)(quint64, const QUrl &)>(ComputerEventCaller::cdTo), 
               [&](quint64 winId, const QUrl &url) {
    // 具体重载的处理逻辑
});
```

### 3.4 Qt 信号测试

```cpp
#include <QSignalSpy>

TEST_F(ApplicationTest, SignalEmission_AppAttributes)
{
    Application *app = createTestApplication();
    
    // 创建信号监听器
    QSignalSpy appAttributeChangedSpy(app, &Application::appAttributeChanged);
    QSignalSpy iconSizeLevelChangedSpy(app, &Application::iconSizeLevelChanged);
    
    // 触发信号
    app->d->_q_onSettingsValueChanged("ApplicationAttribute", "IconSizeLevel", 2);
    
    // 验证信号发射
    EXPECT_EQ(appAttributeChangedSpy.count(), 1);
    EXPECT_EQ(iconSizeLevelChangedSpy.count(), 1);
    EXPECT_EQ(iconSizeLevelChangedSpy.at(0).at(0).toInt(), 2);
    
    delete app;
}
```

### 3.5 Widget 和 UI 组件测试

**Widget 尺寸和布局测试**:
```cpp
TEST_F(DevicePropertyDialogTest, ContentHeight_WithExtendedControls_CalculatesCorrectHeight)
{
    // Mock widget height methods
    const int mockIconHeight = 128;
    const int mockBasicInfoHeight = 50;
    
    stub.set_lamda(&QWidget::height, [=](QWidget *widget) -> int {
        __DBG_STUB_INVOKE__
        if (widget == dialog->deviceIcon)
            return mockIconHeight;
        else if (widget == dialog->basicInfo)
            return mockBasicInfoHeight;
        return 10;   // Default height for other widgets
    });
    
    stub.set_lamda(&QWidget::contentsMargins, [](QWidget *) -> QMargins {
        __DBG_STUB_INVOKE__
        return QMargins(10, 10, 10, 10);
    });
    
    // Test contentHeight calculation
    int actualHeight = dialog->contentHeight();
    EXPECT_GT(actualHeight, 0);
}
```

**事件处理测试**:
```cpp
TEST_F(WidgetTest, KeyPressEvent_EscapeKey_ClosesDialog)
{
    bool closeCalled = false;
    
    // Mock close method
    stub.set_lamda(&QWidget::close, [&closeCalled](QWidget *) -> bool {
        __DBG_STUB_INVOKE__
        closeCalled = true;
        return true;
    });
    
    // Create escape key event
    QKeyEvent escapeEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    
    // Test keyPressEvent with Escape key
    dialog->keyPressEvent(&escapeEvent);
    
    // Verify close was called
    EXPECT_TRUE(closeCalled);
}
```

**事件过滤器测试**:
```cpp
TEST_F(WidgetTest, EventFilter_PaintEvent_DrawsRoundedBackground)
{
    bool paintingOccurred = false;
    QColor capturedColor;
    
    stub.set_lamda(&QPainter::setRenderHint, [](QPainter *, QPainter::RenderHint, bool) {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(&QPainter::fillPath, [&](QPainter *, const QPainterPath &, const QBrush &brush) {
        __DBG_STUB_INVOKE__
        paintingOccurred = true;
        capturedColor = brush.color();
    });
    
    // Create paint event
    QPaintEvent paintEvent(QRect(0, 0, 200, 100));
    
    // Test eventFilter with paint event
    bool result = background->eventFilter(parentWidget, &paintEvent);
    
    // Verify painting occurred
    EXPECT_TRUE(result);
    EXPECT_TRUE(paintingOccurred);
}
```

### 3.6 性能测试示例

```cpp
TEST_F(ApplicationTest, Performance_BasicOperations)
{
    Application *app = createTestApplication();
    
    // Mock 操作以快速返回
    stub.set_lamda(/* ... */);
    
    // 性能测试
    const int iterations = 1000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        Application::appAttribute(Application::kIconSizeLevel);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // 验证性能要求
    EXPECT_LT(duration.count(), 1000);
    
    delete app;
}
```

### 3.7 错误处理和边界条件测试

**空指针和无效参数测试**:
```cpp
TEST_F(TestClass, ErrorHandling_InvalidParameters_HandlesGracefully)
{
    // Test with various invalid parameters
    QUrl invalidUrl;
    QString emptyPath = "";
    quint64 zeroWinId = 0;
    QList<QUrl> emptyUrls;
    
    EXPECT_NO_THROW(ComputerEventCaller::cdTo(nullptr, invalidUrl));
    EXPECT_NO_THROW(ComputerEventCaller::cdTo(mockWidget, emptyPath));
    EXPECT_NO_THROW(ComputerEventCaller::cdTo(zeroWinId, invalidUrl));
    EXPECT_NO_THROW(ComputerEventCaller::sendShowPropertyDialog(emptyUrls));
}
```

**特殊字符处理测试**:
```cpp
TEST_F(TestClass, EventParameters_SpecialCharacters_HandlesCorrectly)
{
    quint64 testWinId = 12345;
    QUrl testUrl("entry://test-with-special-chars_&%$.blockdev");
    QString newName = "New Name with 特殊字符 & % $";
    
    // Mock event publishing
    bool eventPublished = false;
    using PublishFunc = bool (EventDispatcherManager::*)(const QString&, const QString&, const QUrl&);
    auto publish = static_cast<PublishFunc>(&EventDispatcherManager::publish);
    stub.set_lamda(publish, [&](EventDispatcherManager *, const QString &, const QString &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        eventPublished = true;
        return true;
    });
    
    // Test with special characters
    EXPECT_NO_THROW(ComputerEventCaller::sendOpenItem(testWinId, testUrl));
    EXPECT_NO_THROW(ComputerEventCaller::sendItemRenamed(testUrl, newName));
    
    EXPECT_TRUE(eventPublished);
}
```

### 3.8 复杂场景组合测试

**多个事件调用测试**:
```cpp
TEST_F(TestClass, MultipleEventCalls_DifferentParameters_HandlesCorrectly)
{
    quint64 testWinId = 12345;
    QUrl testUrl1("entry://test1.blockdev");
    QUrl testUrl2("entry://test2.blockdev");
    QString newName = "Renamed Device";
    
    // Mock all event publishing
    int publishCallCount = 0;
    using PublishFunc = bool (EventDispatcherManager::*)(const QString&, const QString&, const QUrl&);
    auto publish = static_cast<PublishFunc>(&EventDispatcherManager::publish);
    stub.set_lamda(publish, [&](EventDispatcherManager *, const QString &, const QString &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        publishCallCount++;
        return true;
    });
    
    // Call multiple events
    ComputerEventCaller::sendOpenItem(testWinId, testUrl1);
    ComputerEventCaller::sendCtrlNOnItem(testWinId, testUrl2);
    ComputerEventCaller::sendItemRenamed(testUrl1, newName);
    
    EXPECT_EQ(publishCallCount, 4);  // Verify all events were published
}
```

## 4. CMake 配置

### 4.1 库测试配置 (`libs/dfm-base/CMakeLists.txt`)

```cmake
cmake_minimum_required(VERSION 3.10)

# 使用 DFM 测试工具创建库测试
dfm_create_library_test("dfm-base")
```

### 4.2 插件测试配置

```cmake
cmake_minimum_required(VERSION 3.10)

# 查找测试文件（自动）
# 查找插件源文件（自动）
# 创建测试可执行文件
dfm_create_plugin_test_enhanced("my-plugin" "${CMAKE_CURRENT_SOURCE_DIR}/../../src/plugins/my-plugin")
```

### 4.3 服务测试配置

```cmake
cmake_minimum_required(VERSION 3.10)

# 创建服务测试（自动排除 main.cpp）
dfm_create_service_test_enhanced("my-service" "${CMAKE_CURRENT_SOURCE_DIR}/../../src/services/my-service")
```

### 4.4 编译选项

**测试编译标志**:
- `-fno-inline` - 禁用内联，提高覆盖率准确性
- `-fno-access-control` - 禁用访问控制，便于测试私有成员
- `-O0` - 无优化，便于调试
- `-fprofile-arcs -ftest-coverage` - 代码覆盖率
- `-fsanitize=undefined,address,leak` - 内存检测（Debug 模式）

## 5. 运行测试

### 5.1 一键运行脚本

```bash
# 完整测试流程（推荐）
./run-ut.sh

# 从指定步骤开始
./run-ut.sh --from-step 4  # 跳过编译，直接运行测试

# 查看帮助
./run-ut.sh --help
```

**脚本执行步骤**:
1. 准备构建环境
2. 配置 CMake
3. 编译测试
4. 运行单元测试
5. 生成覆盖率报告
6. 生成测试报告

### 5.2 手动运行测试

```bash
# 创建构建目录
mkdir -p ../build-autotests
cd ../build-autotests

# 配置 CMake
cmake ../autotests -DCMAKE_BUILD_TYPE=Debug -DDFM_STANDALONE_TEST=ON

# 编译
cmake --build . -j $(nproc)

# 运行测试
ctest --output-on-failure --verbose

# 运行特定测试
ctest -R "test-dfm-base"

# 生成覆盖率
lcov --directory . --capture --output-file coverage.info
genhtml coverage.info --output-directory coverage/html
```

### 5.3 测试输出位置

```
../build-autotests/
├── test-reports/
│   ├── test_report.html      # 综合测试报告
│   ├── test_output.log       # 测试输出日志
│   ├── test_results.xml      # JUnit 格式测试结果
│   └── coverage_output.log    # 覆盖率生成日志
└── coverage/
    └── html/
        └── index.html         # 代码覆盖率报告
```

## 6. 测试最佳实践

### 6.1 测试组织原则

**AAA 模式 (Arrange-Act-Assert)**:
```cpp
TEST_F(MyTest, Example) {
    // Arrange: 设置测试数据和环境
    Application *app = createTestApplication();
    stub.set_lamda(/* mock */);
    
    // Act: 执行被测试的操作
    QVariant result = Application::appAttribute(Application::kIconSizeLevel);
    
    // Assert: 验证结果
    EXPECT_EQ(result.toInt(), 2);
    
    // Cleanup
    delete app;
}
```

### 6.2 测试命名规范

**测试类命名**: `<ClassName>Test`
```cpp
class ApplicationTest : public ::testing::Test { };
class SettingsTest : public ::testing::Test { };
```

**测试用例命名**: `<Feature>_<Scenario>_<ExpectedResult>`
```cpp
TEST_F(ApplicationTest, SingletonCreation_Basic) { }
TEST_F(ApplicationTest, AppAttribute_GetSet) { }
TEST_F(ApplicationTest, SignalEmission_AppAttributes) { }
TEST_F(ApplicationTest, InvalidAttribute_Handling) { }
```

### 6.3 测试覆盖范围

**应该测试**:
- ✅ 公共 API 的所有功能
- ✅ 边界条件和错误处理
- ✅ 信号和槽连接
- ✅ 状态转换
- ✅ 线程安全（如适用）

**不应该测试**:
- ❌ 实现细节（私有成员，除非必要）
- ❌ 第三方库功能
- ❌ Qt 框架本身

### 6.4 Stub 使用建议

**最佳实践**:
1. **在 SetUp 中初始化**: `stub.clear()` 确保干净的测试环境
2. **在 TearDown 中清理**: 防止 stub 泄漏到其他测试
3. **使用 lambda 捕获**: 验证函数调用和参数
4. **精确函数签名**: 使用 `static_cast` 指定重载函数

**常见模式**:
```cpp
// 模式 1: 验证调用
bool called = false;
stub.set_lamda(&SomeClass::method, [&called](...) { called = true; });
EXPECT_TRUE(called);

// 模式 2: 控制返回值
stub.set_lamda(&SomeClass::getValue, []() { return 42; });
EXPECT_EQ(obj.getValue(), 42);

// 模式 3: 验证参数
QString capturedParam;
stub.set_lamda(&SomeClass::setValue, [&capturedParam](const QString &s) { 
    capturedParam = s; 
});
EXPECT_EQ(capturedParam, "expected");
```

### 6.5 测试数据管理

**测试 Fixture 模式**:
```cpp
class ApplicationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 每个测试前执行
        stub.clear();
        app = createTestApplication();
    }
    
    void TearDown() override {
        // 每个测试后执行
        stub.clear();
        delete app;
        app = nullptr;
    }
    
    // 共享的测试辅助函数
    Application* createTestApplication() {
        return new Application();
    }
    
    // 共享的测试数据
    stub_ext::StubExt stub;
    Application *app = nullptr;
};
```

## 7. 测试框架特性

### 7.1 ASAN 支持

**自动配置**:
- ASAN 报告路径: `build-autotests/asan-reports/asan_<unit_id>.log`
- 仅在 Debug 模式下启用
- 通过 `DFM_SETUP_ASAN_REPORT(unit_id)` 宏配置

### 7.2 代码覆盖率

**自动配置**:
- 编译时: `-fprofile-arcs -ftest-coverage`
- 链接时: `-lgcov`
- 报告生成: `lcov` + `genhtml`

**覆盖率报告**:
- HTML 格式，直观展示代码覆盖情况
- 自动过滤测试文件
- 显示行覆盖率、分支覆盖率

### 7.3 测试报告生成

**自动生成**:
- JUnit 格式 XML（CI/CD 集成）
- HTML 综合报告
- 测试输出日志
- 覆盖率日志

## 8. 常见问题和解决方案

### 8.1 编译错误

**问题**: 找不到 GTest
```bash
sudo apt install libgtest-dev
```

**问题**: Qt6 测试组件缺失
```bash
# 确保安装了 Qt6 Test 组件
```

### 8.2 测试运行错误

**问题**: ASAN 报告目录不存在
- **解决**: CMake 会自动创建，检查构建目录权限

**问题**: Stub 函数未生效
- **解决**: 检查函数签名是否匹配，使用 `static_cast` 指定重载

### 8.3 覆盖率问题

**问题**: 覆盖率始终为 0
- **解决**: 确保使用 Debug 模式编译，检查编译标志

**问题**: 覆盖率报告包含测试文件
- **解决**: 已自动过滤，检查 lcov 参数

## 9. 添加新测试的步骤

### 9.1 添加库测试

1. **创建测试目录**: `autotests/libs/my-lib/`
2. **创建测试文件**: `test_mylib.cpp`
3. **创建 main.cpp**: 
   ```cpp
   #include "dfm_test_main.h"
   DFM_TEST_MAIN(my_lib)
   ```
4. **创建 CMakeLists.txt**:
   ```cmake
   cmake_minimum_required(VERSION 3.10)
   dfm_create_library_test("my-lib")
   ```
5. **在主 CMakeLists.txt 中添加**: `add_subdirectory(libs/my-lib)`

### 9.2 添加插件测试

1. **创建测试目录**: `autotests/plugins/my-plugin/`
2. **创建测试文件**: `test_myplugin.cpp`
3. **创建 main.cpp**: 
   ```cpp
   #include "dfm_test_main.h"
   DFM_TEST_MAIN(my_plugin)
   ```
4. **创建 CMakeLists.txt**:
   ```cmake
   cmake_minimum_required(VERSION 3.10)
   dfm_create_plugin_test_enhanced("my-plugin" 
       "${CMAKE_CURRENT_SOURCE_DIR}/../../src/plugins/my-plugin")
   ```
5. **在主 CMakeLists.txt 中添加**: `add_subdirectory(my-plugin)`

### 9.3 添加服务测试

类似插件测试，但使用:
```cmake
dfm_create_service_test_enhanced("my-service" 
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/services/my-service")
```

## 10. 总结

### 10.1 核心要点

1. **测试框架**: Google Test + CMake + CTest
2. **测试主入口**: `DFM_TEST_MAIN` 宏统一管理
3. **Mock 工具**: `stub_ext::StubExt` 进行函数 stub
4. **Qt 支持**: 自动选择 QApplication/QCoreApplication
5. **覆盖率**: 自动配置 lcov，生成 HTML 报告

### 10.2 工作流程

1. **编写测试**: 继承 `::testing::Test`，使用 `TEST_F` 定义测试用例
2. **配置 CMake**: 使用 DFM 测试工具函数创建测试
3. **运行测试**: 使用 `run-ut.sh` 脚本或手动运行
4. **查看报告**: 检查测试报告和覆盖率报告

### 10.3 关键文件

- `dfm_test_main.h` - 测试主入口宏
- `DFMTestUtils.cmake` - CMake 测试工具
- `stubext.h` - Stub 扩展工具
- `run-ut.sh` - 一键测试脚本

---

**最后更新**: 2025-12-12
**维护者**: zero
**版本**: v2.0 - 添加了详细的UI和事件插桩方法指南

