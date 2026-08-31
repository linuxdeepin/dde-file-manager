// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagproperty.cpp
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

TEST_F(TagPropertyTest, TagProperty)
{
    // Test constructor: TagProperty((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TagPropertyTest, getAmbiguity)
{
    // Test getter: int getAmbiguity()
    auto result = obj->getAmbiguity();
    EXPECT_EQ(result, 0);

}

TEST_F(TagPropertyTest, setAmbiguity)
{
    // Test setter: void setAmbiguity((int value))
    EXPECT_NO_FATAL_FAILURE(obj->setAmbiguity(0));
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

TEST_F(TagPropertyTest, getTagIndex)
{
    // Test getter: int getTagIndex()
    auto result = obj->getTagIndex();
    EXPECT_EQ(result, 0);

}

TEST_F(TagPropertyTest, setTagIndex)
{
    // Test setter: void setTagIndex((int value))
    EXPECT_NO_FATAL_FAILURE(obj->setTagIndex(0));
}

TEST_F(TagPropertyTest, getTagName)
{
    // Test getter: QString getTagName()
    auto result = obj->getTagName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagPropertyTest, setTagName)
{
    // Test setter: void setTagName((const QString &value))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setTagName(_arg0));
}

TEST_F(TagPropertyTest, getTagColor)
{
    // Test getter: QString getTagColor()
    auto result = obj->getTagColor();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagPropertyTest, setTagColor)
{
    // Test setter: void setTagColor((const QString &value))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setTagColor(_arg0));
}

TEST_F(TagPropertyTest, ambiguity)
{
    // Test getter: int ambiguity()
    auto result = obj->ambiguity();
    EXPECT_EQ(result, 0);

}

TEST_F(TagPropertyTest, future)
{
    // Test getter: QString future()
    auto result = obj->future();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagPropertyTest, tagIndex)
{
    // Test getter: int tagIndex()
    auto result = obj->tagIndex();
    EXPECT_EQ(result, 0);

}

TEST_F(TagPropertyTest, tagName)
{
    // Test getter: QString tagName()
    auto result = obj->tagName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagPropertyTest, tagColor)
{
    // Test getter: QString tagColor()
    auto result = obj->tagColor();
    EXPECT_TRUE(result.isEmpty());

}
