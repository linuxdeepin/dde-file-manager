// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasproxymodel.cpp
 * @brief Unit tests for CanvasProxyModel methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "model/canvasproxymodel.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasProxyModelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasProxyModel();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasProxyModel *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasProxyModelTest, data)
{
    // Test method: QVariant data((const QModelIndex &index, int itemRole))
    QModelIndex _arg0{};
    auto result = obj->data(_arg0, 0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasProxyModelTest, fileUrl)
{
    // Test method: QUrl fileUrl((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->fileUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasProxyModelTest, files)
{
    // Test getter: QList<QUrl> files()
    auto result = obj->files();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasProxyModelTest, index)
{
    // Test method: QModelIndex index((int row, int column, const QModelIndex &parent))
    QModelIndex _arg2{};
    auto result = obj->index(0, 0, _arg2);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasProxyModelTest, parent)
{
    // Test method: QModelIndex parent((const QModelIndex &child))
    QModelIndex _arg0{};
    auto result = obj->parent(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasProxyModelTest, setSourceModel)
{
    // Test setter: void setSourceModel((QAbstractItemModel *model))
    EXPECT_NO_FATAL_FAILURE(obj->setSourceModel(nullptr));
}

TEST_F(CanvasProxyModelTest, sort)
{
    // Test bool getter: sort()
    bool result = obj->sort();
    EXPECT_FALSE(result);

}

TEST_F(CanvasProxyModelTest, take)
{
    // Test method: bool take((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->take(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CanvasProxyModelTest, CanvasProxyModel)
{
    // Test constructor: CanvasProxyModel((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
