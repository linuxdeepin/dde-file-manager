// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagproperty_1.cpp
 * @brief Unit tests for TagProperty methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "tools/upgrade/units/beans/tagproperty.h"

#include <QTest>

using namespace src;

class TagPropertyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagProperty();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagProperty *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagPropertyTest, getAmbiguity)
{
    // Test getter: int getAmbiguity()
    auto result = obj->getAmbiguity();
    EXPECT_EQ(result, 0);

}

TEST_F(TagPropertyTest, getFuture)
{
    // Test getter: QString getFuture()
    auto result = obj->getFuture();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagPropertyTest, setFuture)
{
    // Test setter: void setFuture((const QString &value))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setFuture(_arg0));
}
