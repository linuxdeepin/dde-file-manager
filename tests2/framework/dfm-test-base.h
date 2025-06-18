// dfm-test-base.h - DDE文件管理器测试基础设施
// 提供增强的测试框架核心，包括改进的打桩基础设施和Qt6Test集成支持

#pragma once

#include <memory>
#include <functional>
#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include <QCoreApplication>
#include <gtest/gtest.h>

// 包含dfm-framework的头文件
#include <dfm-framework/dfm_framework_global.h>

namespace DFMTest {

/**
 * @brief 测试基类，提供通用的测试设置和清理功能
 */
class TestBase : public QObject
{
    Q_OBJECT

public:
    explicit TestBase(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~TestBase() = default;

protected:
    /**
     * @brief 初始化测试环境
     */
    virtual void initTestCase() {}
    
    /**
     * @brief 清理测试环境
     */
    virtual void cleanupTestCase() {}
    
    /**
     * @brief 每个测试前的初始化
     */
    virtual void init() {}
    
    /**
     * @brief 每个测试后的清理
     */
    virtual void cleanup() {}
};

/**
 * @brief 信号测试辅助类
 */
class SignalTester
{
public:
    explicit SignalTester(QObject *sender, const char *signal)
        : spy(std::make_unique<QSignalSpy>(sender, signal))
    {
    }
    
    /**
     * @brief 等待信号发射
     * @param timeout 超时时间（毫秒）
     * @return 是否收到信号
     */
    bool wait(int timeout = 5000) {
        return spy->wait(timeout);
    }
    
    /**
     * @brief 获取信号发射次数
     */
    int count() const {
        return spy->count();
    }
    
    /**
     * @brief 清空信号记录
     */
    void clear() {
        spy->clear();
    }
    
    /**
     * @brief 验证信号发射次数
     */
    bool verifyCount(int expected) const {
        return spy->count() == expected;
    }
    
    /**
     * @brief 获取信号参数
     */
    QList<QVariant> takeFirst() {
        return spy->takeFirst();
    }

private:
    std::unique_ptr<QSignalSpy> spy;
};

/**
 * @brief Qt对象模拟工厂
 */
template<typename QtObject>
class MockFactory
{
public:
    /**
     * @brief 创建模拟对象
     */
    static std::unique_ptr<QtObject> createMock() {
        auto mock = std::make_unique<QtObject>();
        configureForTesting(mock.get());
        return mock;
    }
    
private:
    /**
     * @brief 配置对象用于测试
     */
    static void configureForTesting(QtObject *object) {
        // 为测试配置对象
        if constexpr (std::is_base_of_v<QObject, QtObject>) {
            // 设置对象名以便调试
            object->setObjectName(QString("Mock_%1").arg(QtObject::staticMetaObject.className()));
        }
    }
};

} // namespace DFMTest

// 便利宏定义
#define DFM_TEST_SIGNAL_SPY(object, signal) \
    DFMTest::SignalTester spy_##__LINE__(object, SIGNAL(signal))

#define DFM_VERIFY_SIGNAL_EMITTED(spy, count) \
    EXPECT_TRUE(spy.verifyCount(count)) << "Expected " << count << " signals, got " << spy.count()

#define DFM_WAIT_FOR_SIGNAL(spy, timeout) \
    EXPECT_TRUE(spy.wait(timeout)) << "Signal not emitted within " << timeout << "ms"

// Qt Test和Google Test集成宏
#define DFM_TEST_MAIN(TestClass) \
int main(int argc, char *argv[]) \
{ \
    QCoreApplication app(argc, argv); \
    ::testing::InitGoogleTest(&argc, argv); \
    \
    /* 设置测试环境 */ \
    qputenv("QT_QPA_PLATFORM", "offscreen"); \
    qputenv("QT_LOGGING_RULES", "*.debug=false"); \
    \
    TestClass tc; \
    return QTest::qExec(&tc, argc, argv); \
}

#define DFM_GTEST_MAIN() \
int main(int argc, char *argv[]) \
{ \
    QCoreApplication app(argc, argv); \
    ::testing::InitGoogleTest(&argc, argv); \
    \
    /* 设置测试环境 */ \
    qputenv("QT_QPA_PLATFORM", "offscreen"); \
    qputenv("QT_LOGGING_RULES", "*.debug=false"); \
    \
    return RUN_ALL_TESTS(); \
}
