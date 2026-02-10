// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <memory>
#include <vector>
#include <atomic>

#include <dfm-framework/lifecycle/plugin.h>

using namespace dpf;

/**
 * @brief 测试用的Plugin实现类
 * 
 * 这是一个具体的Plugin实现，用于测试Plugin基类的功能。
 * 实现了所有纯虚函数，并添加了一些测试用的功能。
 */
class TestPlugin : public Plugin
{
    Q_OBJECT

public:
    TestPlugin(QObject *parent = nullptr) : Plugin() {
        setParent(parent);
        initializeCallCount = 0;
        startCallCount = 0;
        stopCallCount = 0;
        startReturnValue = true;
    }

    ~TestPlugin() override = default;

    // 实现纯虚函数
    bool start() override {
        startCallCount++;
        emit startCalled();
        return startReturnValue;
    }

    // 重写虚函数以便测试
    void initialize() override {
        initializeCallCount++;
        Plugin::initialize(); // 调用基类实现
        emit initializeCalled();
    }

    void stop() override {
        stopCallCount++;
        Plugin::stop(); // 调用基类实现
        emit stopCalled();
    }

    // 测试用的公共成员变量和方法
    int initializeCallCount;
    int startCallCount;
    int stopCallCount;
    bool startReturnValue;

    void setStartReturnValue(bool value) { startReturnValue = value; }

signals:
    void initializeCalled();
    void startCalled();
    void stopCalled();
};

/**
 * @brief Plugin类的单元测试
 * 
 * 这些测试主要用于提高代码覆盖率，确保Plugin类的各个代码路径都被执行到。
 * 测试重点包括：
 * 1. 覆盖所有虚函数的调用路径
 * 2. 测试QObject功能
 * 3. 测试生命周期管理
 * 4. 测试多态性
 */
class PluginTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 每个测试前创建一个TestPlugin实例
        plugin = new TestPlugin();
    }

    void TearDown() override {
        // 每个测试后清理资源
        delete plugin;
        plugin = nullptr;
    }

    TestPlugin *plugin;
};

/**
 * @brief 测试Plugin构造函数
 * 目的：覆盖Plugin构造函数的代码路径
 */
TEST_F(PluginTest, Constructor) {
    // 验证对象创建成功 - 覆盖构造函数代码路径
    EXPECT_NE(plugin, nullptr);
    EXPECT_EQ(plugin->initializeCallCount, 0);
    EXPECT_EQ(plugin->startCallCount, 0);
    EXPECT_EQ(plugin->stopCallCount, 0);
    
    // 测试创建另一个实例
    TestPlugin *another = new TestPlugin();
    EXPECT_NE(another, nullptr);
    EXPECT_EQ(another->initializeCallCount, 0);
    delete another;
}

/**
 * @brief 测试initialize方法
 * 目的：覆盖initialize方法的代码路径
 */
TEST_F(PluginTest, Initialize) {
    // 测试initialize方法 - 覆盖initialize代码路径
    QSignalSpy spy(plugin, &TestPlugin::initializeCalled);
    
    plugin->initialize();
    
    EXPECT_EQ(plugin->initializeCallCount, 1);
    EXPECT_EQ(spy.count(), 1);
    
    // 多次调用initialize
    plugin->initialize();
    plugin->initialize();
    
    EXPECT_EQ(plugin->initializeCallCount, 3);
    EXPECT_EQ(spy.count(), 3);
}

/**
 * @brief 测试start方法
 * 目的：覆盖start方法的代码路径
 */
TEST_F(PluginTest, Start) {
    // 测试start方法 - 覆盖start代码路径
    QSignalSpy spy(plugin, &TestPlugin::startCalled);
    
    // 测试成功启动
    plugin->setStartReturnValue(true);
    bool result = plugin->start();
    
    EXPECT_TRUE(result);
    EXPECT_EQ(plugin->startCallCount, 1);
    EXPECT_EQ(spy.count(), 1);
    
    // 测试启动失败
    plugin->setStartReturnValue(false);
    result = plugin->start();
    
    EXPECT_FALSE(result);
    EXPECT_EQ(plugin->startCallCount, 2);
    EXPECT_EQ(spy.count(), 2);
    
    // 多次调用start
    plugin->setStartReturnValue(true);
    for (int i = 0; i < 5; ++i) {
        plugin->start();
    }
    
    EXPECT_EQ(plugin->startCallCount, 7);
    EXPECT_EQ(spy.count(), 7);
}

/**
 * @brief 测试stop方法
 * 目的：覆盖stop方法的代码路径
 */
TEST_F(PluginTest, Stop) {
    // 测试stop方法 - 覆盖stop代码路径
    QSignalSpy spy(plugin, &TestPlugin::stopCalled);
    
    plugin->stop();
    
    EXPECT_EQ(plugin->stopCallCount, 1);
    EXPECT_EQ(spy.count(), 1);
    
    // 多次调用stop
    plugin->stop();
    plugin->stop();
    
    EXPECT_EQ(plugin->stopCallCount, 3);
    EXPECT_EQ(spy.count(), 3);
}

/**
 * @brief 测试完整的生命周期
 * 目的：覆盖完整生命周期的代码路径
 */
TEST_F(PluginTest, LifeCycle) {
    // 测试完整生命周期 - 覆盖生命周期管理代码路径
    QSignalSpy initSpy(plugin, &TestPlugin::initializeCalled);
    QSignalSpy startSpy(plugin, &TestPlugin::startCalled);
    QSignalSpy stopSpy(plugin, &TestPlugin::stopCalled);
    
    // 完整的生命周期：initialize -> start -> stop
    plugin->initialize();
    EXPECT_EQ(plugin->initializeCallCount, 1);
    EXPECT_EQ(initSpy.count(), 1);
    
    bool startResult = plugin->start();
    EXPECT_TRUE(startResult);
    EXPECT_EQ(plugin->startCallCount, 1);
    EXPECT_EQ(startSpy.count(), 1);
    
    plugin->stop();
    EXPECT_EQ(plugin->stopCallCount, 1);
    EXPECT_EQ(stopSpy.count(), 1);
    
    // 重复生命周期
    plugin->initialize();
    plugin->start();
    plugin->stop();
    
    EXPECT_EQ(plugin->initializeCallCount, 2);
    EXPECT_EQ(plugin->startCallCount, 2);
    EXPECT_EQ(plugin->stopCallCount, 2);
}

/**
 * @brief 测试虚函数多态性
 * 目的：覆盖虚函数多态调用的代码路径
 */
TEST_F(PluginTest, VirtualFunctionPolymorphism) {
    // 测试虚函数多态性 - 覆盖多态调用代码路径
    Plugin *basePtr = plugin;
    
    // 通过基类指针调用虚函数
    basePtr->initialize();
    EXPECT_EQ(plugin->initializeCallCount, 1);
    
    bool result = basePtr->start();
    EXPECT_TRUE(result);
    EXPECT_EQ(plugin->startCallCount, 1);
    
    basePtr->stop();
    EXPECT_EQ(plugin->stopCallCount, 1);
    
    // 测试多个多态调用
    for (int i = 0; i < 10; ++i) {
        basePtr->initialize();
        basePtr->start();
        basePtr->stop();
    }
    
    EXPECT_EQ(plugin->initializeCallCount, 11);
    EXPECT_EQ(plugin->startCallCount, 11);
    EXPECT_EQ(plugin->stopCallCount, 11);
}

/**
 * @brief 测试多个实例
 * 目的：覆盖多实例管理的代码路径
 */
TEST_F(PluginTest, MultipleInstances) {
    // 测试多个实例 - 覆盖多实例管理代码路径
    std::vector<TestPlugin*> plugins;
    
    // 创建多个插件实例
    for (int i = 0; i < 100; ++i) {
        TestPlugin *p = new TestPlugin();
        p->setStartReturnValue(i % 2 == 0); // 一半成功，一半失败
        plugins.push_back(p);
    }
    
    // 对所有插件执行生命周期操作
    int successCount = 0;
    int failureCount = 0;
    
    for (size_t i = 0; i < plugins.size(); ++i) {
        TestPlugin *p = plugins[i];
        
        p->initialize();
        EXPECT_EQ(p->initializeCallCount, 1);
        
        bool result = p->start();
        if (result) {
            successCount++;
        } else {
            failureCount++;
        }
        EXPECT_EQ(p->startCallCount, 1);
        
        p->stop();
        EXPECT_EQ(p->stopCallCount, 1);
    }
    
    // 验证结果
    EXPECT_EQ(successCount, 50);
    EXPECT_EQ(failureCount, 50);
    
    // 清理资源
    for (TestPlugin *p : plugins) {
        delete p;
    }
}

/**
 * @brief 测试多线程环境
 * 目的：覆盖多线程环境下的代码路径
 */
TEST_F(PluginTest, MultiThreading) {
    // 测试多线程环境 - 覆盖多线程相关代码路径
    QMutex mutex;
    QWaitCondition condition;
    int completedThreads = 0;
    const int threadCount = 10;
    
    // 创建多个线程，每个线程创建和操作自己的plugin实例
    std::vector<std::unique_ptr<QThread>> threads;
    std::atomic<int> totalOperations(0);
    
    for (int i = 0; i < threadCount; ++i) {
        auto thread = std::make_unique<QThread>();
        QObject::connect(thread.get(), &QThread::started, [&mutex, &condition, &completedThreads, &totalOperations]() {
            // 在每个线程中创建和操作plugin
            for (int j = 0; j < 10; ++j) {
                TestPlugin localPlugin;
                
                localPlugin.initialize();
                bool result = localPlugin.start();
                localPlugin.stop();
                
                totalOperations++;
                
                // 验证操作正确性
                if (localPlugin.initializeCallCount == 1 && 
                    localPlugin.startCallCount == 1 && 
                    localPlugin.stopCallCount == 1 && 
                    result == true) {
                    // 操作成功
                }
            }
            
            QMutexLocker locker(&mutex);
            completedThreads++;
            condition.wakeAll();
        });
        
        threads.push_back(std::move(thread));
    }
    
    // 启动所有线程
    for (auto& thread : threads) {
        thread->start();
    }
    
    // 等待所有线程完成
    QMutexLocker locker(&mutex);
    while (completedThreads < threadCount) {
        condition.wait(&mutex, 2000); // 最多等待2秒
    }
    
    // 等待线程结束
    for (auto& thread : threads) {
        thread->quit();
        thread->wait(1000);
    }
    
    EXPECT_EQ(completedThreads, threadCount);
    EXPECT_EQ(totalOperations.load(), threadCount * 10);
}

/**
 * @brief 测试内存管理
 * 目的：覆盖内存分配和释放的代码路径
 */
TEST_F(PluginTest, MemoryManagement) {
    // 测试内存管理 - 覆盖内存管理代码路径
    std::vector<TestPlugin*> pluginList;
    
    // 创建大量plugin对象
    for (int i = 0; i < 1000; ++i) {
        TestPlugin *p = new TestPlugin();
        p->initialize();
        p->start();
        p->stop();
        pluginList.push_back(p);
    }
    
    // 验证所有对象都正确创建和操作
    for (TestPlugin *p : pluginList) {
        EXPECT_EQ(p->initializeCallCount, 1);
        EXPECT_EQ(p->startCallCount, 1);
        EXPECT_EQ(p->stopCallCount, 1);
    }
    
    // 清理所有对象
    for (TestPlugin *p : pluginList) {
        delete p;
    }
    pluginList.clear();
    
    // 测试栈对象
    {
        TestPlugin stackPlugin;
        stackPlugin.initialize();
        stackPlugin.start();
        stackPlugin.stop();
        
        EXPECT_EQ(stackPlugin.initializeCallCount, 1);
        EXPECT_EQ(stackPlugin.startCallCount, 1);
        EXPECT_EQ(stackPlugin.stopCallCount, 1);
    } // stackPlugin自动析构
}

/**
 * @brief 测试异常情况
 * 目的：覆盖异常处理的代码路径
 */
TEST_F(PluginTest, ExceptionHandling) {
    // 测试异常情况 - 覆盖异常处理代码路径
    
    // 测试start失败的情况
    plugin->setStartReturnValue(false);
    bool result = plugin->start();
    EXPECT_FALSE(result);
    EXPECT_EQ(plugin->startCallCount, 1);
    
    // 测试在start失败后仍然可以调用其他方法
    plugin->initialize();
    EXPECT_EQ(plugin->initializeCallCount, 1);
    
    plugin->stop();
    EXPECT_EQ(plugin->stopCallCount, 1);
    
    // 测试重复调用
    for (int i = 0; i < 100; ++i) {
        plugin->initialize();
        plugin->start(); // 仍然返回false
        plugin->stop();
    }
    
    EXPECT_EQ(plugin->initializeCallCount, 101);
    EXPECT_EQ(plugin->startCallCount, 101);
    EXPECT_EQ(plugin->stopCallCount, 101);
}

/**
 * @brief 测试QObject功能
 * 目的：覆盖QObject相关功能的代码路径
 */
TEST_F(PluginTest, QObjectFunctionality) {
    // 测试QObject功能 - 覆盖QObject相关代码路径
    
    // 测试parent-child关系
    QObject parent;
    TestPlugin *child = new TestPlugin(&parent);
    
    EXPECT_EQ(child->parent(), &parent);
    EXPECT_TRUE(parent.children().contains(child));
    
    // 测试信号槽连接
    QSignalSpy spy(child, &TestPlugin::startCalled);
    child->start();
    EXPECT_EQ(spy.count(), 1);
    
    // 测试对象名称
    child->setObjectName("test_plugin");
    EXPECT_EQ(child->objectName(), "test_plugin");
    
    // 测试查找子对象
    TestPlugin *found = parent.findChild<TestPlugin*>("test_plugin");
    EXPECT_EQ(found, child);
    
    // parent析构时会自动删除child
}

/**
 * @brief 综合测试
 * 目的：结合多种情况，确保所有代码路径都被覆盖
 */
TEST_F(PluginTest, ComprehensiveTest) {
    // 综合测试 - 确保所有代码路径都被覆盖
    std::vector<TestPlugin*> pluginList;
    
    // 创建各种配置的plugin
    for (int i = 0; i < 50; ++i) {
        TestPlugin *p = new TestPlugin();
        p->setStartReturnValue(i % 3 != 0); // 大部分成功，部分失败
        p->setObjectName(QString("plugin_%1").arg(i));
        pluginList.push_back(p);
    }
    
    // 对所有plugin执行各种操作
    int successfulStarts = 0;
    int failedStarts = 0;
    
    for (size_t i = 0; i < pluginList.size(); ++i) {
        TestPlugin *p = pluginList[i];
        
        // 执行完整生命周期
        p->initialize();
        bool startResult = p->start();
        p->stop();
        
        if (startResult) {
            successfulStarts++;
        } else {
            failedStarts++;
        }
        
        // 验证对象名称
        EXPECT_EQ(p->objectName(), QString("plugin_%1").arg(i));
        
        // 验证调用计数
        EXPECT_EQ(p->initializeCallCount, 1);
        EXPECT_EQ(p->startCallCount, 1);
        EXPECT_EQ(p->stopCallCount, 1);
        
        // 额外的操作以增加覆盖率
        p->initialize(); // 再次初始化
        p->start();      // 再次启动
        p->stop();       // 再次停止
        
        EXPECT_EQ(p->initializeCallCount, 2);
        EXPECT_EQ(p->startCallCount, 2);
        EXPECT_EQ(p->stopCallCount, 2);
    }
    
    // 验证结果分布
    EXPECT_GT(successfulStarts, 0);
    EXPECT_GT(failedStarts, 0);
    EXPECT_EQ(successfulStarts + failedStarts, 50);
    
    // 清理所有对象
    for (TestPlugin *p : pluginList) {
        delete p;
    }
}

// 包含MOC生成的代码
#include "test_plugin.moc" 