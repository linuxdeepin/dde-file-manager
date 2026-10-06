// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tabbar_cov2.cpp
 * @brief Coverage for tabbar.cpp methods not covered by test_tabbar_cov.cpp:
 *        insertInactiveTab, appendInactiveTab, forceRemoveTab, isPinned,
 *        canInsertFromMimeData, paintTab.
 */

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
#include <QPainter>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace dfmplugin_titlebar;

namespace {
class ExposedTabBar2 : public TabBar
{
public:
    using TabBar::insertInactiveTab;
    using TabBar::appendInactiveTab;
    using TabBar::forceRemoveTab;
    using TabBar::isPinned;
    using TabBar::canInsertFromMimeData;
    using TabBar::paintTab;
    using TabBar::mousePressEvent;
    using TabBar::insertFromMimeData;
    using TabBar::insertFromMimeDataOnDragEnter;
};
}   // namespace

class UT_TabBarCov2 : public testing::Test
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
        tabBar = new ExposedTabBar2();
        tabBar->resize(600, 40);
    }
    void TearDown() override
    {
        delete tabBar;
        tabBar = nullptr;
        stub.clear();
    }
    static QMimeData *makeTabMimeData(qint64 pid)
    {
        QJsonObject obj;
        obj[TabDef::kTabUrl] = QUrl("file:///home/test").toString();
        obj[TabDef::kTabAlias] = QString("test");
        obj[TabDef::kProcessId] = static_cast<double>(pid);
        QMimeData *data = new QMimeData();
        data->setData("application/x-dde-filemanager-tab",
                      QJsonDocument(obj).toJson());
        return data;
    }
    ExposedTabBar2 *tabBar { nullptr };
    stub_ext::StubExt stub;
};

// ---- insertInactiveTab ----

TEST_F(UT_TabBarCov2, InsertInactiveTab_NewTabCreated)
{
    QUrl url("file:///home/test");
    QSignalSpy spy(tabBar, &TabBar::newTabCreated);
    int idx = tabBar->insertInactiveTab(0, url, false);
    EXPECT_GE(idx, 0);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_TabBarCov2, InsertInactiveTab_PinnedFlagStored)
{
    QUrl url("file:///home/test");
    int idx = tabBar->insertInactiveTab(0, url, true);
    EXPECT_GE(idx, 0);
    EXPECT_TRUE(tabBar->isPinned(idx));
}

TEST_F(UT_TabBarCov2, InsertInactiveTab_NotPinnedByDefault)
{
    QUrl url("file:///home/test");
    int idx = tabBar->insertInactiveTab(0, url, false);
    EXPECT_GE(idx, 0);
    EXPECT_FALSE(tabBar->isPinned(idx));
}

TEST_F(UT_TabBarCov2, AppendInactiveTab_AppendsAtEnd)
{
    QUrl url1("file:///home/test1");
    QUrl url2("file:///home/test2");
    int idx1 = tabBar->insertInactiveTab(0, url1, false);
    int idx2 = tabBar->appendInactiveTab(url2, false);
    EXPECT_GT(idx2, idx1);
}

// ---- forceRemoveTab ----

TEST_F(UT_TabBarCov2, ForceRemoveTab_PinnedTab_UnpinsThenRemoves)
{
    QUrl url("file:///home/test");
    int idx = tabBar->insertInactiveTab(0, url, true);
    EXPECT_TRUE(tabBar->isPinned(idx));
    EXPECT_NO_FATAL_FAILURE({ tabBar->forceRemoveTab(idx); });
}

TEST_F(UT_TabBarCov2, ForceRemoveTab_NonPinned_RemovesDirectly)
{
    QUrl url("file:///home/test");
    int idx = tabBar->insertInactiveTab(0, url, false);
    EXPECT_NO_FATAL_FAILURE({ tabBar->forceRemoveTab(idx); });
}

// ---- isPinned ----

TEST_F(UT_TabBarCov2, IsPinned_ReturnsFalseForUnpinnedTab)
{
    QUrl url("file:///home/test");
    int idx = tabBar->insertInactiveTab(0, url, false);
    EXPECT_FALSE(tabBar->isPinned(idx));
}

TEST_F(UT_TabBarCov2, IsPinned_ReturnsTrueForPinnedTab)
{
    QUrl url("file:///home/test");
    int idx = tabBar->insertInactiveTab(0, url, true);
    EXPECT_TRUE(tabBar->isPinned(idx));
}

// ---- canInsertFromMimeData ----

TEST_F(UT_TabBarCov2, CanInsertFromMimeData_NullSource_ReturnsFalse)
{
    EXPECT_FALSE(tabBar->canInsertFromMimeData(0, nullptr));
}

TEST_F(UT_TabBarCov2, CanInsertFromMimeData_NoTabFormat_ReturnsFalse)
{
    QMimeData data;
    data.setText("hello");
    EXPECT_FALSE(tabBar->canInsertFromMimeData(0, &data));
}

TEST_F(UT_TabBarCov2, CanInsertFromMimeData_SameProcessId_ReturnsTrue)
{
    std::unique_ptr<QMimeData> data(makeTabMimeData(QApplication::applicationPid()));
    EXPECT_TRUE(tabBar->canInsertFromMimeData(0, data.get()));
}

TEST_F(UT_TabBarCov2, CanInsertFromMimeData_DifferentProcessId_ReturnsFalse)
{
    std::unique_ptr<QMimeData> data(makeTabMimeData(QApplication::applicationPid() + 1));
    EXPECT_FALSE(tabBar->canInsertFromMimeData(0, data.get()));
}

TEST_F(UT_TabBarCov2, CanInsertFromMimeData_InvalidJson_ReturnsFalse)
{
    QMimeData data;
    data.setData("application/x-dde-filemanager-tab", "not-json");
    EXPECT_FALSE(tabBar->canInsertFromMimeData(0, &data));
}

// ---- paintTab ----

TEST_F(UT_TabBarCov2, PaintTab_NoCrash)
{
    QUrl url("file:///home/test");
    int idx = tabBar->insertInactiveTab(0, url, false);

    QPixmap pixmap(200, 40);
    QPainter painter(&pixmap);
    QStyleOptionTab option;
    option.rect = QRect(0, 0, 200, 40);
    EXPECT_NO_FATAL_FAILURE({ tabBar->paintTab(&painter, idx, option); });
}
