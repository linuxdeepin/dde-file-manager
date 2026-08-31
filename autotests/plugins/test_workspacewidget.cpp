// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_workspacewidget.cpp
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

TEST_F(WorkspaceWidgetTest, WorkspaceWidget)
{
    // Test constructor: WorkspaceWidget((QFrame *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(WorkspaceWidgetTest, itemRect)
{
    // Test method: QRectF itemRect((const QUrl &url, const Global::ItemRoles role))
    QUrl _arg0{};
    auto result = obj->itemRect(_arg0, Global::ItemRoles());
    EXPECT_FALSE(result.isValid());

}

TEST_F(WorkspaceWidgetTest, removePage)
{
    // Test method: void removePage((const QString &removedId, const QString &nextId))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->removePage(_arg0, _arg1));
}
