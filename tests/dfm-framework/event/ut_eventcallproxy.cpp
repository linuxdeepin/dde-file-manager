#include "event/pubsub/event.h"
#include "event/pubsub/eventcallproxy.h"
#include "framework.h"

#include <QCoreApplication>
#include <QMetaObject>
#include <QUrl>

#include <gtest/gtest.h>

class TestHander1 : public dpf::EventHandler, dpf::AutoEventHandlerRegister<TestHander1>
{
    // add Q_OBJECT in business code
public:
    static Qt::HANDLE threadId;

    TestHander1()
        : dpf::AutoEventHandlerRegister<TestHander1>() {}

    static EventHandler::Type type()
    {
        return EventHandler::Type::Sync;
    }

    static QStringList topics()
    {
        return QStringList() << "WindowEvent";
    }

    void eventProcess(const dpf::Event &event) override
    {
        qInfo() << Q_FUNC_INFO << event
                << QThread::currentThread()
                << QThread::currentThreadId();
        threadId = QThread::currentThreadId();
    }

    static Qt::HANDLE getRunThreadID() { return TestHander1::threadId; }
};
Qt::HANDLE TestHander1::threadId = nullptr;

class TestHander2 : public dpf::EventHandler, dpf::AutoEventHandlerRegister<TestHander2>
{
    // add Q_OBJECT in business code
public:
    static Qt::HANDLE threadId;
    TestHander2()
        : dpf::AutoEventHandlerRegister<TestHander2>() {}

    static EventHandler::Type type()
    {
        return EventHandler::Type::Async;
    }

    static QStringList topics()
    {
        return QStringList() << "LauchEvent";
    }

    void eventProcess(const dpf::Event &event) override
    {
        qInfo() << Q_FUNC_INFO << event
                << QThread::currentThread()
                << QThread::currentThreadId();
        threadId = QThread::currentThreadId();
    }

    static Qt::HANDLE getRunThreadID() { return threadId; }
};
Qt::HANDLE TestHander2::threadId = nullptr;

class UT_EventCallProxy : public testing::Test
{

public:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
    }
};

TEST_F(UT_EventCallProxy, test_callEvent)
{

    dpf::Event windowEvent;
    windowEvent.setTopic("WindowEvent");
    EXPECT_EQ(true, dpf::EventCallProxy::instance().pubEvent(windowEvent));

    // 同步执行线程非空
    EXPECT_EQ(true, TestHander1::getRunThreadID() != Qt::HANDLE(nullptr));

    // 主线程
    EXPECT_EQ(true, TestHander1::getRunThreadID() == qApp->thread()->currentThreadId());

    // 异步线程空
    EXPECT_EQ(true, TestHander2::getRunThreadID() == Qt::HANDLE(nullptr));

    dpf::Event lauchEvent;
    lauchEvent.setTopic("LauchEvent");
    EXPECT_EQ(true, dpf::EventCallProxy::instance().pubEvent(lauchEvent));

    // 同步执行线程非空
    EXPECT_EQ(true, TestHander1::getRunThreadID() != Qt::HANDLE(nullptr));
    // 主线程
    EXPECT_EQ(true, TestHander1::getRunThreadID() == qApp->thread()->currentThreadId());

    // 异步线程并行等待
    sleep(2);

    // 异步线程非空
    EXPECT_EQ(true, TestHander2::getRunThreadID() != Qt::HANDLE(nullptr));
    // 异步线程非主线程
    EXPECT_EQ(true, TestHander2::getRunThreadID() != qApp->thread()->currentThreadId());
}
