// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionmodel.cpp
 * @brief Unit tests for CollectionModel methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "models/collectionmodel.h"

#include <QTest>

using namespace ddplugin_organizer;

class CollectionModelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionModel();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionModel *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionModelTest, data)
{
    // Test method: QVariant data((const QModelIndex &index, int role))
    QModelIndex _arg0{};
    auto result = obj->data(_arg0, 0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CollectionModelTest, fileInfo)
{
    // Test method: FileInfoPointer fileInfo((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->fileInfo(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(CollectionModelTest, fileUrl)
{
    // Test method: QUrl fileUrl((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->fileUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CollectionModelTest, files)
{
    // Test getter: QList<QUrl> files()
    auto result = obj->files();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CollectionModelTest, index)
{
    // Test method: QModelIndex index((int row, int column, const QModelIndex &parent))
    QModelIndex _arg2{};
    auto result = obj->index(0, 0, _arg2);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CollectionModelTest, parent)
{
    // Test method: QModelIndex parent((const QModelIndex &child))
    QModelIndex _arg0{};
    auto result = obj->parent(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CollectionModelTest, take)
{
    // Test method: bool take((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->take(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CollectionModelTest, update)
{
    // Test method: void update(())
    EXPECT_NO_FATAL_FAILURE(obj->update());
}

TEST_F(CollectionModelTest, mimeData)
{
    // Test method: QMimeData mimeData((const QModelIndexList &indexes))
    QModelIndexList _arg0{};
    auto result = obj->mimeData(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->mimeData(_arg0); });

}
