#include "event/event.h"
#include "event/eventcallproxy.h"
#include "definitions/globaldefinitions.h"

#include <QCoreApplication>
#include <QMetaObject>
#include <QUrl>

#include <gtest/gtest.h>

DPF_USE_NAMESPACE

class SynchInhClass: public dpf::SynchEventHandler
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

};DPF_EVENT_HANDLER(SynchInhClass,"WindowEvent");

class AsynchInhClass: public dpf::AsynchEventHandler
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

};DPF_EVENT_HANDLER(AsynchInhClass,"WindowEvent");

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
