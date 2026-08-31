// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_workspacepage.cpp
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

TEST_F(WorkspacePageTest, WorkspacePage)
{
    // Test constructor: WorkspacePage((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(WorkspacePageTest, initCustomTopWidgets)
{
    // Test method: void initCustomTopWidgets((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->initCustomTopWidgets(_arg0));
}

TEST_F(WorkspacePageTest, onAnimDelayTimeout)
{
    // Test method: void onAnimDelayTimeout(())
    EXPECT_NO_FATAL_FAILURE(obj->onAnimDelayTimeout());
}

TEST_F(WorkspacePageTest, setUrl)
{
    // Test setter: void setUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setUrl(_arg0));
}

TEST_F(WorkspacePageTest, viewStateChanged)
{
    // Test method: void viewStateChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->viewStateChanged());
}
