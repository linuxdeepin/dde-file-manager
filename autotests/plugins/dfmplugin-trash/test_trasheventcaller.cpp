// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QList>
#include <QVariantHash>

#include "events/trasheventcaller.h"
#include "dfmplugin_trash_global.h"
#include "utils/trashhelper.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/widgets/filemanagerwindowsmanager.h"

#include <dfm-framework/dpf.h>

#include <stubext.h>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DPTRASH_USE_NAMESPACE
DPF_USE_NAMESPACE

using namespace dfmplugin_trash;

class TestTrashEventCaller : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

// Test sendOpenWindow method
TEST_F(TestTrashEventCaller, SendOpenWindow_ValidUrl_CallsSuccessfully)
{
    QUrl testUrl("trash:///test");
    bool signalPublished = false;

    // Mock dpfSignalDispatcher->publish using the correct template signature
    typedef bool (EventDispatcherManager::*Publish)(dpf::EventType, const QUrl &);
    auto publish = static_cast<Publish>(&EventDispatcherManager::publish);
    stub.set_lamda(publish, [&](EventDispatcherManager *, dpf::EventType type, const QUrl &url) -> bool {
        __DBG_STUB_INVOKE__
        signalPublished = true;
        EXPECT_EQ(type, GlobalEventType::kOpenNewWindow);
        EXPECT_EQ(url, testUrl);
        return true;
    });

    // Call the method - this will execute the actual implementation
    TrashEventCaller::sendOpenWindow(testUrl);

    // Verify the mock was called
    EXPECT_TRUE(signalPublished);
}

// Test sendOpenTab method
TEST_F(TestTrashEventCaller, SendOpenTab_ValidParameters_CallsSuccessfully)
{
    quint64 testWinId = 12345;
    QUrl testUrl("trash:///test");
    bool signalPublished = false;

    // Mock dpfSignalDispatcher->publish using the correct template signature
    typedef bool (EventDispatcherManager::*Publish)(dpf::EventType, quint64, const QUrl &);
    auto publish = static_cast<Publish>(&EventDispatcherManager::publish);
    stub.set_lamda(publish, [&](EventDispatcherManager *, dpf::EventType type, quint64 winId, const QUrl &url) -> bool {
        __DBG_STUB_INVOKE__
        signalPublished = true;
        EXPECT_EQ(type, GlobalEventType::kOpenNewTab);
        EXPECT_EQ(winId, testWinId);
        EXPECT_EQ(url, testUrl);
        return true;
    });

    // Call the method - this will execute the actual implementation
    TrashEventCaller::sendOpenTab(testWinId, testUrl);

    // Verify the mock was called
    EXPECT_TRUE(signalPublished);
}

// Test sendOpenFiles method
TEST_F(TestTrashEventCaller, SendOpenFiles_ValidParameters_CallsSuccessfully)
{
    quint64 testWinId = 12345;
    QList<QUrl> testUrls = { QUrl("trash:///test1"), QUrl("trash:///test2") };
    bool signalPublished = false;

    // Mock dpfSignalDispatcher->publish using the correct template signature
    typedef bool (EventDispatcherManager::*Publish)(dpf::EventType, quint64, const QList<QUrl> &);
    auto publish = static_cast<Publish>(&EventDispatcherManager::publish);
    stub.set_lamda(publish, [&](EventDispatcherManager *, dpf::EventType type, quint64 winId, const QList<QUrl> &urls) -> bool {
        __DBG_STUB_INVOKE__
        signalPublished = true;
        EXPECT_EQ(type, GlobalEventType::kOpenFiles);
        EXPECT_EQ(winId, testWinId);
        EXPECT_EQ(urls, testUrls);
        return true;
    });

    // Call the method - this will execute the actual implementation
    TrashEventCaller::sendOpenFiles(testWinId, testUrls);

    // Verify the mock was called
    EXPECT_TRUE(signalPublished);
}

// Test sendEmptyTrash method
TEST_F(TestTrashEventCaller, SendEmptyTrash_ValidParameters_CallsSuccessfully)
{
    quint64 testWinId = 12345;
    QList<QUrl> testUrls = { QUrl("trash:///test1"), QUrl("trash:///test2") };
    bool signalPublished = false;

    // Mock dpfSignalDispatcher->publish using the correct template signature
    typedef bool (EventDispatcherManager::*Publish)(dpf::EventType, quint64, const QList<QUrl> &);
    auto publish = static_cast<Publish>(&EventDispatcherManager::publish);
    stub.set_lamda(publish, [&](EventDispatcherManager *, dpf::EventType type, quint64 winId, const QList<QUrl> &urls) -> bool {
        __DBG_STUB_INVOKE__
        signalPublished = true;
        EXPECT_EQ(type, GlobalEventType::kCleanTrash);
        EXPECT_EQ(winId, testWinId);
        EXPECT_EQ(urls, testUrls);
        // EXPECT_EQ(noticeType, AbstractJobHandler::DeleteDialogNoticeType::kEmptyTrash);
        // EXPECT_EQ(properties, nullptr);
        return true;
    });

    // Call the method - this will execute the actual implementation
    TrashEventCaller::sendEmptyTrash(testWinId, testUrls);

    // Verify the mock was called
    EXPECT_TRUE(signalPublished);
}

// Test sendTrashPropertyDialog method
TEST_F(TestTrashEventCaller, SendTrashPropertyDialog_ValidUrl_CallsSuccessfully)
{
    QUrl testUrl("trash:///test");
    bool slotPushed = false;

    // Mock dpfSlotChannel->push using the correct template signature
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, const QList<QUrl> &, const QVariantHash &);
    auto push = static_cast<Push>(&EventChannelManager::push);
    stub.set_lamda(push, [&](EventChannelManager *, const QString &space, const QString &topic, const QList<QUrl> &urls, const QVariantHash &properties) -> QVariant {
        __DBG_STUB_INVOKE__
        slotPushed = true;
        EXPECT_EQ(space, QString("dfmplugin_propertydialog"));
        EXPECT_EQ(topic, QString("slot_PropertyDialog_Show"));
        EXPECT_EQ(urls.size(), 1);
        EXPECT_EQ(urls.first(), testUrl);
        EXPECT_TRUE(properties.isEmpty());
        return QVariant();
    });

    // Call the method - this will execute the actual implementation
    TrashEventCaller::sendTrashPropertyDialog(testUrl);

    // Verify the mock was called
    EXPECT_TRUE(slotPushed);
}

// Test sendShowEmptyTrash method
TEST_F(TestTrashEventCaller, SendShowEmptyTrash_ValidParameters_CallsSuccessfully)
{
    quint64 testWinId = 12345;
    bool visible = true;
    bool slotPushed = false;

    // Mock dpfSlotChannel->push using the correct template signature
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, const QString &);
    auto push = static_cast<Push>(&EventChannelManager::push);
    stub.set_lamda(push, [&](EventChannelManager *, const QString &space, const QString &topic, const QString &scheme) -> bool {
        __DBG_STUB_INVOKE__
        slotPushed = true;
        EXPECT_EQ(space, QString("dfmplugin_workspace"));
        EXPECT_EQ(topic, QString("slot_ShowCustomTopWidget"));
        // EXPECT_EQ(winId, testWinId);
        EXPECT_EQ(scheme, TrashHelper::scheme());
        // EXPECT_EQ(show, visible);
        return true;
    });

    // Call the method - this will execute the actual implementation
    TrashEventCaller::sendShowEmptyTrash(testWinId, visible);

    // Verify the mock was called
    EXPECT_TRUE(slotPushed);
}

// Test sendCheckTabAddable method with true result
TEST_F(TestTrashEventCaller, SendCheckTabAddable_ValidWindowId_ReturnsExpectedResult)
{
    quint64 testWinId = 12345;
    bool expectedResult = true;
    bool slotPushed = false;

    // Mock dpfSlotChannel->push using the correct template signature
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, quint64);
    auto push = static_cast<Push>(&EventChannelManager::push);
    stub.set_lamda(push, [&](EventChannelManager *, const QString &space, const QString &topic, quint64 winId) -> QVariant {
        __DBG_STUB_INVOKE__
        slotPushed = true;
        EXPECT_EQ(space, QString("dfmplugin_titlebar"));
        EXPECT_EQ(topic, QString("slot_Tab_Addable"));
        EXPECT_EQ(winId, testWinId);
        return QVariant(expectedResult);
    });

    // Call the method - this will execute the actual implementation
    bool result = TrashEventCaller::sendCheckTabAddable(testWinId);

    // Verify the result and that the mock was called
    EXPECT_EQ(result, expectedResult);
    EXPECT_TRUE(slotPushed);
}

// Test sendCheckTabAddable method with false result
TEST_F(TestTrashEventCaller, SendCheckTabAddable_FalseResult_ReturnsFalse)
{
    quint64 testWinId = 12345;
    bool expectedResult = false;

    // Mock dpfSlotChannel->push using the correct template signature
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, quint64);
    auto push = static_cast<Push>(&EventChannelManager::push);
    stub.set_lamda(push, [expectedResult](EventChannelManager *, const QString &space, const QString &topic, quint64 winId) -> QVariant {
        __DBG_STUB_INVOKE__
        return QVariant(expectedResult);
    });

    // Call the method - this will execute the actual implementation
    bool result = TrashEventCaller::sendCheckTabAddable(testWinId);

    // Verify the result
    EXPECT_EQ(result, expectedResult);
}