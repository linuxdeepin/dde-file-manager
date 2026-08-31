// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasproxymodelprivate.cpp
 * @brief Unit tests for CanvasProxyModelPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "model/canvasproxymodel.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasProxyModelPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasProxyModelPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasProxyModelPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasProxyModelPrivateTest, clearMapping)
{
    // Test method: void clearMapping(())
    EXPECT_NO_FATAL_FAILURE(obj->clearMapping());
}

TEST_F(CanvasProxyModelPrivateTest, indexs)
{
    // Test method: QModelIndexList indexs((const QList<QUrl> &files))
    QList<QUrl> _arg0{};
    auto result = obj->indexs(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasProxyModelPrivateTest, removeFilter)
{
    // Test method: bool removeFilter((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->removeFilter(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CanvasProxyModelPrivateTest, resetFilter)
{
    // Test method: bool resetFilter((QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->resetFilter(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CanvasProxyModelPrivateTest, sortMainDesktopFile)
{
    // Test method: void sortMainDesktopFile((QList<QUrl> &files, Qt::SortOrder order))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sortMainDesktopFile(_arg0, Qt::SortOrder()));
}

TEST_F(CanvasProxyModelPrivateTest, sourceAboutToBeReset)
{
    // Test method: void sourceAboutToBeReset(())
    EXPECT_NO_FATAL_FAILURE(obj->sourceAboutToBeReset());
}

TEST_F(CanvasProxyModelPrivateTest, sourceDataChanged)
{
    // Test method: void sourceDataChanged((const QModelIndex &sourceTopleft, const QModelIndex &sourceBottomright, const QVector<int> &roles))
    QModelIndex _arg0{};
    QModelIndex _arg1{};
    QVector<int> _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->sourceDataChanged(_arg0, _arg1, _arg2));
}

TEST_F(CanvasProxyModelPrivateTest, sourceDataRenamed)
{
    // Test method: void sourceDataRenamed((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sourceDataRenamed(_arg0, _arg1));
}

TEST_F(CanvasProxyModelPrivateTest, sourceReset)
{
    // Test method: void sourceReset(())
    EXPECT_NO_FATAL_FAILURE(obj->sourceReset());
}

TEST_F(CanvasProxyModelPrivateTest, sourceRowsAboutToBeRemoved)
{
    // Test method: void sourceRowsAboutToBeRemoved((const QModelIndex &sourceParent, int start, int end))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sourceRowsAboutToBeRemoved(_arg0, 0, 0));
}

TEST_F(CanvasProxyModelPrivateTest, sourceRowsInserted)
{
    // Test method: void sourceRowsInserted((const QModelIndex &sourceParent, int start, int end))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sourceRowsInserted(_arg0, 0, 0));
}
