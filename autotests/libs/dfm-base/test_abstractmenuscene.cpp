// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractmenuscene.cpp
 * @brief Unit tests for AbstractMenuScene default implementations.
 */

#include <gtest/gtest.h>
#include <QMenu>
#include <QAction>
#include <QVariantHash>

#include <dfm-base/interfaces/abstractmenuscene.h>
#include "dfm-base/interfaces/private/abstractmenuscene_p.h"

using namespace dfmbase;

class TestMenuScene : public AbstractMenuScene
{
public:
    explicit TestMenuScene(const QString &n, QObject *parent = nullptr)
        : AbstractMenuScene(parent), m_name(n) { }
    QString name() const override { return m_name; }
    QString m_name;
};

TEST(AbstractMenuSceneTest, InitializeReturnsTrueWithEmptyParams)
{
    TestMenuScene scene("test");
    QVariantHash params;
    EXPECT_TRUE(scene.initialize(params));
}

TEST(AbstractMenuSceneTest, CreateWithNullParent)
{
    TestMenuScene scene("test");
    QMenu menu;
    EXPECT_NO_FATAL_FAILURE({ (void)scene.create(&menu); });
}

TEST(AbstractMenuSceneTest, UpdateStateNoCrash)
{
    TestMenuScene scene("test");
    QMenu menu;
    EXPECT_NO_FATAL_FAILURE({ scene.updateState(&menu); });
}

TEST(AbstractMenuSceneTest, TriggeredReturnsFalseWithNoChildren)
{
    TestMenuScene scene("test");
    QAction action("act");
    EXPECT_FALSE(scene.triggered(&action));
}

TEST(AbstractMenuSceneTest, ActionFilterReturnsFalse)
{
    TestMenuScene scene("test");
    QAction action("act");
    EXPECT_FALSE(scene.actionFilter(&scene, &action));
}

TEST(AbstractMenuSceneTest, SceneReturnsNullptrWithNoChildren)
{
    TestMenuScene scene("test");
    QAction action("act");
    EXPECT_EQ(scene.scene(&action), nullptr);
}

TEST(AbstractMenuSceneTest, AddSubsceneSucceeds)
{
    TestMenuScene scene("parent");
    auto *child = new TestMenuScene("child");
    EXPECT_TRUE(scene.addSubscene(child));
    EXPECT_FALSE(scene.subscene().isEmpty());
}

TEST(AbstractMenuSceneTest, AddSubsceneNullReturnsFalse)
{
    TestMenuScene scene("parent");
    EXPECT_FALSE(scene.addSubscene(nullptr));
}

TEST(AbstractMenuSceneTest, RemoveSubscene)
{
    TestMenuScene scene("parent");
    auto *child = new TestMenuScene("child");
    scene.addSubscene(child);
    EXPECT_NO_FATAL_FAILURE({ scene.removeSubscene(child); });
    EXPECT_TRUE(scene.subscene().isEmpty());
}

TEST(AbstractMenuSceneTest, SetSubscene)
{
    TestMenuScene scene("parent");
    auto *c1 = new TestMenuScene("c1");
    auto *c2 = new TestMenuScene("c2");
    QList<AbstractMenuScene *> subs { c1, c2 };
    EXPECT_NO_FATAL_FAILURE({ scene.setSubscene(subs); });
    EXPECT_EQ(scene.subscene().size(), 2);
}

TEST(AbstractMenuSceneTest, FilterActionBySubsceneHelperNull)
{
    QAction action("act");
    EXPECT_FALSE(filterActionBySubscene(nullptr, &action));
}

// ---- Coverage additions: private init check + dtor ----

TEST(AbstractMenuSceneTest, InitializeParamsIsValidDefaultFalse)
{
    AbstractMenuScenePrivate priv(nullptr);
    // With isEmptyArea=false and empty selectFiles/focusFile/currentDir, returns false.
    EXPECT_FALSE(priv.initializeParamsIsValid());
}

TEST(AbstractMenuSceneTest, InitializeParamsIsValidWithEmptyAreaTrue)
{
    AbstractMenuScenePrivate priv(nullptr);
    priv.isEmptyArea = true;
    EXPECT_TRUE(priv.initializeParamsIsValid());
}
