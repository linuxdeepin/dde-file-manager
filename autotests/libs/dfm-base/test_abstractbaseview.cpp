// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractbaseview.cpp
 * @brief Unit tests for AbstractBaseView default implementations.
 */

#include <gtest/gtest.h>
#include <QWidget>
#include <QUrl>
#include <QList>
#include <QAction>

#include <dfm-base/interfaces/abstractbaseview.h>

using namespace dfmbase;

// Minimal concrete subclass to exercise AbstractBaseView's default virtual methods.
class TestAbstractBaseView : public AbstractBaseView
{
public:
    QWidget *widget() const override { return &m_widget; }
    QUrl rootUrl() const override { return m_rootUrl; }
    bool setRootUrl(const QUrl &url) override { m_rootUrl = url; return true; }

private:
    mutable QWidget m_widget;
    QUrl m_rootUrl;
};

TEST(AbstractBaseViewTest, DefaultViewStateIsIdle)
{
    TestAbstractBaseView view;
    EXPECT_EQ(view.viewState(), AbstractBaseView::ViewState::kViewIdle);
}

TEST(AbstractBaseViewTest, DefaultToolBarActionListIsEmpty)
{
    TestAbstractBaseView view;
    EXPECT_TRUE(view.toolBarActionList().isEmpty());
}

TEST(AbstractBaseViewTest, DefaultSelectedUrlListIsEmpty)
{
    TestAbstractBaseView view;
    EXPECT_TRUE(view.selectedUrlList().isEmpty());
}

TEST(AbstractBaseViewTest, RefreshIsNoopAndSafe)
{
    TestAbstractBaseView view;
    EXPECT_NO_FATAL_FAILURE({ view.refresh(); });
}

TEST(AbstractBaseViewTest, DefaultContentWidgetIsNull)
{
    TestAbstractBaseView view;
    // The default implementation returns nullptr, but the concrete subclass
    // overrides widget() — still, contentWidget() returns nullptr.
    EXPECT_EQ(view.contentWidget(), nullptr);
}

TEST(AbstractBaseViewTest, SetRootUrlRoundTrips)
{
    TestAbstractBaseView view;
    QUrl url("file:///tmp/test");
    view.setRootUrl(url);
    EXPECT_EQ(view.rootUrl(), url);
}

TEST(AbstractBaseViewTest, LocalViewDestructsCleanly)
{
    EXPECT_NO_FATAL_FAILURE({ TestAbstractBaseView view; });
}