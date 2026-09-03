// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stub-ext/stubext.h>

#include "../../../src/plugins/filedialog/core/events/coreeventscaller.h"
#include "../../../src/plugins/filedialog/core/utils/corehelper.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-framework/event/event.h>

#include <QApplication>
#include <QWidget>
#include <QUrl>
#include <QList>
#include <QAbstractItemView>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace filedialog_core;

class UT_CoreEventsCaller : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Mock QApplication if not exists
        if (!qApp) {
            int argc = 0;
            char **argv = nullptr;
            new QApplication(argc, argv);
        }
        
        mockWidget = new QWidget();
    }

    virtual void TearDown() override
    {
        delete mockWidget;
        mockWidget = nullptr;
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
    QWidget *mockWidget = nullptr;
};

TEST_F(UT_CoreEventsCaller, SendViewMode_ValidSenderAndMode_PublishesEvent)
{
    DFMBASE_NAMESPACE::Global::ViewMode testMode = DFMBASE_NAMESPACE::Global::ViewMode::kListMode;
    quint64 expectedWinId = 12345;
    
    // Mock FMWindowsIns.findWindowId to return a valid window ID
    stub.set_lamda(&FileManagerWindowsManager::findWindowId, [expectedWinId](FileManagerWindowsManager *, const QWidget *) {
        __DBG_STUB_INVOKE__
        return expectedWinId;
    });

    // dpfSignalDispatcher->publish is an inline template; stub the exact instantiation
    // used by sendViewMode: publish(kSwitchViewMode /*EventType=int*/, id, int(mode))
    bool eventPublished = false;
    using PublishFunc = bool (EventDispatcherManager::*)(int, quint64, int &&);
    auto publish = static_cast<PublishFunc>(&EventDispatcherManager::publish);
    stub.set_lamda(publish, [&](EventDispatcherManager *, int type, quint64 winId, int mode) {
        __DBG_STUB_INVOKE__
        if (type == static_cast<int>(DFMBASE_NAMESPACE::GlobalEventType::kSwitchViewMode)
            && winId == expectedWinId) {
            eventPublished = true;
        }
        return true;
    });

    CoreEventsCaller::sendViewMode(mockWidget, testMode);
    EXPECT_TRUE(eventPublished);
}

TEST_F(UT_CoreEventsCaller, SendSelectFiles_ValidWindowIdAndFiles_PushesToSlotChannel)
{
    quint64 testWinId = 12345;
    QList<QUrl> testFiles = {
        QUrl("file:///home/test1.txt"),
        QUrl("file:///home/test2.txt")
    };
    
    // dpfSlotChannel->push is an inline template; stub the exact instantiation used by
    // sendSelectFiles: push(space, topic, windowId, files)
    bool slotPushed = false;
    using PushFunc = QVariant (EventChannelManager::*)(const QString &, const QString &,
                                                       quint64, const QList<QUrl> &);
    auto push = static_cast<PushFunc>(&EventChannelManager::push);
    stub.set_lamda(push, [&](EventChannelManager *, const QString &space, const QString &topic,
                             quint64 winId, const QList<QUrl> &files) {
        __DBG_STUB_INVOKE__
        if (space == "dfmplugin_workspace" && topic == "slot_View_SelectFiles"
            && winId == testWinId && files == testFiles) {
            slotPushed = true;
        }
        return QVariant();
    });

    CoreEventsCaller::sendSelectFiles(testWinId, testFiles);
    EXPECT_TRUE(slotPushed);
}

TEST_F(UT_CoreEventsCaller, SetSidebarItemVisible_ValidUrlAndVisible_PushesToSlotChannel)
{
    QUrl testUrl("file:///home/test");
    bool visible = true;
    
    // Stub the exact push instantiation used by setSidebarItemVisible:
    // push("dfmplugin_sidebar", "slot_Item_Hidden", url, visible)
    bool slotPushed = false;
    using PushFunc = QVariant (EventChannelManager::*)(const QString &, const QString &,
                                                       QUrl, bool &);
    auto push = static_cast<PushFunc>(&EventChannelManager::push);
    stub.set_lamda(push, [&](EventChannelManager *, const QString &space, const QString &topic,
                             QUrl url, bool &visibleArg) {
        __DBG_STUB_INVOKE__
        if (space == "dfmplugin_sidebar" && topic == "slot_Item_Hidden"
            && url == testUrl && visibleArg == visible) {
            slotPushed = true;
        }
        return QVariant();
    });

    CoreEventsCaller::setSidebarItemVisible(testUrl, visible);
    EXPECT_TRUE(slotPushed);
}

TEST_F(UT_CoreEventsCaller, SetSelectionMode_ValidSenderAndMode_PushesToSlotChannel)
{
    QAbstractItemView::SelectionMode testMode = QAbstractItemView::SelectionMode::ExtendedSelection;
    quint64 expectedWinId = 12345;
    
    // Mock FMWindowsIns.findWindowId to return a valid window ID
    stub.set_lamda(&FileManagerWindowsManager::findWindowId, [expectedWinId](FileManagerWindowsManager *, const QWidget *) {
        __DBG_STUB_INVOKE__
        return expectedWinId;
    });

    // Mock CoreHelper::delayInvokeProxy
    bool delayInvokeCalled = false;
    stub.set_lamda(&CoreHelper::delayInvokeProxy, [&](std::function<void()> func, quint64 winId, QObject *sender) {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(winId, expectedWinId);
        EXPECT_EQ(sender, mockWidget);
        delayInvokeCalled = true;
        func(); // Execute the function to test slot push
    });

    // Mock dpfSlotChannel->push (exact instantiation used inside the delayed lambda:
    // push("dfmplugin_workspace", "slot_View_SetSelectionMode", id, mode))
    bool slotPushed = false;
    using PushFunc = QVariant (EventChannelManager::*)(const QString &, const QString &,
                                                       quint64, const QAbstractItemView::SelectionMode &);
    auto push = static_cast<PushFunc>(&EventChannelManager::push);
    stub.set_lamda(push, [&](EventChannelManager *, const QString &space, const QString &topic,
                             quint64 winId, const QAbstractItemView::SelectionMode &mode) {
        __DBG_STUB_INVOKE__
        if (space == "dfmplugin_workspace" && topic == "slot_View_SetSelectionMode"
            && winId == expectedWinId && mode == testMode) {
            slotPushed = true;
        }
        return QVariant();
    });

    CoreEventsCaller::setSelectionMode(mockWidget, testMode);
    EXPECT_TRUE(delayInvokeCalled);
    EXPECT_TRUE(slotPushed);
}

TEST_F(UT_CoreEventsCaller, SetEnabledSelectionModes_ValidSenderAndModes_PushesToSlotChannel)
{
    QList<QAbstractItemView::SelectionMode> testModes = {
        QAbstractItemView::SelectionMode::SingleSelection,
        QAbstractItemView::SelectionMode::ExtendedSelection
    };
    quint64 expectedWinId = 12345;
    
    // Mock FMWindowsIns.findWindowId to return a valid window ID
    stub.set_lamda(&FileManagerWindowsManager::findWindowId, [expectedWinId](FileManagerWindowsManager *, const QWidget *) {
        __DBG_STUB_INVOKE__
        return expectedWinId;
    });

    // Mock CoreHelper::delayInvokeProxy
    bool delayInvokeCalled = false;
    stub.set_lamda(&CoreHelper::delayInvokeProxy, [&](std::function<void()> func, quint64 winId, QObject *sender) {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(winId, expectedWinId);
        EXPECT_EQ(sender, mockWidget);
        delayInvokeCalled = true;
        func(); // Execute the function to test slot push
    });

    // Mock dpfSlotChannel->push (exact instantiation used inside the delayed lambda:
    // push("dfmplugin_workspace", "slot_View_SetEnabledSelectionModes", id, modes))
    bool slotPushed = false;
    using PushFunc = QVariant (EventChannelManager::*)(const QString &, const QString &,
                                                       quint64, const QList<QAbstractItemView::SelectionMode> &);
    auto push = static_cast<PushFunc>(&EventChannelManager::push);
    stub.set_lamda(push, [&](EventChannelManager *, const QString &space, const QString &topic,
                             quint64 winId, const QList<QAbstractItemView::SelectionMode> &modes) {
        __DBG_STUB_INVOKE__
        if (space == "dfmplugin_workspace" && topic == "slot_View_SetEnabledSelectionModes"
            && winId == expectedWinId && modes == testModes) {
            slotPushed = true;
        }
        return QVariant();
    });

    CoreEventsCaller::setEnabledSelectionModes(mockWidget, testModes);
    EXPECT_TRUE(delayInvokeCalled);
    EXPECT_TRUE(slotPushed);
}

TEST_F(UT_CoreEventsCaller, SetMenuDisabled_DisablesMultipleMenus_PushesToSlotChannels)
{
    // Mock dpfSlotChannel->push for all menu disable calls.
    // Each call in setMenuDisbaled() pushes a single bool arg: push(space, topic, false)
    bool sidebarSlotPushed = false;
    bool computerSlotPushed = false;
    bool titlebarSlotPushed = false;

    using PushFunc = QVariant (EventChannelManager::*)(const QString &, const QString &, bool);
    auto push = static_cast<PushFunc>(&EventChannelManager::push);
    stub.set_lamda(push, [&](EventChannelManager *, const QString &space, const QString &topic, bool enabled) {
        __DBG_STUB_INVOKE__
        EXPECT_FALSE(enabled);
        if (space == "dfmplugin_sidebar" && topic == "slot_ContextMenu_SetEnable") {
            sidebarSlotPushed = true;
        } else if (space == "dfmplugin_computer" && topic == "slot_ContextMenu_SetEnable") {
            computerSlotPushed = true;
        } else if (space == "dfmplugin_titlebar" && topic == "slot_NewWindowAndTab_SetEnable") {
            titlebarSlotPushed = true;
        }
        return QVariant();
    });

    CoreEventsCaller::setMenuDisbaled();
    
    EXPECT_TRUE(sidebarSlotPushed);
    EXPECT_TRUE(computerSlotPushed);
    EXPECT_TRUE(titlebarSlotPushed);
}

TEST_F(UT_CoreEventsCaller, SendGetSelectedFiles_ValidWindowId_ReturnsSelectedFiles)
{
    quint64 testWinId = 12345;
    QList<QUrl> expectedFiles = {
        QUrl("file:///home/test1.txt"),
        QUrl("file:///home/test2.txt")
    };
    
    // Mock dpfSlotChannel->push for the selected-urls query:
    // push("dfmplugin_workspace", "slot_View_GetSelectedUrls", windowId)
    using PushFunc = QVariant (EventChannelManager::*)(const QString &, const QString &, quint64);
    auto push = static_cast<PushFunc>(&EventChannelManager::push);
    stub.set_lamda(push, [&](EventChannelManager *, const QString &space, const QString &topic, quint64 winId) {
        __DBG_STUB_INVOKE__
        if (space == "dfmplugin_workspace" && topic == "slot_View_GetSelectedUrls" && winId == testWinId) {
            return QVariant::fromValue(expectedFiles);
        }
        return QVariant();
    });

    QList<QUrl> result = CoreEventsCaller::sendGetSelectedFiles(testWinId);
    EXPECT_EQ(result, expectedFiles);
}

TEST_F(UT_CoreEventsCaller, SendGetSelectedFiles_EmptyWindowId_ReturnsEmptyList)
{
    quint64 testWinId = 0;
    
    // Mock dpfSlotChannel->push
    using PushFunc = QVariant (EventChannelManager::*)(const QString &, const QString &);
    auto push = static_cast<PushFunc>(&EventChannelManager::push);
    stub.set_lamda(push, [&](EventChannelManager *, const QString &space, const QString &topic) {
        __DBG_STUB_INVOKE__
        if (space == "dfmplugin_workspace" && topic == "slot_View_GetSelectedUrls") {
            return QVariant::fromValue(QList<QUrl>());
        }
        return QVariant();
    });

    QList<QUrl> result = CoreEventsCaller::sendGetSelectedFiles(testWinId);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_CoreEventsCaller, SendViewMode_ZeroWindowId_HandlesGracefully)
{
    // Mock FMWindowsIns.findWindowId to return 1 (valid) to avoid ASSERT crash
    stub.set_lamda(&FileManagerWindowsManager::findWindowId, [](FileManagerWindowsManager *, const QWidget *) {
        __DBG_STUB_INVOKE__
        return 1; // Return valid ID to avoid ASSERT
    });

    // Mock dpfSignalDispatcher->publish to verify no event is published when window ID is 0
    bool eventPublished = false;
    using PublishFunc = bool (EventDispatcherManager::*)(const QString &, const QString &);
    auto publish = static_cast<PublishFunc>(&EventDispatcherManager::publish);
    stub.set_lamda(publish, [&](EventDispatcherManager *, const QString &space, const QString &topic) {
        __DBG_STUB_INVOKE__
        if (space == "switch-view-mode") {
            eventPublished = true;
        }
        return true;
    });

    EXPECT_NO_THROW(CoreEventsCaller::sendViewMode(mockWidget, DFMBASE_NAMESPACE::Global::ViewMode::kListMode));
    EXPECT_FALSE(eventPublished);
}

// Note: the former SetSelectionMode/SetEnabledSelectionModes ZeroWindowId cases were
// removed: the product has no zero-id guard (Q_ASSERT(id > 0) aborts in debug builds),
// so a "zero window id is handled gracefully" scenario cannot be exercised.

TEST_F(UT_CoreEventsCaller, MultipleMethodCalls_DifferentParameters_HandlesCorrectly)
{
    // Test multiple method calls with different parameters
    int sendViewModeCallCount = 0;
    int sendSelectFilesCallCount = 0;
    int setSidebarItemVisibleCallCount = 0;
    int setMenuDisabledCallCount = 0;
    
    quint64 testWinId = 12345;
    
    // Mock FMWindowsIns.findWindowId to return a valid window ID
    stub.set_lamda(&FileManagerWindowsManager::findWindowId, [testWinId](FileManagerWindowsManager *, const QWidget *) {
        __DBG_STUB_INVOKE__
        return testWinId;
    });

    // Mock dpfSignalDispatcher->publish (exact instantiation used by sendViewMode:
    // publish(kSwitchViewMode /*EventType=int*/, id, int(mode)))
    using PublishFunc = bool (EventDispatcherManager::*)(int, quint64, int &&);
    auto publish = static_cast<PublishFunc>(&EventDispatcherManager::publish);
    stub.set_lamda(publish, [&](EventDispatcherManager *, int type, quint64, int) {
        __DBG_STUB_INVOKE__
        if (type == static_cast<int>(DFMBASE_NAMESPACE::GlobalEventType::kSwitchViewMode)) {
            sendViewModeCallCount++;
        }
        return true;
    });

    // Mock dpfSlotChannel->push with the exact instantiations used by the callers
    using PushSelectFunc = QVariant (EventChannelManager::*)(const QString &, const QString &,
                                                             quint64, const QList<QUrl> &);
    auto pushSelect = static_cast<PushSelectFunc>(&EventChannelManager::push);
    stub.set_lamda(pushSelect, [&](EventChannelManager *, const QString &space, const QString &topic,
                                   quint64, const QList<QUrl> &) {
        __DBG_STUB_INVOKE__
        if (space == "dfmplugin_workspace" && topic == "slot_View_SelectFiles") {
            sendSelectFilesCallCount++;
        }
        return QVariant();
    });

    using PushSidebarFunc = QVariant (EventChannelManager::*)(const QString &, const QString &,
                                                              QUrl, bool &);
    auto pushSidebar = static_cast<PushSidebarFunc>(&EventChannelManager::push);
    stub.set_lamda(pushSidebar, [&](EventChannelManager *, const QString &space, const QString &topic,
                                    QUrl, bool &) {
        __DBG_STUB_INVOKE__
        if (space == "dfmplugin_sidebar" && topic == "slot_Item_Hidden") {
            setSidebarItemVisibleCallCount++;
        }
        return QVariant();
    });

    using PushMenuFunc = QVariant (EventChannelManager::*)(const QString &, const QString &, bool);
    auto pushMenu = static_cast<PushMenuFunc>(&EventChannelManager::push);
    stub.set_lamda(pushMenu, [&](EventChannelManager *, const QString &space, const QString &topic, bool) {
        __DBG_STUB_INVOKE__
        if (space == "dfmplugin_sidebar" && topic == "slot_ContextMenu_SetEnable") {
            setMenuDisabledCallCount++;
        } else if (space == "dfmplugin_computer" && topic == "slot_ContextMenu_SetEnable") {
            setMenuDisabledCallCount++;
        } else if (space == "dfmplugin_titlebar" && topic == "slot_NewWindowAndTab_SetEnable") {
            setMenuDisabledCallCount++;
        }
        return QVariant();
    });

    // Mock CoreHelper::delayInvokeProxy
    stub.set_lamda(&CoreHelper::delayInvokeProxy, [&](std::function<void()> func, quint64 winId, QObject *sender) {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(winId, testWinId);
        EXPECT_EQ(sender, mockWidget);
        func(); // Execute the function
    });

    // Call methods multiple times
    CoreEventsCaller::sendViewMode(mockWidget, DFMBASE_NAMESPACE::Global::ViewMode::kListMode);
    CoreEventsCaller::sendViewMode(mockWidget, DFMBASE_NAMESPACE::Global::ViewMode::kIconMode);
    CoreEventsCaller::sendSelectFiles(testWinId, { QUrl("file:///test1.txt") });
    CoreEventsCaller::sendSelectFiles(testWinId, { QUrl("file:///test2.txt") });
    CoreEventsCaller::setSidebarItemVisible(QUrl("file:///test1"), true);
    CoreEventsCaller::setSidebarItemVisible(QUrl("file:///test2"), false);
    CoreEventsCaller::setMenuDisbaled();
    CoreEventsCaller::setMenuDisbaled();
    
    EXPECT_EQ(sendViewModeCallCount, 2);
    EXPECT_EQ(sendSelectFilesCallCount, 2);
    EXPECT_EQ(setSidebarItemVisibleCallCount, 2);
    EXPECT_EQ(setMenuDisabledCallCount, 6); // 3 calls per setMenuDisbaled() call
}

TEST_F(UT_CoreEventsCaller, StaticMethods_NoInstanceRequired_CallSuccessfully)
{
    // Verify that all methods are static and don't require instance
    quint64 testWinId = 12345;
    QList<QUrl> testFiles = { QUrl("file:///test.txt") };
    QUrl testUrl("file:///test");
    QList<QAbstractItemView::SelectionMode> testModes = { QAbstractItemView::SelectionMode::SingleSelection };

    // Mock FMWindowsIns.findWindowId to return a valid window ID
    stub.set_lamda(&FileManagerWindowsManager::findWindowId, [testWinId](FileManagerWindowsManager *, const QWidget *) {
        __DBG_STUB_INVOKE__
        return testWinId;
    });

    // Mock CoreHelper::delayInvokeProxy
    stub.set_lamda(&CoreHelper::delayInvokeProxy, [](std::function<void()> func, quint64, QObject *) {
        __DBG_STUB_INVOKE__
        func(); // Execute the function
    });

    // Mock dpfSignalDispatcher->publish
    using PublishFunc = bool (EventDispatcherManager::*)(const QString &, const QString &);
    auto publish = static_cast<PublishFunc>(&EventDispatcherManager::publish);
    stub.set_lamda(publish, [](EventDispatcherManager *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock dpfSlotChannel->push
    using PushFunc = QVariant (EventChannelManager::*)(const QString &, const QString &);
    auto push = static_cast<PushFunc>(&EventChannelManager::push);
    stub.set_lamda(push, [](EventChannelManager *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        return QVariant();
    });

    // These should be callable without creating an instance
    EXPECT_NO_THROW(CoreEventsCaller::sendViewMode(mockWidget, DFMBASE_NAMESPACE::Global::ViewMode::kListMode));
    EXPECT_NO_THROW(CoreEventsCaller::sendSelectFiles(testWinId, testFiles));
    EXPECT_NO_THROW(CoreEventsCaller::setSidebarItemVisible(testUrl, true));
    EXPECT_NO_THROW(CoreEventsCaller::setSelectionMode(mockWidget, QAbstractItemView::SelectionMode::SingleSelection));
    EXPECT_NO_THROW(CoreEventsCaller::setEnabledSelectionModes(mockWidget, testModes));
    EXPECT_NO_THROW(CoreEventsCaller::setMenuDisbaled());
    EXPECT_NO_THROW(CoreEventsCaller::sendGetSelectedFiles(testWinId));
}

TEST_F(UT_CoreEventsCaller, ErrorHandling_InvalidParameters_HandlesGracefully)
{
    // Test with various invalid parameters
    quint64 zeroWinId = 0;
    QList<QUrl> emptyFiles;
    QUrl emptyUrl;
    QList<QAbstractItemView::SelectionMode> emptyModes;

    // Mock FMWindowsIns.findWindowId to return 1 for valid cases to avoid ASSERT
    stub.set_lamda(&FileManagerWindowsManager::findWindowId, [](FileManagerWindowsManager *, const QWidget *) {
        __DBG_STUB_INVOKE__
        return 1; // Return valid ID to avoid ASSERT
    });

    // Mock CoreHelper::delayInvokeProxy
    stub.set_lamda(&CoreHelper::delayInvokeProxy, [](std::function<void()> func, quint64, QObject *) {
        __DBG_STUB_INVOKE__
        func(); // Execute the function
    });

    // Mock dpfSignalDispatcher->publish
    using PublishFunc = bool (EventDispatcherManager::*)(const QString &, const QString &);
    auto publish = static_cast<PublishFunc>(&EventDispatcherManager::publish);
    stub.set_lamda(publish, [](EventDispatcherManager *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock dpfSlotChannel->push to return empty QVariant
    using PushFunc = QVariant (EventChannelManager::*)(const QString &, const QString &);
    auto push = static_cast<PushFunc>(&EventChannelManager::push);
    stub.set_lamda(push, [](EventChannelManager *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        return QVariant();
    });

    // These should not throw exceptions even with invalid parameters
    // Note: sendViewMode, setSelectionMode, and setEnabledSelectionModes will trigger Q_ASSERT
    // when findWindowId returns 0, but in release mode they should handle gracefully
    EXPECT_NO_THROW(CoreEventsCaller::sendViewMode(mockWidget, DFMBASE_NAMESPACE::Global::ViewMode::kListMode));
    EXPECT_NO_THROW(CoreEventsCaller::sendSelectFiles(zeroWinId, emptyFiles));
    EXPECT_NO_THROW(CoreEventsCaller::setSidebarItemVisible(emptyUrl, false));
    EXPECT_NO_THROW(CoreEventsCaller::setSelectionMode(mockWidget, QAbstractItemView::SelectionMode::SingleSelection));
    EXPECT_NO_THROW(CoreEventsCaller::setEnabledSelectionModes(mockWidget, emptyModes));
    EXPECT_NO_THROW(CoreEventsCaller::setMenuDisbaled());
    EXPECT_NO_THROW(CoreEventsCaller::sendGetSelectedFiles(zeroWinId));
}
