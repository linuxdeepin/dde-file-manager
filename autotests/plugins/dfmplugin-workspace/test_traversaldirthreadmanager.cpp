// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "utils/traversaldirthreadmanager.h"

#include <QUrl>
#include <QStringList>
#include <QDir>
#include <QElapsedTimer>

using namespace dfmplugin_workspace;

class TraversalDirThreadManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        testUrl = QUrl::fromLocalFile("/tmp/test");
        nameFilters << "*.txt" << "*.pdf";
        filters = QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot;
        manager = new TraversalDirThreadManager(testUrl, nameFilters, filters);
    }

    void TearDown() override
    {
        delete manager;
        stub.clear();
    }

    QUrl testUrl;
    QStringList nameFilters;
    QDir::Filters filters;
    TraversalDirThreadManager *manager;
    stub_ext::StubExt stub;
};

TEST_F(TraversalDirThreadManagerTest, Constructor_CreatesInstance)
{
    EXPECT_NE(manager, nullptr);
}

TEST_F(TraversalDirThreadManagerTest, SetSortAgruments_SetsSortArguments)
{
    Qt::SortOrder order = Qt::DescendingOrder;
    dfmbase::Global::ItemRoles role = dfmbase::Global::ItemRoles::kItemDisplayRole;
    bool isMixDirAndFile = true;
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        manager->setSortAgruments(order, role, isMixDirAndFile);
    });
}

TEST_F(TraversalDirThreadManagerTest, SetTraversalToken_SetsToken)
{
    QString token = "test_token_123";
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        manager->setTraversalToken(token);
    });
}

TEST_F(TraversalDirThreadManagerTest, Start_StartsThread)
{
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        manager->start();
    });
}

TEST_F(TraversalDirThreadManagerTest, IsRunning_ChecksRunning)
{
    bool result = manager->isRunning();
    
    // Should return false initially
    EXPECT_FALSE(result);
}

TEST_F(TraversalDirThreadManagerTest, OnAsyncIteratorOver_ProcessesIteratorOver)
{
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        manager->onAsyncIteratorOver();
    });
}
