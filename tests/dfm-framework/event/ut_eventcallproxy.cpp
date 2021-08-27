#include "event/event.h"
#include "event/eventcallproxy.h"

#include <QCoreApplication>
#include <QMetaObject>
#include <QUrl>

#include <gtest/gtest.h>

DPF_USE_NAMESPACE

class TestHander1: public dpf::SynchEventHandler
{
    Qt::HANDLE threadId = nullptr;
public:
    void eventProcess(const dpf::Event &event) override
    {
        qInfo() << Q_FUNC_INFO << event
                << QThread::currentThread()
                << QThread::currentThreadId();
        threadId = QThread::currentThreadId();
    }

    Qt::HANDLE getRunThreadID(){return threadId;}

};DPF_EVENT_HANDLER(TestHander1,"WindowEvent");

class TestHander2: public dpf::AsynchEventHandler
{
    Qt::HANDLE threadId = nullptr;
public:
    void eventProcess(const dpf::Event &event) override
    {
        qInfo() << Q_FUNC_INFO << event
                << QThread::currentThread()
                << QThread::currentThreadId();
        threadId = QThread::currentThreadId();
    }

    Qt::HANDLE getRunThreadID(){return threadId;}

};DPF_EVENT_HANDLER(TestHander2,"LauchEvent");

class UT_EventCallProxy : public testing::Test
{

public:
    virtual void SetUp() override {

    }

    virtual void TearDown() override {
    }
};

TEST_F(UT_EventCallProxy, test_callEvent)
{

    Event windowEvent;
    windowEvent.setTopic("WindowEvent");
    DPF_EVENT_CALL(windowEvent);

    //同步执行线程非空
    EXPECT_EQ(true, GlobalPrivate::TestHander1_regInstence->getRunThreadID() != Qt::HANDLE(nullptr));

    //主线程
    EXPECT_EQ(true, GlobalPrivate::TestHander1_regInstence->getRunThreadID() == qApp->thread()->currentThreadId());

    //异步线程空
    EXPECT_EQ(true, GlobalPrivate::TestHander2_regInstence->getRunThreadID() == Qt::HANDLE(nullptr));

    Event lauchEvent;
    lauchEvent.setTopic("LauchEvent");
    DPF_EVENT_CALL(lauchEvent);

    //同步执行线程非空
    EXPECT_EQ(true, GlobalPrivate::TestHander1_regInstence->getRunThreadID() != Qt::HANDLE(nullptr));
    //主线程
    EXPECT_EQ(true, GlobalPrivate::TestHander1_regInstence->getRunThreadID() == qApp->thread()->currentThreadId());

    //异步线程并行等待
    sleep(5);

    //异步线程非空
    EXPECT_EQ(true, GlobalPrivate::TestHander2_regInstence->getRunThreadID() != Qt::HANDLE(nullptr));
    //异步线程非主线程
    EXPECT_EQ(true, GlobalPrivate::TestHander2_regInstence->getRunThreadID() != qApp->thread()->currentThreadId());
}
