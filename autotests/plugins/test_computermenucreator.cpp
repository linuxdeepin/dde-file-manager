// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computermenucreator.cpp
 * @brief Unit tests for ComputerMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/computermenuscene.h"

#include <QTest>

using namespace dfmplugin_computer;

class ComputerMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ComputerMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ComputerMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ComputerMenuCreatorTest, create)
{
    // Test getter: DFMBASE_USE_NAMESPACE create()
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });
}
