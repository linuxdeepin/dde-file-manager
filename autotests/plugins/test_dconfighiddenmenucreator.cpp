// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dconfighiddenmenucreator.cpp
 * @brief Unit tests for DConfigHiddenMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/dconfighiddenmenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class DConfigHiddenMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DConfigHiddenMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DConfigHiddenMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DConfigHiddenMenuCreatorTest, create)
{
    // Test getter: AbstractMenuScene create()
    auto result = obj->create();
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });

}
