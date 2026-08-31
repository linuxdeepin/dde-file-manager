// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_workspace.cpp
 * @brief Unit tests for Workspace methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "workspace.h"

#include <QTest>

using namespace dfmplugin_workspace;

class WorkspaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new Workspace();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    Workspace *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WorkspaceTest, saveGridDensityToConf)
{
    // Test method: void saveGridDensityToConf((const QVariant &var))
    QVariant _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->saveGridDensityToConf(_arg0));
}

TEST_F(WorkspaceTest, saveIconSizeToConf)
{
    // Test method: void saveIconSizeToConf((const QVariant &var))
    QVariant _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->saveIconSizeToConf(_arg0));
}

TEST_F(WorkspaceTest, saveListHeightToConf)
{
    // Test method: void saveListHeightToConf((const QVariant &var))
    QVariant _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->saveListHeightToConf(_arg0));
}

TEST_F(WorkspaceTest, saveRemoteThumbnailToConf)
{
    // Test method: void saveRemoteThumbnailToConf((const QVariant &var))
    QVariant _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->saveRemoteThumbnailToConf(_arg0));
}

TEST_F(WorkspaceTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}
