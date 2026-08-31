// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_shredfilemodel.cpp
 * @brief Unit tests for ShredFileModel methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "shred/shredfilemodel.h"

#include <QTest>

using namespace dfmplugin_utils;

class ShredFileModelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShredFileModel();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShredFileModel *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShredFileModelTest, data)
{
    // Test method: QVariant data((const QModelIndex &index, int role))
    QModelIndex _arg0{};
    auto result = obj->data(_arg0, 0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(ShredFileModelTest, index)
{
    // Test method: QModelIndex index((int row, int column, const QModelIndex &parent))
    QModelIndex _arg2{};
    auto result = obj->index(0, 0, _arg2);
    EXPECT_FALSE(result.isValid());

}

TEST_F(ShredFileModelTest, parent)
{
    // Test method: QModelIndex parent((const QModelIndex &child))
    QModelIndex _arg0{};
    auto result = obj->parent(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(ShredFileModelTest, setFileList)
{
    // Test setter: void setFileList((const QList<QUrl> &fileList))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setFileList(_arg0));
}
