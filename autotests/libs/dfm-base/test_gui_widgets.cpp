// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QApplication>
#include <QString>
#include <QUrl>

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/widgets/dfmsplitter/splitter.h>
#include <dfm-base/interfaces/abstractbaseview.h>
#include <dfm-base/widgets/dfmstatusbar/basicstatusbar.h>

using namespace dfmbase;

TEST(GUIWidgetsTest, SplitterConstruct)
{
    Splitter s(Qt::Horizontal);
    SUCCEED();
}

TEST(GUIWidgetsTest, SplitterHandleConstruct)
{
    Splitter s(Qt::Horizontal);
    SplitterHandle h(Qt::Horizontal, &s);
    SUCCEED();
}

TEST(GUIWidgetsTest, FMWindowsManagerInstance)
{
    EXPECT_NO_FATAL_FAILURE({ (void)FMWindowsIns; });
}

TEST(GUIWidgetsTest, FMWindowsManagerWindowIdList)
{
    EXPECT_NO_FATAL_FAILURE({ (void)FMWindowsIns.windowIdList(); });
}

TEST(GUIWidgetsTest, FMWindowsManagerPreviousActivedWindowId)
{
    EXPECT_NO_FATAL_FAILURE({ (void)FMWindowsIns.previousActivedWindowId(); });
}

TEST(GUIWidgetsTest, FMWindowsManagerLastActivedWindowId)
{
    EXPECT_NO_FATAL_FAILURE({ (void)FMWindowsIns.lastActivedWindowId(); });
}

TEST(GUIWidgetsTest, FMWindowsManagerResetPreviousActivedWindowId)
{
    EXPECT_NO_FATAL_FAILURE({ FMWindowsIns.resetPreviousActivedWindowId(); });
}

TEST(GUIWidgetsTest, FMWindowsManagerContainsCurrentUrl)
{
    QUrl url("file:///tmp");
    EXPECT_NO_FATAL_FAILURE({ (void)FMWindowsIns.containsCurrentUrl(url); });
}

namespace {
class FakeBaseView : public AbstractBaseView
{
public:
    FakeBaseView() : AbstractBaseView() {}
    QWidget *widget() const override { return nullptr; }
    QUrl rootUrl() const override { return QUrl(); }
    bool setRootUrl(const QUrl &) override { return false; }
};
}   // namespace

TEST(GUIWidgetsTest, AbstractBaseViewDefaultViewState)
{
    FakeBaseView v;
    EXPECT_NO_FATAL_FAILURE({ (void)v.viewState(); });
}

TEST(GUIWidgetsTest, AbstractBaseViewDefaultToolBarActions)
{
    FakeBaseView v;
    EXPECT_TRUE(v.toolBarActionList().isEmpty());
}

TEST(GUIWidgetsTest, AbstractBaseViewDefaultSelectedUrlList)
{
    FakeBaseView v;
    EXPECT_TRUE(v.selectedUrlList().isEmpty());
}

TEST(GUIWidgetsTest, AbstractBaseViewDefaultRefresh)
{
    FakeBaseView v;
    EXPECT_NO_FATAL_FAILURE({ v.refresh(); });
}

TEST(GUIWidgetsTest, AbstractBaseViewDefaultContentWidget)
{
    FakeBaseView v;
    EXPECT_EQ(v.contentWidget(), nullptr);
}

TEST(GUIWidgetsTest, BasicStatusBarConstruct)
{
    BasicStatusBar bar;
    SUCCEED();
}

TEST(GUIWidgetsTest, BasicStatusBarItemSelected)
{
    BasicStatusBar bar;
    EXPECT_NO_FATAL_FAILURE({ bar.itemSelected(0, 0, 0, {}); });
}

TEST(GUIWidgetsTest, BasicStatusBarItemCounted)
{
    BasicStatusBar bar;
    EXPECT_NO_FATAL_FAILURE({ bar.itemCounted(10); });
}

TEST(GUIWidgetsTest, BasicStatusBarClearLayout)
{
    BasicStatusBar bar;
    EXPECT_NO_FATAL_FAILURE({ bar.clearLayoutAndAnchors(); });
}

TEST(GUIWidgetsTest, BasicStatusBarUpdateStatusMessage)
{
    BasicStatusBar bar;
    EXPECT_NO_FATAL_FAILURE({ bar.updateStatusMessage(); });
}
