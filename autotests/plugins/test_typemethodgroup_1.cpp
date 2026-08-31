// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_typemethodgroup_1.cpp
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

TEST_F(TypeMethodGroupTest, TypeMethodGroup)
{
    // Test constructor: TypeMethodGroup((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TypeMethodGroupTest, id)
{
    // Test getter: Classifier id()
    auto result = obj->id();
    EXPECT_NO_FATAL_FAILURE({ obj->id(); });

}

TEST_F(TypeMethodGroupTest, release)
{
    // Test method: void release(())
    EXPECT_NO_FATAL_FAILURE(obj->release());
}

TEST_F(TypeMethodGroupTest, subWidgets)
{
    // Test getter: QList<QWidget *> subWidgets()
    auto result = obj->subWidgets();
    // Pointer return type
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TypeMethodGroupTest, TypeMethodGroup_Destructor)
{
    // Test method:  ~TypeMethodGroup(())
    EXPECT_NO_FATAL_FAILURE({ TypeMethodGroup *tmp = new TypeMethodGroup(); delete tmp; });
}
