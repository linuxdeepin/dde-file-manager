// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractscenecreator.cpp
 * @brief Unit tests for AbstractSceneCreator (interfaces/abstractscenecreator.cpp)
 *        via a minimal concrete subclass. Covers ctor, dtor, addChild (empty /
 *        new / duplicate), removeChild (existing / missing), getChildren.
 */

#include <gtest/gtest.h>
#include <QString>
#include <QStringList>

#include <dfm-base/interfaces/abstractscenecreator.h>

using namespace dfmbase;

namespace {
class FakeSceneCreator : public AbstractSceneCreator
{
public:
    AbstractMenuScene *create() override { return nullptr; }
};
}   // namespace

TEST(AbstractSceneCreatorTest, CtorAndDtorAreSafe)
{
    {
        FakeSceneCreator c;
        EXPECT_TRUE(c.getChildren().isEmpty());
    }
    SUCCEED();
}

TEST(AbstractSceneCreatorTest, AddChildEmptyReturnsFalse)
{
    FakeSceneCreator c;
    EXPECT_FALSE(c.addChild(QString()));
}

TEST(AbstractSceneCreatorTest, AddChildNewReturnsTrue)
{
    FakeSceneCreator c;
    EXPECT_TRUE(c.addChild("scene1"));
    EXPECT_EQ(c.getChildren().size(), 1);
    EXPECT_TRUE(c.getChildren().contains("scene1"));
}

TEST(AbstractSceneCreatorTest, AddChildDuplicateIsIdempotent)
{
    FakeSceneCreator c;
    c.addChild("scene1");
    EXPECT_TRUE(c.addChild("scene1"));   // returns true but doesn't duplicate
    EXPECT_EQ(c.getChildren().size(), 1);
}

TEST(AbstractSceneCreatorTest, AddMultipleChildren)
{
    FakeSceneCreator c;
    c.addChild("a");
    c.addChild("b");
    c.addChild("c");
    EXPECT_EQ(c.getChildren().size(), 3);
}

TEST(AbstractSceneCreatorTest, RemoveChildExisting)
{
    FakeSceneCreator c;
    c.addChild("scene1");
    c.addChild("scene2");
    c.removeChild("scene1");
    EXPECT_EQ(c.getChildren().size(), 1);
    EXPECT_FALSE(c.getChildren().contains("scene1"));
}

TEST(AbstractSceneCreatorTest, RemoveChildMissingIsNoOp)
{
    FakeSceneCreator c;
    c.addChild("scene1");
    c.removeChild("nonexistent");
    EXPECT_EQ(c.getChildren().size(), 1);
}

TEST(AbstractSceneCreatorTest, RemoveChildEmptyIsNoOp)
{
    FakeSceneCreator c;
    c.addChild("scene1");
    c.removeChild(QString());
    EXPECT_EQ(c.getChildren().size(), 1);
}
