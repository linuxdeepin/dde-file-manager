// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_shredfilemodel_1.cpp
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

TEST_F(ShredFileModelTest, columnCount)
{
    // Test method: int columnCount((const QModelIndex &parent))
    QModelIndex _arg0{};
    auto result = obj->columnCount(_arg0);
    EXPECT_EQ(result, 0);

}

TEST_F(ShredFileModelTest, rowCount)
{
    // Test method: int rowCount((const QModelIndex &parent))
    QModelIndex _arg0{};
    auto result = obj->rowCount(_arg0);
    EXPECT_EQ(result, 0);

}
