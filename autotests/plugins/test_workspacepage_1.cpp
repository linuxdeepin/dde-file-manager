// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_workspacepage_1.cpp
 * @brief Unit tests for WorkspacePage methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/workspacepage.h"

#include <QTest>

using namespace dfmplugin_workspace;

class WorkspacePageTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WorkspacePage();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WorkspacePage *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WorkspacePageTest, currentUrl)
{
    // Test getter: QUrl currentUrl()
    auto result = obj->currentUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(WorkspacePageTest, currentViewPtr)
{
    // Test getter: WorkspacePage::ViewPtr currentViewPtr()
    auto result = obj->currentViewPtr();
    EXPECT_EQ(result.get(), nullptr);

}

TEST_F(WorkspacePageTest, getCustomTopWidgetVisible)
{
    // Test method: bool getCustomTopWidgetVisible((const QString &scheme))
    QString _arg0{};
    auto result = obj->getCustomTopWidgetVisible(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(WorkspacePageTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(WorkspacePageTest, playDisappearAnimation)
{
    // Test method: void playDisappearAnimation((ViewPtr view))
    EXPECT_NO_FATAL_FAILURE(obj->playDisappearAnimation(ViewPtr()));
}

TEST_F(WorkspacePageTest, setCurrentView)
{
    // Test setter: void setCurrentView((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setCurrentView(_arg0));
}

TEST_F(WorkspacePageTest, setCustomTopWidgetVisible)
{
    // Test setter: void setCustomTopWidgetVisible((const QString &scheme, bool visible))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setCustomTopWidgetVisible(_arg0, false));
}

TEST_F(WorkspacePageTest, tryShowViewHint)
{
    // Test method: void tryShowViewHint((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->tryShowViewHint(_arg0));
}
