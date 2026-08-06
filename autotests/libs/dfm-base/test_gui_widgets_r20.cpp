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

TEST(GUIWidgetsR20Test, SplitterConstruct)
{
    Splitter s(Qt::Horizontal);
    SUCCEED();
}

TEST(GUIWidgetsR20Test, SplitterHandleConstruct)
{
    Splitter s(Qt::Horizontal);
    SplitterHandle h(Qt::Horizontal, &s);
    SUCCEED();
}

TEST(GUIWidgetsR20Test, FMWindowsManagerInstance)
{
    EXPECT_NO_FATAL_FAILURE({ (void)FMWindowsIns; });
}

TEST(GUIWidgetsR20Test, FMWindowsManagerWindowIdList)
{
    EXPECT_NO_FATAL_FAILURE({ (void)FMWindowsIns.windowIdList(); });
}

TEST(GUIWidgetsR20Test, FMWindowsManagerPreviousActivedWindowId)
{
    EXPECT_NO_FATAL_FAILURE({ (void)FMWindowsIns.previousActivedWindowId(); });
}

TEST(GUIWidgetsR20Test, FMWindowsManagerLastActivedWindowId)
{
    EXPECT_NO_FATAL_FAILURE({ (void)FMWindowsIns.lastActivedWindowId(); });
}

TEST(GUIWidgetsR20Test, FMWindowsManagerResetPreviousActivedWindowId)
{
    EXPECT_NO_FATAL_FAILURE({ FMWindowsIns.resetPreviousActivedWindowId(); });
}

TEST(GUIWidgetsR20Test, FMWindowsManagerContainsCurrentUrl)
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

TEST(GUIWidgetsR20Test, AbstractBaseViewDefaultViewState)
{
    FakeBaseView v;
    EXPECT_NO_FATAL_FAILURE({ (void)v.viewState(); });
}

TEST(GUIWidgetsR20Test, AbstractBaseViewDefaultToolBarActions)
{
    FakeBaseView v;
    EXPECT_TRUE(v.toolBarActionList().isEmpty());
}

TEST(GUIWidgetsR20Test, AbstractBaseViewDefaultSelectedUrlList)
{
    FakeBaseView v;
    EXPECT_TRUE(v.selectedUrlList().isEmpty());
}

TEST(GUIWidgetsR20Test, AbstractBaseViewDefaultRefresh)
{
    FakeBaseView v;
    EXPECT_NO_FATAL_FAILURE({ v.refresh(); });
}

TEST(GUIWidgetsR20Test, AbstractBaseViewDefaultContentWidget)
{
    FakeBaseView v;
    EXPECT_EQ(v.contentWidget(), nullptr);
}

TEST(GUIWidgetsR20Test, BasicStatusBarConstruct)
{
    BasicStatusBar bar;
    SUCCEED();
}

TEST(GUIWidgetsR20Test, BasicStatusBarItemSelected)
{
    BasicStatusBar bar;
    EXPECT_NO_FATAL_FAILURE({ bar.itemSelected(0, 0, 0, {}); });
}

TEST(GUIWidgetsR20Test, BasicStatusBarItemCounted)
{
    BasicStatusBar bar;
    EXPECT_NO_FATAL_FAILURE({ bar.itemCounted(10); });
}

TEST(GUIWidgetsR20Test, BasicStatusBarClearLayout)
{
    BasicStatusBar bar;
    EXPECT_NO_FATAL_FAILURE({ bar.clearLayoutAndAnchors(); });
}

TEST(GUIWidgetsR20Test, BasicStatusBarUpdateStatusMessage)
{
    BasicStatusBar bar;
    EXPECT_NO_FATAL_FAILURE({ bar.updateStatusMessage(); });
}
