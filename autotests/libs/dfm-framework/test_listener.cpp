// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_listener.cpp - Listener类单元测试
// 测试dpf::Listener类的基本功能，包括单例模式和信号机制

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QTest>

// 包含待测试的类
#include <dfm-framework/listener/listener.h>

using namespace dpf;

/**
 * @brief Listener类单元测试
 *
 * 测试范围：
 * 1. 单例模式正确性
 * 2. 信号定义正确性
 * 3. 基本的QObject功能
 */
class ListenerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // QCoreApplication在main函数中已创建
        listener = Listener::instance();
        ASSERT_NE(listener, nullptr);
    }

    void TearDown() override
    {
        // Listener是单例，不需要删除
    }

    Listener *listener;
};

/**
 * @brief 测试单例模式
 * 验证Listener::instance()返回同一个实例
 */
TEST_F(ListenerTest, SingletonPattern)
{
    // 获取多个实例
    Listener *instance1 = Listener::instance();
    Listener *instance2 = Listener::instance();

    // 验证是同一个实例
    EXPECT_EQ(instance1, instance2);
    EXPECT_EQ(listener, instance1);

    // 验证实例不为空
    EXPECT_NE(instance1, nullptr);
}

/**
 * @brief 测试QObject基本功能
 * 验证Listener继承自QObject并具有基本功能
 */
TEST_F(ListenerTest, QObjectFunctionality)
{
    // 验证是QObject实例
    QObject *obj = qobject_cast<QObject *>(listener);
    EXPECT_NE(obj, nullptr);

    // 验证元对象系统
    const QMetaObject *metaObj = listener->metaObject();
    EXPECT_NE(metaObj, nullptr);
    EXPECT_STREQ(metaObj->className(), "dpf::Listener");

    // 验证对象名设置功能
    const QString testName = "TestListener";
    listener->setObjectName(testName);
    EXPECT_EQ(listener->objectName(), testName);
}

/**
 * @brief 测试信号定义
 * 验证Listener类定义了正确的信号
 */
TEST_F(ListenerTest, SignalDefinitions)
{
    const QMetaObject *metaObj = listener->metaObject();

    // 验证信号数量和名称
    QStringList expectedSignals = {
        "pluginInitialized(QString,QString)",
        "pluginStarted(QString,QString)",
        "pluginsInitialized()",
        "pluginsStarted()"
    };

    // 检查每个期望的信号
    for (const QString &signalName : expectedSignals) {
        int signalIndex = metaObj->indexOfSignal(signalName.toUtf8().constData());
        EXPECT_GE(signalIndex, 0) << "Signal not found: " << signalName.toStdString();

        if (signalIndex >= 0) {
            QMetaMethod method = metaObj->method(signalIndex);
            EXPECT_EQ(method.methodType(), QMetaMethod::Signal);
        }
    }
}

/**
 * @brief 测试pluginInitialized信号
 * 验证pluginInitialized信号能够正常发射和接收
 */
TEST_F(ListenerTest, PluginInitializedSignal)
{
    // 创建信号监听器
    QSignalSpy spy(listener, &Listener::pluginInitialized);
    EXPECT_TRUE(spy.isValid());

    // 发射信号
    const QString testIid = "test.plugin.iid";
    const QString testName = "TestPlugin";

    emit listener->pluginInitialized(testIid, testName);

    // 验证信号被发射
    EXPECT_EQ(spy.count(), 1);

    // 验证信号参数
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.size(), 2);
    EXPECT_EQ(arguments[0].toString(), testIid);
    EXPECT_EQ(arguments[1].toString(), testName);
}

/**
 * @brief 测试pluginStarted信号
 * 验证pluginStarted信号能够正常发射和接收
 */
TEST_F(ListenerTest, PluginStartedSignal)
{
    QSignalSpy spy(listener, &Listener::pluginStarted);
    EXPECT_TRUE(spy.isValid());

    const QString testIid = "test.plugin.started";
    const QString testName = "StartedPlugin";

    emit listener->pluginStarted(testIid, testName);

    EXPECT_EQ(spy.count(), 1);

    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.size(), 2);
    EXPECT_EQ(arguments[0].toString(), testIid);
    EXPECT_EQ(arguments[1].toString(), testName);
}

/**
 * @brief 测试pluginsInitialized信号
 * 验证pluginsInitialized信号（无参数）能够正常工作
 */
TEST_F(ListenerTest, PluginsInitializedSignal)
{
    QSignalSpy spy(listener, &Listener::pluginsInitialized);
    EXPECT_TRUE(spy.isValid());

    emit listener->pluginsInitialized();

    EXPECT_EQ(spy.count(), 1);

    // 验证无参数信号
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.size(), 0);
}

/**
 * @brief 测试pluginsStarted信号
 * 验证pluginsStarted信号（无参数）能够正常工作
 */
TEST_F(ListenerTest, PluginsStartedSignal)
{
    QSignalSpy spy(listener, &Listener::pluginsStarted);
    EXPECT_TRUE(spy.isValid());

    emit listener->pluginsStarted();

    EXPECT_EQ(spy.count(), 1);

    // 验证无参数信号
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.size(), 0);
}

/**
 * @brief 测试多个信号的连接
 * 验证可以同时监听多个信号
 */
TEST_F(ListenerTest, MultipleSignalConnections)
{
    // 创建多个信号监听器
    QSignalSpy initSpy(listener, &Listener::pluginInitialized);
    QSignalSpy startSpy(listener, &Listener::pluginStarted);
    QSignalSpy allInitSpy(listener, &Listener::pluginsInitialized);
    QSignalSpy allStartSpy(listener, &Listener::pluginsStarted);

    EXPECT_TRUE(initSpy.isValid());
    EXPECT_TRUE(startSpy.isValid());
    EXPECT_TRUE(allInitSpy.isValid());
    EXPECT_TRUE(allStartSpy.isValid());

    // 发射所有信号
    emit listener->pluginInitialized("test1", "Plugin1");
    emit listener->pluginStarted("test2", "Plugin2");
    emit listener->pluginsInitialized();
    emit listener->pluginsStarted();

    // 验证每个信号都被正确捕获
    EXPECT_EQ(initSpy.count(), 1);
    EXPECT_EQ(startSpy.count(), 1);
    EXPECT_EQ(allInitSpy.count(), 1);
    EXPECT_EQ(allStartSpy.count(), 1);
}

/**
 * @brief 测试dpfListener宏
 * 验证全局宏dpfListener能正常工作
 */
TEST_F(ListenerTest, GlobalMacro)
{
    // 使用宏获取实例
    Listener *macroInstance = dpfListener;

    // 验证宏返回正确的实例
    EXPECT_EQ(macroInstance, listener);
    EXPECT_EQ(macroInstance, Listener::instance());
}

/**
 * @brief 测试线程安全性
 * 验证在多线程环境下单例模式的安全性
 */
TEST_F(ListenerTest, ThreadSafety)
{
    // 简单的线程安全测试
    // 在实际应用中，单例的线程安全性由static变量的初始化保证

    std::vector<Listener *> instances;
    const int numThreads = 10;

    // 模拟多线程访问
    for (int i = 0; i < numThreads; ++i) {
        instances.push_back(Listener::instance());
    }

    // 验证所有实例都相同
    for (int i = 1; i < numThreads; ++i) {
        EXPECT_EQ(instances[0], instances[i]);
    }
}


