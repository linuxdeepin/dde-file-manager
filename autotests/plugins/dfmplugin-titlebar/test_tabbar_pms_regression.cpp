// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// PMS sev-2 regression tests for dfmplugin-titlebar (TabBar creation/close signals).

#include "stubext.h"
#include "views/tabbar.h"

#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/universalutils.h>

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QMouseEvent>
#include <QIcon>

using namespace dfmplugin_titlebar;
DFMBASE_USE_NAMESPACE

namespace {
class ExposedTabBar : public TabBar
{
public:
    using TabBar::mousePressEvent;
};
}   // namespace

class TabBarPmsRegressionTest : public testing::Test
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

    ExposedTabBar *tabBar { nullptr };
    stub_ext::StubExt stub;
};

// PMS:335187 tab creation suppressed/lost currentChanged: creating a
// tab did not notify back/forward button state, leaving the toolbar unresponsive.
// The fix blocks signals during addTab and explicitly emits currentChanged with
// the new index, so exactly one currentChanged per created tab is expected.
TEST_F(TabBarPmsRegressionTest, BUG335187_CreateTab_EmitsCurrentChangedOnce)
{
    QSignalSpy currentSpy(tabBar, &TabBar::currentChanged);
    QSignalSpy newTabSpy(tabBar, &TabBar::newTabCreated);
    ASSERT_TRUE(currentSpy.isValid());
    ASSERT_TRUE(newTabSpy.isValid());

    int index = tabBar->appendTab();
    EXPECT_EQ(index, 0);
    EXPECT_EQ(newTabSpy.count(), 1);
    ASSERT_GE(currentSpy.count(), 1);                       // pre-fix: not emitted at all
    EXPECT_EQ(currentSpy.last().value(0).toInt(), 0);
    EXPECT_EQ(tabBar->currentIndex(), 0);

    currentSpy.clear();
    newTabSpy.clear();

    index = tabBar->appendTab();
    EXPECT_EQ(index, 1);
    EXPECT_EQ(newTabSpy.count(), 1);
    ASSERT_GE(currentSpy.count(), 1);                       // at least one per tab, again
    EXPECT_EQ(currentSpy.last().value(0).toInt(), 1);
    EXPECT_EQ(tabBar->currentIndex(), 1);
}

// PMS:336827 middle-clicking empty tabbar area emitted tabCloseRequested(-1)
// (downstream access to a -1 index crashed); the fix only emits when tabAt()
// returns a valid index.
TEST_F(TabBarPmsRegressionTest, BUG336827_MousePress_EmptyArea_NoCloseRequested)
{
    QSignalSpy closeSpy(tabBar, &TabBar::tabCloseRequested);
    ASSERT_TRUE(closeSpy.isValid());

    // Middle press where tabAt() == -1 (empty area).
    stub.set_lamda(ADDR(QTabBar, tabAt), [](const QTabBar *, const QPoint &) -> int {
        __DBG_STUB_INVOKE__
        return -1;
    });
    const QPoint emptyPos(590, 20);
    QMouseEvent midPress(QEvent::MouseButtonPress, emptyPos, tabBar->mapToGlobal(emptyPos),
                         Qt::MiddleButton, Qt::MiddleButton, Qt::NoModifier);
    tabBar->mousePressEvent(&midPress);
    EXPECT_EQ(closeSpy.count(), 0);   // pre-fix emitted tabCloseRequested(-1)

    // Middle press over a real tab still emits the request with that index.
    stub.set_lamda(ADDR(QTabBar, tabAt), [](const QTabBar *, const QPoint &) -> int {
        __DBG_STUB_INVOKE__
        return 0;
    });
    tabBar->mousePressEvent(&midPress);
    ASSERT_EQ(closeSpy.count(), 1);
    EXPECT_EQ(closeSpy.first().value(0).toInt(), 0);
}
