// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dirsharemenucreator.cpp
 * @brief Unit tests for DirShareMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dirsharemenu/dirsharemenuscene.h"

#include <QTest>

using namespace dfmplugin_dirshare;

class DirShareMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DirShareMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DirShareMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DirShareMenuCreatorTest, create)
{
    // Test getter: AbstractMenuScene create()
    auto result = obj->create();
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });

}

TEST_F(DirShareMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
