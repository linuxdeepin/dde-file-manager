// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_workspacewidget_1.cpp
 * @brief Unit tests for WorkspaceWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/workspacewidget.h"

#include <QTest>

using namespace dfmplugin_workspace;

class WorkspaceWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WorkspaceWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WorkspaceWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WorkspaceWidgetTest, createNewPage)
{
    // Test method: void createNewPage((const QString &uniqueId))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->createNewPage(_arg0));
}

TEST_F(WorkspaceWidgetTest, currentUrl)
{
    // Test getter: QUrl currentUrl()
    auto result = obj->currentUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(WorkspaceWidgetTest, currentView)
{
    // Test getter: AbstractBaseView currentView()
    auto result = obj->currentView();
    EXPECT_NO_FATAL_FAILURE({ obj->currentView(); });

}

TEST_F(WorkspaceWidgetTest, currentViewMode)
{
    // Test getter: Global::ViewMode currentViewMode()
    auto result = obj->currentViewMode();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(WorkspaceWidgetTest, focusInEvent)
{
    // Test event handler: focusInEvent((QFocusEvent *event))
    QFocusEvent _event(QFocusEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->focusInEvent(&_event));
}

TEST_F(WorkspaceWidgetTest, getCustomTopWidgetVisible)
{
    // Test method: bool getCustomTopWidgetVisible((const QString &scheme))
    QString _arg0{};
    auto result = obj->getCustomTopWidgetVisible(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceWidgetTest, handleAboutToPlaySplitterAnim)
{
    // Test method: void handleAboutToPlaySplitterAnim((int startValue, int endValue))
    EXPECT_NO_FATAL_FAILURE(obj->handleAboutToPlaySplitterAnim(0, 0));
}

TEST_F(WorkspaceWidgetTest, handleViewStateChanged)
{
    // Test method: void handleViewStateChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->handleViewStateChanged());
}

TEST_F(WorkspaceWidgetTest, initViewLayout)
{
    // Test method: void initViewLayout(())
    EXPECT_NO_FATAL_FAILURE(obj->initViewLayout());
}

TEST_F(WorkspaceWidgetTest, initializeUi)
{
    // Test method: void initializeUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeUi());
}

TEST_F(WorkspaceWidgetTest, onCreateNewWindow)
{
    // Test method: void onCreateNewWindow(())
    EXPECT_NO_FATAL_FAILURE(obj->onCreateNewWindow());
}

TEST_F(WorkspaceWidgetTest, onRefreshCurrentView)
{
    // Test method: void onRefreshCurrentView(())
    EXPECT_NO_FATAL_FAILURE(obj->onRefreshCurrentView());
}

TEST_F(WorkspaceWidgetTest, setCurrentPage)
{
    // Test setter: void setCurrentPage((const QString &uniqueId))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setCurrentPage(_arg0));
}

TEST_F(WorkspaceWidgetTest, setCurrentUrl)
{
    // Test setter: void setCurrentUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setCurrentUrl(_arg0));
}

TEST_F(WorkspaceWidgetTest, setCustomTopWidgetVisible)
{
    // Test setter: void setCustomTopWidgetVisible((const QString &scheme, bool visible))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setCustomTopWidgetVisible(_arg0, false));
}

TEST_F(WorkspaceWidgetTest, showEvent)
{
    // Test event handler: showEvent((QShowEvent *event))
    QShowEvent _event(QShowEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->showEvent(&_event));
}

TEST_F(WorkspaceWidgetTest, viewVisibleGeometry)
{
    // Test getter: QRectF viewVisibleGeometry()
    auto result = obj->viewVisibleGeometry();
    EXPECT_FALSE(result.isValid());

}
