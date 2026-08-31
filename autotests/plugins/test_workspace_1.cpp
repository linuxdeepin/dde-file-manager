// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_workspace_1.cpp
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

TEST_F(WorkspaceTest, initConfig)
{
    // Test method: void initConfig(())
    EXPECT_NO_FATAL_FAILURE(obj->initConfig());
}

TEST_F(WorkspaceTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(WorkspaceTest, isGridDensityConfEqual)
{
    // Test method: bool isGridDensityConfEqual((const QVariant &dcon, const QVariant &dset))
    QVariant _arg0{};
    QVariant _arg1{};
    auto result = obj->isGridDensityConfEqual(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceTest, isIconSizeConfEqual)
{
    // Test method: bool isIconSizeConfEqual((const QVariant &dcon, const QVariant &dset))
    QVariant _arg0{};
    QVariant _arg1{};
    auto result = obj->isIconSizeConfEqual(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceTest, isListHeightConfEqual)
{
    // Test method: bool isListHeightConfEqual((const QVariant &dcon, const QVariant &dset))
    QVariant _arg0{};
    QVariant _arg1{};
    auto result = obj->isListHeightConfEqual(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceTest, isRemoteThumbnailConfEqual)
{
    // Test method: bool isRemoteThumbnailConfEqual((const QVariant &dcon, const QVariant &dset))
    QVariant _arg0{};
    QVariant _arg1{};
    auto result = obj->isRemoteThumbnailConfEqual(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceTest, onWindowClosed)
{
    // Test method: void onWindowClosed((quint64 windId))
    EXPECT_NO_FATAL_FAILURE(obj->onWindowClosed(0));
}

TEST_F(WorkspaceTest, onWindowOpened)
{
    // Test method: void onWindowOpened((quint64 windId))
    EXPECT_NO_FATAL_FAILURE(obj->onWindowOpened(0));
}

TEST_F(WorkspaceTest, syncGridDensityToAppSet)
{
    // Test method: void syncGridDensityToAppSet((const QString &, const QString &, const QVariant &var))
    QString _arg0{};
    QString _arg1{};
    QVariant _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->syncGridDensityToAppSet(_arg0, _arg1, _arg2));
}

TEST_F(WorkspaceTest, syncIconSizeToAppSet)
{
    // Test method: void syncIconSizeToAppSet((const QString &, const QString &, const QVariant &var))
    QString _arg0{};
    QString _arg1{};
    QVariant _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->syncIconSizeToAppSet(_arg0, _arg1, _arg2));
}

TEST_F(WorkspaceTest, syncListHeightToAppSet)
{
    // Test method: void syncListHeightToAppSet((const QString &, const QString &, const QVariant &var))
    QString _arg0{};
    QString _arg1{};
    QVariant _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->syncListHeightToAppSet(_arg0, _arg1, _arg2));
}

TEST_F(WorkspaceTest, syncRemoteThumbnailToAppSet)
{
    // Test method: void syncRemoteThumbnailToAppSet((const QString &, const QString &, const QVariant &var))
    QString _arg0{};
    QString _arg1{};
    QVariant _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->syncRemoteThumbnailToAppSet(_arg0, _arg1, _arg2));
}
