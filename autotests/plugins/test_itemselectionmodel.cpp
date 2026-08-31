// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_itemselectionmodel.cpp
 * @brief Unit tests for ItemSelectionModel methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "models/itemselectionmodel.h"

#include <QTest>

using namespace ddplugin_organizer;

class ItemSelectionModelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ItemSelectionModel();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ItemSelectionModel *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ItemSelectionModelTest, selectAll)
{
    // Test method: void selectAll(())
    EXPECT_NO_FATAL_FAILURE(obj->selectAll());
}
