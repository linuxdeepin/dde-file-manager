// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionviewprivate.cpp
 * @brief Unit tests for CollectionViewPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/collectionview.h"

#include <QTest>

using namespace ddplugin_organizer;

class CollectionViewPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionViewPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionViewPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionViewPrivateTest, checkTargetEnable)
{
    // Test method: bool checkTargetEnable((QDropEvent *event, const QUrl &targetUrl))
    QUrl _arg1{};
    auto result = obj->checkTargetEnable(nullptr, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(CollectionViewPrivateTest, checkXdndDirectSave)
{
    // Test method: bool checkXdndDirectSave((QDragEnterEvent *event))
    auto result = obj->checkXdndDirectSave(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CollectionViewPrivateTest, clearClipBoard)
{
    // Test method: void clearClipBoard(())
    EXPECT_NO_FATAL_FAILURE(obj->clearClipBoard());
}

TEST_F(CollectionViewPrivateTest, deleteFiles)
{
    // Test method: void deleteFiles(())
    EXPECT_NO_FATAL_FAILURE(obj->deleteFiles());
}

TEST_F(CollectionViewPrivateTest, drop)
{
    // Test method: bool drop((QDropEvent *event))
    auto result = obj->drop(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CollectionViewPrivateTest, dropBetweenCollection)
{
    // Test method: bool dropBetweenCollection((QDropEvent *event))
    auto result = obj->dropBetweenCollection(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CollectionViewPrivateTest, dropDirectSaveMode)
{
    // Test method: bool dropDirectSaveMode((QDropEvent *event))
    auto result = obj->dropDirectSaveMode(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CollectionViewPrivateTest, dropFromCanvas)
{
    // Test method: bool dropFromCanvas((QDropEvent *event))
    auto result = obj->dropFromCanvas(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CollectionViewPrivateTest, findIndex)
{
    // Test method: QModelIndex findIndex((const QString &key, bool matchStart, const QModelIndex &current, bool reverseOrder, bool excludeCurrent))
    QString _arg0{};
    QModelIndex _arg2{};
    auto result = obj->findIndex(_arg0, false, _arg2, false, false);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CollectionViewPrivateTest, selectCollection)
{
    // Test method: void selectCollection(())
    EXPECT_NO_FATAL_FAILURE(obj->selectCollection());
}

TEST_F(CollectionViewPrivateTest, selectItems)
{
    // Test method: void selectItems((const QList<QUrl> &fileUrl))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->selectItems(_arg0));
}

TEST_F(CollectionViewPrivateTest, selectRect)
{
    // Test method: void selectRect((const QRect &rect))
    QRect _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->selectRect(_arg0));
}

TEST_F(CollectionViewPrivateTest, showMenu)
{
    // Test method: void showMenu(())
    EXPECT_NO_FATAL_FAILURE(obj->showMenu());
}
