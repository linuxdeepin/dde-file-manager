#include "event/event.h"
#include "event/eventcallproxy.h"

#include <QCoreApplication>
#include <QMetaObject>
#include <QUrl>

#include <gtest/gtest.h>

DPF_USE_NAMESPACE

class SynchInhClass: public dpf::EventHandler, AutoEventHandlerRegister<SynchInhClass>
{
    Qt::HANDLE threadId = nullptr;
public:
    SynchInhClass(): AutoEventHandlerRegister<SynchInhClass>() {}
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

    Qt::HANDLE getRunThreadID(){return threadId;}

};

class AsynchInhClass: public dpf::EventHandler, AutoEventHandlerRegister<AsynchInhClass>
{
    Qt::HANDLE threadId = nullptr;
public:
    AsynchInhClass():  AutoEventHandlerRegister<AsynchInhClass>() {}

    static EventHandler::Type type()
    {
        return EventHandler::Type::Async;
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

    Qt::HANDLE getRunThreadID(){return threadId;}

};

class UT_EventCallProxy : public testing::Test
{

public:

    virtual void SetUp() override {

    }

    virtual void TearDown() override {
    }
};

TEST_F(UT_EventCallProxy, test_inhSynchEventHandler)
{

}

TEST_F(UT_EventCallProxy, test_inhAsynchEventHandler)
{

}
