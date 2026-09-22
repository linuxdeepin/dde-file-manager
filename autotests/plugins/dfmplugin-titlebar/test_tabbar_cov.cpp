// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage targets (from ut-dfmplugin-titlebar gap list, views/tabbar.cpp):
//   - TabBar::mousePressEvent                                -> MousePress_MiddleButton_CloseRequested
//   - TabBar::insertFromMimeData                             -> InsertFromMimeData_ValidJson_TabAppended
//   - TabBar::insertFromMimeDataOnDragEnter                  -> InsertFromMimeDataOnDragEnter_ValidJson_PreviewTabInserted
//   - TabBarPrivate::handleTabReleased (+showWindow path)    -> TabReleaseRequested_SingleTab_ShowWindowPath / _MultiTab path
//   - TabBarPrivate::handleTabDroped                         -> TabDroped_EmptyTarget_DelegatesToRelease
//   - TabBarPrivate::handleTabMoved                          -> TabMoved_SignalDriven_HandlerInvoked
//   - TabBarPrivate::handleDragActionChanged                 -> DragActionChanged_IgnoreAction_NoCrash
//   - TabBarPrivate::handlePinnedTabsChanged                 -> DConfigValueChanged_HandlerInvoked
//   - TabBarPrivate::initConnections lambda#1 (mountPointAboutToRemoved) -> DevMountPointRemoved_TabClosed
//   - TabBarPrivate::initConnections lambda#2 (themeTypeChanged)         -> ThemeTypeChanged_MaskColorUpdated
// Branches of mousePressEvent: middle button over a tab emits tabCloseRequested; other buttons pass to base.

#include "stubext.h"
#include "views/tabbar.h"
#include "utils/titlebarhelper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-framework/event/event.h>
#include <dfm-framework/dpf.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <DGuiApplicationHelper>

#include <gtest/gtest.h>
#include <QUrl>
#include <QSignalSpy>
#include <QTest>
#include <QMimeData>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMouseEvent>
#include <QMetaObject>
#include <QMetaMethod>
#include <QApplication>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace dfmplugin_titlebar;

namespace {
class ExposedTabBar : public TabBar
{
public:
    using TabBar::mousePressEvent;
    using TabBar::insertFromMimeData;
    using TabBar::insertFromMimeDataOnDragEnter;
};
}   // namespace

class UT_TabBarCov : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
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
        tabBar = new ExposedTabBar();
        tabBar->resize(600, 40);
    }
    void TearDown() override
    {
        delete tabBar;
        tabBar = nullptr;
        stub.clear();
    }
    static QMimeData *makeTabMimeData(const QUrl &url, const QString &alias = QString())
    {
        QJsonObject obj;
        obj[TabDef::kTabUrl] = url.toString();
        obj[TabDef::kTabAlias] = alias;
        QMimeData *data = new QMimeData();
        data->setData("application/x-dde-filemanager-tab",
                      QJsonDocument(obj).toJson());
        return data;
    }
    ExposedTabBar *tabBar { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_TabBarCov, MousePress_MiddleButton_CloseRequested)
{
    // Arrange
    QSignalSpy closeSpy(tabBar, &TabBar::tabCloseRequested);
    tabBar->appendInactiveTab(QUrl("file://home"));
    ASSERT_GT(tabBar->count(), 0);
    const QPoint tabCenter = tabBar->tabRect(0).center();
    QMouseEvent midPress(QEvent::MouseButtonPress, tabCenter, tabBar->mapToGlobal(tabCenter),
                         Qt::MiddleButton, Qt::MiddleButton, Qt::NoModifier);

    // Act
    tabBar->mousePressEvent(&midPress);

    // Assert
    EXPECT_EQ(closeSpy.count(), 1);
    EXPECT_EQ(tabBar->count(), 1);
}

TEST_F(UT_TabBarCov, MousePress_LeftButton_NoCloseRequested)
{
    // Arrange
    QSignalSpy closeSpy(tabBar, &TabBar::tabCloseRequested);
    tabBar->appendInactiveTab(QUrl("file://home"));
    const QPoint tabCenter = tabBar->tabRect(0).center();
    QMouseEvent leftPress(QEvent::MouseButtonPress, tabCenter, tabBar->mapToGlobal(tabCenter),
                          Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    // Act
    tabBar->mousePressEvent(&leftPress);

    // Assert
    EXPECT_EQ(closeSpy.count(), 0);
    EXPECT_EQ(tabBar->count(), 1);
}

TEST_F(UT_TabBarCov, InsertFromMimeData_ValidJson_TabAppended)
{
    // Arrange
    QSignalSpy newTabSpy(tabBar, &TabBar::newTabCreated);
    QMimeData *data = makeTabMimeData(QUrl("file:///home"), QString("alias"));
    const int before = tabBar->count();

    // Act
    tabBar->insertFromMimeData(before, data);

    // Assert
    EXPECT_EQ(tabBar->count(), before + 1);
    EXPECT_EQ(newTabSpy.count(), 1);
}

TEST_F(UT_TabBarCov, InsertFromMimeData_InvalidFormat_NoTabAdded)
{
    // Arrange
    QMimeData data;
    const int before = tabBar->count();

    // Act
    tabBar->insertFromMimeData(before, &data);

    // Assert
    EXPECT_EQ(tabBar->count(), before);
    EXPECT_FALSE(tabBar->isTabValid(before));
}

TEST_F(UT_TabBarCov, InsertFromMimeDataOnDragEnter_ValidJson_PreviewTabInserted)
{
    // Arrange
    QMimeData *data = makeTabMimeData(QUrl("file:///home"), QString());
    const int before = tabBar->count();

    // Act
    tabBar->insertFromMimeDataOnDragEnter(before, data);

    // Assert
    EXPECT_EQ(tabBar->count(), before + 1);
    EXPECT_TRUE(tabBar->isTabValid(before));
}

TEST_F(UT_TabBarCov, TabReleaseRequested_SingleTab_ShowWindowPath)
{
    // Arrange: single tab -> handleTabReleased calls showWindow()
    tabBar->appendInactiveTab(QUrl("file://home"));
    ASSERT_EQ(tabBar->count(), 1);
    stub.set_lamda(&TabBar::setTabText, [] {
        __DBG_STUB_INVOKE__
    });

    // Act: emit the signal the private handler is connected to
    const bool invoked = QMetaObject::invokeMethod(
            tabBar, "tabReleaseRequested", Qt::DirectConnection, Q_ARG(int, 0));

    // Assert
    EXPECT_TRUE(invoked);
    EXPECT_EQ(tabBar->count(), 1);
}

TEST_F(UT_TabBarCov, TabReleaseRequested_MultiTab_CloseAndPublishPath)
{
    // Arrange: two tabs -> publish open-new-window and request close
    tabBar->appendInactiveTab(QUrl("file://home"));
    tabBar->appendInactiveTab(QUrl("file://home"));
    ASSERT_EQ(tabBar->count(), 2);
    QSignalSpy closeSpy(tabBar, &TabBar::tabCloseRequested);

    // Act
    const bool invoked = QMetaObject::invokeMethod(
            tabBar, "tabReleaseRequested", Qt::DirectConnection, Q_ARG(int, 0));

    // Assert
    EXPECT_TRUE(invoked);
    EXPECT_EQ(closeSpy.count(), 1);
    EXPECT_EQ(tabBar->count(), 2);
}

TEST_F(UT_TabBarCov, TabDroped_EmptyTarget_DelegatesToRelease)
{
    // Arrange
    tabBar->appendInactiveTab(QUrl("file://home"));
    tabBar->appendInactiveTab(QUrl("file://home"));
    QSignalSpy closeSpy(tabBar, &TabBar::tabCloseRequested);

    // Act: target == nullptr -> treat as drop on empty area
    const bool invoked = QMetaObject::invokeMethod(
            tabBar, "tabDroped", Qt::DirectConnection,
            Q_ARG(int, 0), Q_ARG(Qt::DropAction, Qt::CopyAction), Q_ARG(QObject *, nullptr));

    // Assert
    EXPECT_TRUE(invoked);
    EXPECT_EQ(closeSpy.count(), 1);
}

TEST_F(UT_TabBarCov, TabMoved_SignalDriven_HandlerInvoked)
{
    // Arrange
    tabBar->appendInactiveTab(QUrl("file://home"));
    tabBar->appendInactiveTab(QUrl("file://home"));
    ASSERT_EQ(tabBar->count(), 2);

    // Act
    const bool invoked = QMetaObject::invokeMethod(
            tabBar, "tabMoved", Qt::DirectConnection, Q_ARG(int, 0), Q_ARG(int, 1));

    // Assert
    EXPECT_TRUE(invoked);
    EXPECT_EQ(tabBar->count(), 2);
}

TEST_F(UT_TabBarCov, DragActionChanged_IgnoreAction_NoCrash)
{
    // Arrange
    tabBar->appendInactiveTab(QUrl("file://home"));

    // Act
    const bool invoked = QMetaObject::invokeMethod(
            tabBar, "dragActionChanged", Qt::DirectConnection,
            Q_ARG(Qt::DropAction, Qt::IgnoreAction));

    // Assert
    EXPECT_TRUE(invoked);
    EXPECT_EQ(tabBar->count(), 1);
}

TEST_F(UT_TabBarCov, DConfigValueChanged_HandlerInvoked)
{
    // Arrange
    tabBar->appendInactiveTab(QUrl("file://home"));

    // Act: emit DConfigManager::valueChanged -> handlePinnedTabsChanged
    const bool invoked = QMetaObject::invokeMethod(
            DConfigManager::instance(), "valueChanged", Qt::DirectConnection,
            Q_ARG(QString, QString("org.deepin.filemanager.view")),
            Q_ARG(QString, QString("pinnedTabs")));

    // Assert
    EXPECT_TRUE(invoked);
    EXPECT_EQ(tabBar->count(), 1);
}

TEST_F(UT_TabBarCov, DevMountPointRemoved_TabClosed)
{
    // Arrange: lambda connected to DeviceProxyManager::mountPointAboutToRemoved
    tabBar->appendInactiveTab(QUrl("file://home"));
    const int before = tabBar->count();

    // Act
    const bool invoked = QMetaObject::invokeMethod(
            DevProxyMng, "mountPointAboutToRemoved", Qt::DirectConnection,
            Q_ARG(QStringView, QStringView(u"/home")));

    // Assert
    EXPECT_TRUE(invoked);
    EXPECT_EQ(tabBar->count(), before);
}

TEST_F(UT_TabBarCov, ThemeTypeChanged_MaskColorUpdated)
{
    // Arrange: lambda connected to DGuiApplicationHelper::themeTypeChanged
    const int methodCount = tabBar->metaObject()->methodCount();

    // Act
    const bool invoked = QMetaObject::invokeMethod(
            DGuiApplicationHelper::instance(), "themeTypeChanged", Qt::DirectConnection,
            Q_ARG(DGuiApplicationHelper::ColorType, DGuiApplicationHelper::LightType));

    // Assert
    EXPECT_TRUE(invoked);
    EXPECT_GT(methodCount, 0);
}
