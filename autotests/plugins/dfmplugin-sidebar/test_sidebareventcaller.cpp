// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "events/sidebareventcaller.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-framework/event/event.h>

#include <QUrl>

using namespace dfmplugin_sidebar;
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

class UT_SideBarEventCaller : public testing::Test
{
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_SideBarEventCaller, SendItemActived)
{
    bool isCalled = false;
    quint64 capturedWindowId = 0;
    QUrl capturedUrl;

    stub_ext::StubExt stub;

    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, quint64, const QUrl &);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
        [&isCalled, &capturedWindowId, &capturedUrl](EventDispatcherManager *, EventType type,
                                                       quint64 windowId, const QUrl &url) -> bool {
        __DBG_STUB_INVOKE__
        if (type == GlobalEventType::kChangeCurrentUrl) {
            isCalled = true;
            capturedWindowId = windowId;
            capturedUrl = url;
        }
        return true;
    });

    quint64 testWindowId = 12345;
    QUrl testUrl = QUrl::fromLocalFile("/home/test");

    SideBarEventCaller::sendItemActived(testWindowId, testUrl);

    EXPECT_TRUE(isCalled);
    EXPECT_EQ(capturedWindowId, testWindowId);
    EXPECT_EQ(capturedUrl, testUrl);
}

TEST_F(UT_SideBarEventCaller, SendEject)
{
    bool isCalled = false;
    QUrl capturedUrl;

    stub_ext::StubExt stub;

    typedef bool (EventDispatcherManager::*PublishFunc)(const QString &, const QString &, QUrl);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
        [&isCalled, &capturedUrl](EventDispatcherManager *, const QString &space,
                                   const QString &topic, const QUrl &url) -> bool {
        __DBG_STUB_INVOKE__
        if (space == "dfmplugin_sidebar" && topic == "signal_Item_EjectClicked") {
            isCalled = true;
            capturedUrl = url;
        }
        return true;
    });

    QUrl deviceUrl = QUrl::fromLocalFile("/dev/sdb1");
    SideBarEventCaller::sendEject(deviceUrl);

    EXPECT_TRUE(isCalled);
    EXPECT_EQ(capturedUrl, deviceUrl);
}

TEST_F(UT_SideBarEventCaller, SendOpenWindow_NewWindow)
{
    bool isCalled = false;
    QUrl capturedUrl;
    bool capturedIsNew = false;

    stub_ext::StubExt stub;

    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, QUrl, const bool &);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
        [&isCalled, &capturedUrl, &capturedIsNew](EventDispatcherManager *, EventType type,
                                                    const QUrl &url, bool isNew) -> bool {
        __DBG_STUB_INVOKE__
        if (type == GlobalEventType::kOpenNewWindow) {
            isCalled = true;
            capturedUrl = url;
            capturedIsNew = isNew;
        }
        return true;
    });

    QUrl testUrl = QUrl::fromLocalFile("/home/newwindow");
    SideBarEventCaller::sendOpenWindow(testUrl, true);

    EXPECT_TRUE(isCalled);
    EXPECT_EQ(capturedUrl, testUrl);
    EXPECT_TRUE(capturedIsNew);
}

TEST_F(UT_SideBarEventCaller, SendOpenWindow_ExistingWindow)
{
    bool isCalled = false;
    QUrl capturedUrl;
    bool capturedIsNew = true;

    stub_ext::StubExt stub;

    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, QUrl, const bool &);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
        [&isCalled, &capturedUrl, &capturedIsNew](EventDispatcherManager *, EventType type,
                                                    const QUrl &url, bool isNew) -> bool {
        __DBG_STUB_INVOKE__
        if (type == GlobalEventType::kOpenNewWindow) {
            isCalled = true;
            capturedUrl = url;
            capturedIsNew = isNew;
        }
        return true;
    });

    QUrl testUrl = QUrl::fromLocalFile("/home/existing");
    SideBarEventCaller::sendOpenWindow(testUrl, false);

    EXPECT_TRUE(isCalled);
    EXPECT_EQ(capturedUrl, testUrl);
    EXPECT_FALSE(capturedIsNew);
}

TEST_F(UT_SideBarEventCaller, SendOpenTab)
{
    bool isCalled = false;
    quint64 capturedWindowId = 0;
    QUrl capturedUrl;

    stub_ext::StubExt stub;

    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, quint64, const QUrl &);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
        [&isCalled, &capturedWindowId, &capturedUrl](EventDispatcherManager *, EventType type,
                                                       quint64 windowId, const QUrl &url) -> bool {
        __DBG_STUB_INVOKE__
        if (type == GlobalEventType::kOpenNewTab) {
            isCalled = true;
            capturedWindowId = windowId;
            capturedUrl = url;
        }
        return true;
    });

    quint64 testWindowId = 54321;
    QUrl testUrl = QUrl::fromLocalFile("/home/newtab");

    SideBarEventCaller::sendOpenTab(testWindowId, testUrl);

    EXPECT_TRUE(isCalled);
    EXPECT_EQ(capturedWindowId, testWindowId);
    EXPECT_EQ(capturedUrl, testUrl);
}

TEST_F(UT_SideBarEventCaller, SendShowFilePropertyDialog)
{
    bool isCalled = false;
    QList<QUrl> capturedUrls;

    stub_ext::StubExt stub;

    typedef QVariant (EventChannelManager::*PushFunc)(const QString &, const QString &,
                                                       QList<QUrl>, QVariantHash &&);
    stub.set_lamda(static_cast<PushFunc>(&EventChannelManager::push),
        [&isCalled, &capturedUrls](EventChannelManager *, const QString &space,
                                     const QString &topic, QList<QUrl> urls, QVariantHash) -> QVariant {
        __DBG_STUB_INVOKE__
        if (space == "dfmplugin_propertydialog" && topic == "slot_PropertyDialog_Show") {
            isCalled = true;
            capturedUrls = urls;
        }
        return QVariant();
    });

    QUrl testUrl = QUrl::fromLocalFile("/home/properties");
    SideBarEventCaller::sendShowFilePropertyDialog(testUrl);

    EXPECT_TRUE(isCalled);
    EXPECT_EQ(capturedUrls.size(), 1);
    EXPECT_EQ(capturedUrls[0], testUrl);
}

TEST_F(UT_SideBarEventCaller, SendCheckTabAddable_True)
{
    stub_ext::StubExt stub;

    typedef QVariant (EventChannelManager::*PushFunc)(const QString &, const QString &, quint64);
    stub.set_lamda(static_cast<PushFunc>(&EventChannelManager::push),
        [](EventChannelManager *, const QString &, const QString &, quint64) -> QVariant {
        __DBG_STUB_INVOKE__
        return QVariant(true);
    });

    bool result = SideBarEventCaller::sendCheckTabAddable(99999);
    EXPECT_TRUE(result);
}

TEST_F(UT_SideBarEventCaller, SendCheckTabAddable_False)
{
    stub_ext::StubExt stub;

    typedef QVariant (EventChannelManager::*PushFunc)(const QString &, const QString &, quint64);
    stub.set_lamda(static_cast<PushFunc>(&EventChannelManager::push),
        [](EventChannelManager *, const QString &, const QString &, quint64) -> QVariant {
        __DBG_STUB_INVOKE__
        return QVariant(false);
    });

    bool result = SideBarEventCaller::sendCheckTabAddable(88888);
    EXPECT_FALSE(result);
}
