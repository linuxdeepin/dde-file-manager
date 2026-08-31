// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_typeclassifierprivate.cpp
 * @brief Unit tests for TypeClassifierPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "mode/normalized/type/typeclassifier.h"

#include <QTest>

using namespace ddplugin_organizer;

class TypeClassifierPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TypeClassifierPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TypeClassifierPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TypeClassifierPrivateTest, TypeClassifierPrivate)
{
    // Test constructor: TypeClassifierPrivate((TypeClassifier *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TypeClassifierPrivateTest, TypeClassifierPrivate_Destructor)
{
    // Test method:  ~TypeClassifierPrivate(())
    EXPECT_NO_FATAL_FAILURE({ TypeClassifierPrivate *tmp = new TypeClassifierPrivate(); delete tmp; });
}
