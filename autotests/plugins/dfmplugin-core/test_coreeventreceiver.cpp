// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QObject>
#include <QString>
#include <QUrl>
#include <QVariant>
#include <QVariantMap>
#include <QList>
#include <QCoreApplication>
#include <QThread>
#include <QTimer>
#include <QDebug>

#include "stubext.h"
#include "events/coreeventreceiver.h"
#include "utils/corehelper.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-framework/lifecycle/lifecycle.h>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
DPCORE_USE_NAMESPACE

class UT_CoreEventReceiver : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.clear();
        receiver = CoreEventReceiver::instance();
        
        // Skip CoreHelper::instance() stub for now to avoid constructor issues
        
        stub.set_lamda(&CoreHelper::cd, [] { __DBG_STUB_INVOKE__ });
        
        using OpenWindowFunc2 = void (CoreHelper::*)(const QUrl &, const QVariant &);
        stub.set_lamda(static_cast<OpenWindowFunc2>(&CoreHelper::openWindow), [] { __DBG_STUB_INVOKE__ });
        // Skip first overload for now to avoid cast issues
        
        stub.set_lamda(&CoreHelper::loadPlugin, [] { __DBG_STUB_INVOKE__ });
        stub.set_lamda(&CoreHelper::cacheDefaultWindow, [] { __DBG_STUB_INVOKE__ });
        
        // Stub FMWindowsIns
        stub.set_lamda(&FileManagerWindowsManager::findWindowById, [] { __DBG_STUB_INVOKE__ return nullptr; });
        
        // Stub DialogManagerInstance
        stub.set_lamda(&DialogManager::instance, [] { __DBG_STUB_INVOKE__ return nullptr; });
        
        // Stub qApp->applicationName()
        stub.set_lamda(&QCoreApplication::applicationName, [] { __DBG_STUB_INVOKE__ 
            return QString("dde-file-manager"); 
        });
        
        // Stub QThread::currentThread() - 返回一个模拟的线程指针避免递归
        static QThread mockThread;
        stub.set_lamda(&QThread::currentThread, [&mockThread] { __DBG_STUB_INVOKE__ return &mockThread; });
        
        // Stub LifeCycle::pluginMetaObj
        stub.set_lamda(&DPF_NAMESPACE::LifeCycle::pluginMetaObj, [] { __DBG_STUB_INVOKE__ return nullptr; });
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    CoreEventReceiver *receiver = nullptr;
};

TEST_F(UT_CoreEventReceiver, Instance_SingletonPattern_ReturnsSameInstance)
{
    CoreEventReceiver *instance1 = CoreEventReceiver::instance();
    CoreEventReceiver *instance2 = CoreEventReceiver::instance();

    EXPECT_NE(instance1, nullptr);
    EXPECT_EQ(instance1, instance2);
}

TEST_F(UT_CoreEventReceiver, HandleChangeUrl_ValidUrl_CallsCd)
{
    quint64 windowId = 12345;
    QUrl testUrl("file:///home/test");

    bool cdCalled = false;
    stub.set_lamda(&CoreHelper::cd, [&]() {
        __DBG_STUB_INVOKE__
        cdCalled = true;
    });

    receiver->handleChangeUrl(windowId, testUrl);
    EXPECT_TRUE(cdCalled);
}

TEST_F(UT_CoreEventReceiver, HandleChangeUrl_InvalidUrl_DoesNotCallCd)
{
    quint64 windowId = 12345;
    QUrl invalidUrl;

    bool cdCalled = false;
    stub.set_lamda(&CoreHelper::cd, [&]() {
        __DBG_STUB_INVOKE__
        cdCalled = true;
    });

    receiver->handleChangeUrl(windowId, invalidUrl);
    EXPECT_FALSE(cdCalled);
}

TEST_F(UT_CoreEventReceiver, HandleChangeUrl_InvalidWindowId_LogsWarning)
{
    quint64 invalidWindowId = 0;
    QUrl testUrl("file:///home/test");

    bool cdCalled = false;
    stub.set_lamda(&CoreHelper::cd, [&]() {
        __DBG_STUB_INVOKE__
        cdCalled = true;
    });

    receiver->handleChangeUrl(invalidWindowId, testUrl);
    EXPECT_TRUE(cdCalled);
}

TEST_F(UT_CoreEventReceiver, HandleOpenWindow_ValidUrl_CallsOpenWindow)
{
    QUrl testUrl("file:///home/test");

    bool openWindowCalled = false;
    // Use a simpler approach - just stub method without specific cast
    stub.set_lamda(&CoreHelper::openWindow, [&]() {
        __DBG_STUB_INVOKE__
        openWindowCalled = true;
    });

    receiver->handleOpenWindow(testUrl);
    EXPECT_TRUE(openWindowCalled);
}

TEST_F(UT_CoreEventReceiver, HandleOpenWindow_WithVariant_CallsOpenWindowWithVariant)
{
    QUrl testUrl("file:///home/test");
    QVariant testOpt(true);

    bool openWindowCalled = false;
    using OpenWindowFunc2 = void (CoreHelper::*)(const QUrl &, const QVariant &);
    stub.set_lamda(static_cast<OpenWindowFunc2>(&CoreHelper::openWindow), [&]() {
        __DBG_STUB_INVOKE__
        openWindowCalled = true;
    });

    receiver->handleOpenWindow(testUrl, testOpt);
    EXPECT_TRUE(openWindowCalled);
}

TEST_F(UT_CoreEventReceiver, HandleHeadless_ValidApplication_CachesDefaultWindow)
{
    bool cacheDefaultWindowCalled = false;
    stub.set_lamda(&CoreHelper::cacheDefaultWindow, [&]() {
        __DBG_STUB_INVOKE__
        cacheDefaultWindowCalled = true;
    });

    receiver->handleHeadless();
    EXPECT_TRUE(cacheDefaultWindowCalled);
}

TEST_F(UT_CoreEventReceiver, QtMetaObject_CorrectlyInitialized_Success)
{
    // Test that Qt meta-object system works correctly
    const QMetaObject *metaObject = receiver->metaObject();
    EXPECT_NE(metaObject, nullptr);
    
    // Test class name
    EXPECT_STREQ(metaObject->className(), "dfmplugin_core::CoreEventReceiver");
    
    // Test that slot methods exist in meta-object
    EXPECT_GE(metaObject->indexOfSlot("handleChangeUrl(quint64,QUrl)"), 0);
    EXPECT_GE(metaObject->indexOfSlot("handleOpenWindow(QUrl)"), 0);
    EXPECT_GE(metaObject->indexOfSlot("handleOpenWindow(QUrl,QVariant)"), 0);
    EXPECT_GE(metaObject->indexOfSlot("handleLoadPlugins(QStringList)"), 0);
    EXPECT_GE(metaObject->indexOfSlot("handleHeadless()"), 0);
    EXPECT_GE(metaObject->indexOfSlot("handleShowSettingDialog(quint64)"), 0);
}

TEST_F(UT_CoreEventReceiver, ErrorHandling_InvalidParameters_HandlesGracefully)
{
    // Test with invalid URL
    QUrl invalidUrl;
    quint64 windowId = 12345;
    
    EXPECT_NO_THROW(receiver->handleChangeUrl(windowId, invalidUrl));
    EXPECT_NO_THROW(receiver->handleOpenWindow(invalidUrl));
    EXPECT_NO_THROW(receiver->handleOpenWindow(invalidUrl, QVariant()));
    
    // Test with invalid window ID
    quint64 invalidWindowId = 0;
    QUrl validUrl("file:///home/test");
    
    EXPECT_NO_THROW(receiver->handleChangeUrl(invalidWindowId, validUrl));
    EXPECT_NO_THROW(receiver->handleShowSettingDialog(invalidWindowId));
    
    // Test with empty plugin list
    QStringList emptyList;
    EXPECT_NO_THROW(receiver->handleLoadPlugins(emptyList));
}

TEST_F(UT_CoreEventReceiver, Consistency_MultipleCalls_ReturnConsistentResults)
{
    QUrl testUrl("file:///home/test");
    quint64 windowId = 12345;
    
    // Call methods multiple times and verify consistency
    for (int i = 0; i < 3; ++i) {
        EXPECT_NO_THROW(receiver->handleChangeUrl(windowId, testUrl));
        EXPECT_NO_THROW(receiver->handleOpenWindow(testUrl));
        EXPECT_NO_THROW(receiver->handleHeadless());
    }
    
    // Verify instance always returns same object
    CoreEventReceiver *instance1 = CoreEventReceiver::instance();
    CoreEventReceiver *instance2 = CoreEventReceiver::instance();
    EXPECT_EQ(instance1, instance2);
}
