// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_typemethodgroup.cpp
 * @brief Unit tests for TypeMethodGroup methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "options/methodgroup/typemethodgroup.h"

#include <QTest>

using namespace ddplugin_organizer;

class TypeMethodGroupTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TypeMethodGroup();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TypeMethodGroup *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TypeMethodGroupTest, build)
{
    // Test bool getter: build()
    bool result = obj->build();
    EXPECT_FALSE(result);

}
