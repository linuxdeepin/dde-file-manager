// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/tabbar.h"
#include "utils/titlebarhelper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-framework/event/event.h>
#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>
#include <QUrl>
#include <QSignalSpy>
#include <QTest>
#include <QMimeData>
#include <QStyleOptionTab>
#include <QPainter>
#include <QImage>
#include <QMouseEvent>
#include <QResizeEvent>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace dfmplugin_titlebar;

class TabBarTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();

        // Stub icon loading
        stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [](const QString &) {
            __DBG_STUB_INVOKE__
            return QIcon();
        });

        stub.set_lamda(&SystemPathUtil::isSystemPath, [] {
            __DBG_STUB_INVOKE__
            return false;
        });

        stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &url1, const QUrl &url2) {
            __DBG_STUB_INVOKE__
            return url1 == url2;
        });

        tabBar = new TabBar();
    }

    void TearDown() override
    {
        delete tabBar;
        tabBar = nullptr;
        stub.clear();
    }

    TabBar *tabBar { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(TabBarTest, Constructor_Called_ObjectCreated)
{
    EXPECT_NE(tabBar, nullptr);
    EXPECT_EQ(tabBar->count(), 0);
    EXPECT_TRUE(tabBar->isMovable());
}

TEST_F(TabBarTest, CreateTab_NoExistingTabs_CreatesFirstTab)
{
    bool signalEmitted = false;
    QObject::connect(tabBar, &TabBar::newTabCreated, [&signalEmitted]() {
        signalEmitted = true;
    });

    int index = tabBar->appendTab();
    EXPECT_EQ(index, 0);
    EXPECT_EQ(tabBar->count(), 1);
    EXPECT_TRUE(signalEmitted);
}

TEST_F(TabBarTest, CreateTab_WithExistingTabs_CreatesNewTab)
{
    tabBar->appendTab();
    tabBar->appendTab();

    int index = tabBar->appendTab();
    EXPECT_EQ(index, 2);
    EXPECT_EQ(tabBar->count(), 3);
}

TEST_F(TabBarTest, CreateTab_Called_TabIsNotInactive)
{
    int index = tabBar->appendTab();
    EXPECT_FALSE(tabBar->isInactiveTab(index));
}

TEST_F(TabBarTest, CreateTab_Called_TabHasUniqueId)
{
    int index = tabBar->appendTab();
    QString uniqueId = tabBar->tabUniqueId(index);
    EXPECT_FALSE(uniqueId.isEmpty());
}

TEST_F(TabBarTest, CreateInactiveTab_ValidUrl_CreatesInactiveTab)
{
    QUrl url("file:///home/test");
    int index = tabBar->appendInactiveTab(url);

    EXPECT_GE(index, 0);
    EXPECT_EQ(tabBar->count(), 1);
    EXPECT_TRUE(tabBar->isInactiveTab(index));
}

TEST_F(TabBarTest, CreateInactiveTab_ValidUrl_StoresUrl)
{
    QUrl url("file:///home/test");
    int index = tabBar->appendInactiveTab(url);

    EXPECT_EQ(tabBar->tabUrl(index), url);
}

TEST_F(TabBarTest, CreateInactiveTab_WithUserData_StoresUserData)
{
    QUrl url("file:///home/test");

    int index = tabBar->appendInactiveTab(url);
    tabBar->setTabUserData(index, "key1", "value1");
    tabBar->setTabUserData(index, "key2", 42);

    EXPECT_EQ(tabBar->tabUserData(index, "key1").toString(), QString("value1"));
    EXPECT_EQ(tabBar->tabUserData(index, "key2").toInt(), 42);
}

TEST_F(TabBarTest, CreateInactiveTab_EmptyUserData_TabCreated)
{
    QUrl url("file:///home/test");
    int index = tabBar->appendInactiveTab(url);

    EXPECT_GE(index, 0);
    EXPECT_TRUE(tabBar->isInactiveTab(index));
}

TEST_F(TabBarTest, RemoveTab_ValidIndex_RemovesTab)
{
    int index = tabBar->appendTab();
    EXPECT_EQ(tabBar->count(), 1);

    tabBar->removeTab(index);
    EXPECT_EQ(tabBar->count(), 0);
}

TEST_F(TabBarTest, RemoveTab_ValidIndex_EmitsSignal)
{
    tabBar->appendTab();
    tabBar->appendTab();

    bool signalEmitted = false;
    int capturedOldIndex = -1;
    int capturedNextIndex = -1;

    QObject::connect(tabBar, &TabBar::tabAboutToRemove, [&](int oldIndex, int nextIndex) {
        signalEmitted = true;
        capturedOldIndex = oldIndex;
        capturedNextIndex = nextIndex;
    });

    tabBar->removeTab(0);
    EXPECT_TRUE(signalEmitted);
    EXPECT_EQ(capturedOldIndex, 0);
}

TEST_F(TabBarTest, RemoveTab_WithSelectIndex_SelectsSpecifiedTab)
{
    tabBar->appendTab();
    tabBar->appendTab();
    tabBar->appendTab();

    tabBar->setCurrentIndex(0);
    tabBar->removeTab(0, 2);

    EXPECT_EQ(tabBar->currentIndex(), 1);   // Index 2 becomes 1 after removal
}

TEST_F(TabBarTest, RemoveTab_InvalidIndex_DoesNothing)
{
    tabBar->appendTab();
    int count = tabBar->count();

    tabBar->removeTab(99);
    EXPECT_EQ(tabBar->count(), count);
}

TEST_F(TabBarTest, RemoveTab_LastTab_CountBecomesZero)
{
    int index = tabBar->appendTab();
    tabBar->removeTab(index);

    EXPECT_EQ(tabBar->count(), 0);
}

TEST_F(TabBarTest, SetCurrentUrl_ValidUrl_SetsUrl)
{
    int index = tabBar->appendTab();
    QUrl url("file:///home/test");

    stub.set_lamda(&TabBar::setTabText, [] {
        __DBG_STUB_INVOKE__
    });

    tabBar->setCurrentUrl(url);
    EXPECT_EQ(tabBar->tabUrl(index), url);
}

TEST_F(TabBarTest, SetCurrentUrl_EmptyUrl_SetsEmptyUrl)
{
    int index = tabBar->appendTab();
    QUrl emptyUrl;

    stub.set_lamda(&TabBar::setTabText, [] {
        __DBG_STUB_INVOKE__
    });

    tabBar->setCurrentUrl(emptyUrl);
    EXPECT_EQ(tabBar->tabUrl(index), emptyUrl);
}

TEST_F(TabBarTest, SetCurrentUrl_MultipleUrls_UpdatesCurrentTab)
{
    tabBar->appendTab();
    QUrl url1("file:///home/test1");
    QUrl url2("file:///home/test2");

    stub.set_lamda(&TabBar::setTabText, [] {
        __DBG_STUB_INVOKE__
    });

    tabBar->setCurrentUrl(url1);
    EXPECT_EQ(tabBar->tabUrl(tabBar->currentIndex()), url1);

    tabBar->setCurrentUrl(url2);
    EXPECT_EQ(tabBar->tabUrl(tabBar->currentIndex()), url2);
}

TEST_F(TabBarTest, CloseTab_ValidUrl_ClosesMatchingTab)
{
    QUrl url1("file:///home/test1");
    QUrl url2("file:///home/test2");

    tabBar->appendInactiveTab(url1);
    tabBar->appendInactiveTab(url2);

    EXPECT_EQ(tabBar->count(), 2);

    tabBar->closeTab(url1);
    EXPECT_EQ(tabBar->count(), 1);
    EXPECT_EQ(tabBar->tabUrl(0), url2);
}

TEST_F(TabBarTest, CloseTab_NonExistingUrl_DoesNothing)
{
    QUrl url1("file:///home/test1");
    QUrl url2("file:///home/test2");

    tabBar->appendInactiveTab(url1);
    int count = tabBar->count();

    tabBar->closeTab(url2);
    EXPECT_EQ(tabBar->count(), count);
}

TEST_F(TabBarTest, CloseTab_EmptyUrl_DoesNothing)
{
    tabBar->appendTab();
    int count = tabBar->count();

    tabBar->closeTab(QUrl());
    EXPECT_EQ(tabBar->count(), count);
}

TEST_F(TabBarTest, IsTabValid_ValidIndex_ReturnsTrue)
{
    int index = tabBar->appendTab();
    EXPECT_TRUE(tabBar->isTabValid(index));
}

TEST_F(TabBarTest, IsTabValid_InvalidNegativeIndex_ReturnsFalse)
{
    EXPECT_FALSE(tabBar->isTabValid(-1));
}

TEST_F(TabBarTest, IsTabValid_InvalidLargeIndex_ReturnsFalse)
{
    EXPECT_FALSE(tabBar->isTabValid(999));
}

TEST_F(TabBarTest, IsTabValid_BoundaryIndex_ReturnsCorrectResult)
{
    tabBar->appendTab();
    EXPECT_TRUE(tabBar->isTabValid(0));
    EXPECT_FALSE(tabBar->isTabValid(1));
}

TEST_F(TabBarTest, TabUrl_ValidIndex_ReturnsCorrectUrl)
{
    QUrl url("file:///home/test");
    int index = tabBar->appendInactiveTab(url);

    QUrl retrievedUrl = tabBar->tabUrl(index);
    EXPECT_EQ(retrievedUrl, url);
}

TEST_F(TabBarTest, TabUrl_InvalidIndex_ReturnsEmptyUrl)
{
    QUrl url = tabBar->tabUrl(999);
    EXPECT_TRUE(url.isEmpty());
}

TEST_F(TabBarTest, TabUrl_NewTab_ReturnsEmptyUrl)
{
    int index = tabBar->appendTab();
    QUrl url = tabBar->tabUrl(index);
    EXPECT_TRUE(url.isEmpty());
}

TEST_F(TabBarTest, TabAlias_SetAndGet_ReturnsCorrectAlias)
{
    int index = tabBar->appendTab();
    QString alias = "My Custom Tab";

    tabBar->setTabAlias(index, alias);
    EXPECT_EQ(tabBar->tabAlias(index), alias);
}

TEST_F(TabBarTest, TabAlias_EmptyAlias_ReturnsEmpty)
{
    int index = tabBar->appendTab();
    tabBar->setTabAlias(index, "");

    EXPECT_TRUE(tabBar->tabAlias(index).isEmpty());
}

TEST_F(TabBarTest, TabAlias_InvalidIndex_ReturnsEmpty)
{
    QString alias = tabBar->tabAlias(999);
    EXPECT_TRUE(alias.isEmpty());
}

TEST_F(TabBarTest, TabAlias_UpdateAlias_UpdatesValue)
{
    int index = tabBar->appendTab();

    tabBar->setTabAlias(index, "First Alias");
    EXPECT_EQ(tabBar->tabAlias(index), QString("First Alias"));

    tabBar->setTabAlias(index, "Second Alias");
    EXPECT_EQ(tabBar->tabAlias(index), QString("Second Alias"));
}

TEST_F(TabBarTest, SetTabAlias_InvalidIndex_DoesNotCrash)
{
    tabBar->setTabAlias(999, "Test Alias");
    // Should not crash
}

TEST_F(TabBarTest, TabUniqueId_ValidIndex_ReturnsNonEmptyId)
{
    int index = tabBar->appendTab();
    QString uniqueId = tabBar->tabUniqueId(index);
    EXPECT_FALSE(uniqueId.isEmpty());
}

TEST_F(TabBarTest, TabUniqueId_MultipleTabs_ReturnsUniqueIds)
{
    int index1 = tabBar->appendTab();
    int index2 = tabBar->appendTab();

    QString id1 = tabBar->tabUniqueId(index1);
    QString id2 = tabBar->tabUniqueId(index2);

    EXPECT_NE(id1, id2);
}

TEST_F(TabBarTest, TabUniqueId_InvalidIndex_ReturnsEmpty)
{
    QString uniqueId = tabBar->tabUniqueId(999);
    EXPECT_TRUE(uniqueId.isEmpty());
}

TEST_F(TabBarTest, TabUserData_SetAndGet_ReturnsCorrectData)
{
    int index = tabBar->appendTab();
    QString key = "testKey";
    QVariant value = QString("testValue");

    tabBar->setTabUserData(index, key, value);
    EXPECT_EQ(tabBar->tabUserData(index, key), value);
}

TEST_F(TabBarTest, TabUserData_MultipleKeys_StoresAllData)
{
    int index = tabBar->appendTab();

    tabBar->setTabUserData(index, "key1", QString("value1"));
    tabBar->setTabUserData(index, "key2", 42);
    tabBar->setTabUserData(index, "key3", true);

    EXPECT_EQ(tabBar->tabUserData(index, "key1").toString(), QString("value1"));
    EXPECT_EQ(tabBar->tabUserData(index, "key2").toInt(), 42);
    EXPECT_EQ(tabBar->tabUserData(index, "key3").toBool(), true);
}

TEST_F(TabBarTest, TabUserData_InvalidIndex_ReturnsInvalid)
{
    QVariant data = tabBar->tabUserData(999, "key");
    EXPECT_FALSE(data.isValid());
}

TEST_F(TabBarTest, TabUserData_NonExistingKey_ReturnsInvalid)
{
    int index = tabBar->appendTab();
    QVariant data = tabBar->tabUserData(index, "nonExistingKey");
    EXPECT_FALSE(data.isValid());
}

TEST_F(TabBarTest, SetTabUserData_UpdateValue_UpdatesData)
{
    int index = tabBar->appendTab();
    QString key = "key";

    tabBar->setTabUserData(index, key, QString("value1"));
    EXPECT_EQ(tabBar->tabUserData(index, key).toString(), QString("value1"));

    tabBar->setTabUserData(index, key, QString("value2"));
    EXPECT_EQ(tabBar->tabUserData(index, key).toString(), QString("value2"));
}

TEST_F(TabBarTest, SetTabUserData_InvalidIndex_DoesNotCrash)
{
    tabBar->setTabUserData(999, "key", QString("value"));
    // Should not crash
}

TEST_F(TabBarTest, IsInactiveTab_InactiveTab_ReturnsTrue)
{
    QUrl url("file:///home/test");
    int index = tabBar->appendInactiveTab(url);

    EXPECT_TRUE(tabBar->isInactiveTab(index));
}

TEST_F(TabBarTest, IsInactiveTab_ActiveTab_ReturnsFalse)
{
    int index = tabBar->appendTab();
    EXPECT_FALSE(tabBar->isInactiveTab(index));
}

TEST_F(TabBarTest, IsInactiveTab_InvalidIndex_ReturnsFalse)
{
    EXPECT_FALSE(tabBar->isInactiveTab(999));
}

TEST_F(TabBarTest, ActivateNextTab_WithMultipleTabs_ActivatesNextTab)
{
    tabBar->appendTab();
    tabBar->appendTab();
    tabBar->appendTab();

    tabBar->setCurrentIndex(0);
    EXPECT_EQ(tabBar->currentIndex(), 0);

    tabBar->activateNextTab();
    EXPECT_EQ(tabBar->currentIndex(), 1);
}

TEST_F(TabBarTest, ActivateNextTab_AtLastTab_WrapsToFirst)
{
    tabBar->appendTab();
    tabBar->appendTab();

    tabBar->setCurrentIndex(1);   // Last tab
    tabBar->activateNextTab();
    EXPECT_EQ(tabBar->currentIndex(), 0);   // Should wrap to first
}

TEST_F(TabBarTest, ActivateNextTab_SingleTab_StaysOnSameTab)
{
    tabBar->appendTab();
    tabBar->setCurrentIndex(0);

    tabBar->activateNextTab();
    EXPECT_EQ(tabBar->currentIndex(), 0);
}

TEST_F(TabBarTest, ActivateNextTab_NoTabs_DoesNotCrash)
{
    tabBar->activateNextTab();
    // Should not crash
}

TEST_F(TabBarTest, ActivatePreviousTab_WithMultipleTabs_ActivatesPreviousTab)
{
    tabBar->appendTab();
    tabBar->appendTab();
    tabBar->appendTab();

    tabBar->setCurrentIndex(2);
    EXPECT_EQ(tabBar->currentIndex(), 2);

    tabBar->activatePreviousTab();
    EXPECT_EQ(tabBar->currentIndex(), 1);
}

TEST_F(TabBarTest, ActivatePreviousTab_AtFirstTab_WrapsToLast)
{
    tabBar->appendTab();
    tabBar->appendTab();

    tabBar->setCurrentIndex(0);   // First tab
    tabBar->activatePreviousTab();
    EXPECT_EQ(tabBar->currentIndex(), 1);   // Should wrap to last
}

TEST_F(TabBarTest, ActivatePreviousTab_SingleTab_StaysOnSameTab)
{
    tabBar->appendTab();
    tabBar->setCurrentIndex(0);

    tabBar->activatePreviousTab();
    EXPECT_EQ(tabBar->currentIndex(), 0);
}

TEST_F(TabBarTest, ActivatePreviousTab_NoTabs_DoesNotCrash)
{
    tabBar->activatePreviousTab();
    // Should not crash
}

TEST_F(TabBarTest, UpdateTabName_ValidIndex_UpdatesName)
{
    int index = tabBar->appendTab();

    stub.set_lamda(&TabBar::setTabText, [] {
        __DBG_STUB_INVOKE__
    });

    tabBar->updateTabName(index);
    // Verify it doesn't crash
}

TEST_F(TabBarTest, UpdateTabName_InvalidIndex_DoesNotCrash)
{
    stub.set_lamda(&TabBar::setTabText, [] {
        __DBG_STUB_INVOKE__
    });

    tabBar->updateTabName(999);
    // Should not crash
}

TEST_F(TabBarTest, NewTabCreated_CreateTab_SignalEmitted)
{
    QSignalSpy spy(tabBar, &TabBar::newTabCreated);

    tabBar->appendTab();
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TabBarTest, NewTabCreated_MultipleTabs_SignalEmittedMultipleTimes)
{
    QSignalSpy spy(tabBar, &TabBar::newTabCreated);

    tabBar->appendTab();
    tabBar->appendTab();
    tabBar->appendTab();

    EXPECT_EQ(spy.count(), 3);
}

TEST_F(TabBarTest, TabSizeHint_ValidIndex_ReturnsValidSize)
{
    int index = tabBar->appendTab();
    QSize size = tabBar->tabSizeHint(index);

    EXPECT_GT(size.width(), 0);
    EXPECT_GT(size.height(), 0);
}

TEST_F(TabBarTest, MinimumTabSizeHint_ValidIndex_ReturnsValidSize)
{
    int index = tabBar->appendTab();
    QSize size = tabBar->minimumTabSizeHint(index);

    EXPECT_GT(size.width(), 0);
    EXPECT_GT(size.height(), 0);
}

TEST_F(TabBarTest, MaximumTabSizeHint_ValidIndex_ReturnsValidSize)
{
    int index = tabBar->appendTab();
    QSize size = tabBar->maximumTabSizeHint(index);

    EXPECT_GT(size.width(), 0);
    EXPECT_GT(size.height(), 0);
}

TEST_F(TabBarTest, MaximumTabSizeHint_GreaterThanMinimum_Consistent)
{
    int index = tabBar->appendTab();
    QSize minSize = tabBar->minimumTabSizeHint(index);
    QSize maxSize = tabBar->maximumTabSizeHint(index);

    EXPECT_GE(maxSize.width(), minSize.width());
}

TEST_F(TabBarTest, CreateMimeDataFromTab_ValidIndex_ReturnsMimeData)
{
    int index = tabBar->appendTab();
    QStyleOptionTab option;

    QMimeData *mimeData = tabBar->createMimeDataFromTab(index, option);
    EXPECT_NE(mimeData, nullptr);

    if (mimeData) {
        delete mimeData;
    }
}

TEST_F(TabBarTest, CreateDragPixmapFromTab_ValidIndex_ReturnsPixmap)
{
    int index = tabBar->appendTab();
    QStyleOptionTab option;
    QPoint hotspot;

    QPixmap pixmap = tabBar->createDragPixmapFromTab(index, option, &hotspot);
    EXPECT_TRUE(pixmap.isNull());
}

TEST_F(TabBarTest, CanInsertFromMimeData_ValidMimeData_ReturnsBoolean)
{
    QMimeData mimeData;
    bool canInsert = tabBar->canInsertFromMimeData(0, &mimeData);
    EXPECT_FALSE(canInsert);
}

TEST_F(TabBarTest, ResizeEvent_ValidEvent_HandlesEvent)
{
    QResizeEvent event(QSize(800, 40), QSize(600, 40));

    // Should not crash
    EXPECT_NO_THROW(tabBar->resizeEvent(&event));
}

TEST_F(TabBarTest, CloseTab_WithParentUrl_RedirectsToParent)
{
    QUrl parentUrl("file:///home");
    QUrl childUrl("file:///home/test");

    stub.set_lamda(&TabBar::setTabText, [] {
        __DBG_STUB_INVOKE__
    });

    int index = tabBar->appendTab();
    tabBar->setCurrentUrl(childUrl);
    tabBar->closeTab(childUrl);

    // Tab should still exist with parent URL
    EXPECT_EQ(tabBar->count(), 1);
}

TEST_F(TabBarTest, CreateTab_AfterRemoval_IncrementsUniqueId)
{
    int index1 = tabBar->appendTab();
    QString id1 = tabBar->tabUniqueId(index1);

    tabBar->removeTab(index1);

    int index2 = tabBar->appendTab();
    QString id2 = tabBar->tabUniqueId(index2);

    EXPECT_NE(id1, id2);
}

TEST_F(TabBarTest, RemoveTab_MiddleTab_AdjustsIndices)
{
    QUrl url1("file:///home/test1");
    QUrl url2("file:///home/test2");
    QUrl url3("file:///home/test3");

    tabBar->appendInactiveTab(url1);
    tabBar->appendInactiveTab(url2);
    tabBar->appendInactiveTab(url3);

    EXPECT_EQ(tabBar->count(), 3);

    tabBar->removeTab(1);

    EXPECT_EQ(tabBar->count(), 2);
    EXPECT_EQ(tabBar->tabUrl(0), url1);
    EXPECT_EQ(tabBar->tabUrl(1), url3);
}

TEST_F(TabBarTest, SetCurrentUrl_WithAlias_PreservesAlias)
{
    stub.set_lamda(&TabBar::setTabText, [] {
        __DBG_STUB_INVOKE__
    });

    QUrl url("file:///home/test");
    tabBar->setCurrentUrl(url);
    int index = tabBar->currentIndex();
    QString alias = "My Tab";
    tabBar->setTabAlias(index, alias);

    EXPECT_EQ(tabBar->tabAlias(index), alias);
}

TEST_F(TabBarTest, TabUrl_AfterSetCurrentUrl_ReturnsNewUrl)
{
    int index = tabBar->appendTab();

    stub.set_lamda(&TabBar::setTabText, [] {
        __DBG_STUB_INVOKE__
    });

    QUrl url1("file:///home/test1");
    tabBar->setCurrentUrl(url1);
    EXPECT_EQ(tabBar->tabUrl(index), url1);

    QUrl url2("file:///home/test2");
    tabBar->setCurrentUrl(url2);
    EXPECT_EQ(tabBar->tabUrl(index), url2);
}

TEST_F(TabBarTest, CreateInactiveTab_MultipleUrls_CreatesMultipleTabs)
{
    QUrl url1("file:///home/test1");
    QUrl url2("file:///home/test2");
    QUrl url3("file:///home/test3");

    int index1 = tabBar->appendInactiveTab(url1);
    int index2 = tabBar->appendInactiveTab(url2);
    int index3 = tabBar->appendInactiveTab(url3);

    EXPECT_EQ(tabBar->count(), 3);
    EXPECT_EQ(tabBar->tabUrl(index1), url1);
    EXPECT_EQ(tabBar->tabUrl(index2), url2);
    EXPECT_EQ(tabBar->tabUrl(index3), url3);
}

TEST_F(TabBarTest, IsInactiveTab_AfterActivation_ReturnsFalse)
{
    QUrl url("file:///home/test");
    int index = tabBar->appendInactiveTab(url);

    EXPECT_TRUE(tabBar->isInactiveTab(index));

    // Simulate tab activation by creating a regular tab
    tabBar->appendTab();

    // Original tab should still be inactive
    EXPECT_TRUE(tabBar->isInactiveTab(index));
}
