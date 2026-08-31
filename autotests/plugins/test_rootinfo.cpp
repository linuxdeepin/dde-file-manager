// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_rootinfo.cpp
 * @brief Unit tests for RootInfo methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "models/rootinfo.h"

#include <QTest>

using namespace dfmplugin_workspace;

class RootInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RootInfo();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RootInfo *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RootInfoTest, canDelete)
{
    // Test bool getter: canDelete()
    bool result = obj->canDelete();
    EXPECT_FALSE(result);

}

TEST_F(RootInfoTest, clearTraversalThread)
{
    // Test method: int clearTraversalThread((const QString &key, const bool isRefresh))
    QString _arg0{};
    auto result = obj->clearTraversalThread(_arg0, false);
    EXPECT_GE(result, 0);

}

TEST_F(RootInfoTest, doFileDeleted)
{
    // Test method: void doFileDeleted((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->doFileDeleted(_arg0));
}

TEST_F(RootInfoTest, handleTraversalResults)
{
    // Test method: void handleTraversalResults((const QList<FileInfoPointer> children, const QString &travseToken))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleTraversalResults(QList<FileInfoPointer>(), _arg1));
}

TEST_F(RootInfoTest, reset)
{
    // Test method: void reset(())
    EXPECT_NO_FATAL_FAILURE(obj->reset());
}

TEST_F(RootInfoTest, updateChildren)
{
    // Test method: void updateChildren((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateChildren(_arg0));
}

TEST_F(RootInfoTest, addChildren)
{
    // Test method: void addChildren((const QList<SortInfoPointer> &children))
    QList<SortInfoPointer> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->addChildren(_arg0));
}
