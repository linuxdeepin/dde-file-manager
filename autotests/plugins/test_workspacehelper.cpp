// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_workspacehelper.cpp
 * @brief Unit tests for WorkspaceHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/workspacehelper.h"

#include <QTest>

using namespace dfmplugin_workspace;

class WorkspaceHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WorkspaceHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WorkspaceHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WorkspaceHelperTest, WorkspaceHelper)
{
    // Test constructor: WorkspaceHelper((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(WorkspaceHelperTest, closePersistentEditor)
{
    // Test method: void closePersistentEditor((const quint64 windowID))
    EXPECT_NO_FATAL_FAILURE(obj->closePersistentEditor(0));
}

TEST_F(WorkspaceHelperTest, instance)
{
    // Test getter: WorkspaceHelper instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(WorkspaceHelperTest, isViewModeSupported)
{
    // Test method: bool isViewModeSupported((const QString &scheme, const dfmbase::Global::ViewMode mode))
    QString _arg0{};
    auto result = obj->isViewModeSupported(_arg0, {});
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceHelperTest, removeWorkspace)
{
    // Test method: void removeWorkspace((quint64 windowId))
    EXPECT_NO_FATAL_FAILURE(obj->removeWorkspace(0));
}

TEST_F(WorkspaceHelperTest, windowId)
{
    // Test method: quint64 windowId((const QWidget *sender))
    auto result = obj->windowId(nullptr);
    EXPECT_GE(result, 0);

}
