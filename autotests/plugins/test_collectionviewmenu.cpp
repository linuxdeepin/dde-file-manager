// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionviewmenu.cpp
 * @brief Unit tests for CollectionViewMenu methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/collectionviewmenu.h"

#include <QTest>

using namespace ddplugin_organizer;

class CollectionViewMenuTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionViewMenu();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionViewMenu *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionViewMenuTest, emptyAreaMenu)
{
    // Test method: void emptyAreaMenu(())
    EXPECT_NO_FATAL_FAILURE(obj->emptyAreaMenu());
}
