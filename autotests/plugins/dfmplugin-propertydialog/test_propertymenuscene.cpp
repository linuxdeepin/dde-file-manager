// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QMenu>
#include <QAction>
#include "stubext.h"

#include "menu/propertymenuscene.h"
#include "menu/propertymenuscene_p.h"
#include "dfmplugin_propertydialog_global.h"

DPPROPERTYDIALOG_USE_NAMESPACE

class TestPropertyMenuScene : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

// Test PropertyMenuCreator class
TEST_F(TestPropertyMenuScene, PropertyMenuCreatorCreate)
{
    PropertyMenuCreator creator;
    AbstractMenuScene *scene = creator.create();
    EXPECT_NE(scene, nullptr);
    delete scene;
}

TEST_F(TestPropertyMenuScene, PropertyMenuCreatorName)
{
    QString name = PropertyMenuCreator::name();
    EXPECT_EQ(name, "PropertyMenu");
}

// Test PropertyMenuScenePrivate class
TEST_F(TestPropertyMenuScene, PropertyMenuScenePrivateConstructor)
{
    PropertyMenuScene *scene = new PropertyMenuScene();
    PropertyMenuScenePrivate *privateScene = new PropertyMenuScenePrivate(scene);
    EXPECT_NE(privateScene, nullptr);
    delete privateScene;
    delete scene;
}

// Test PropertyMenuScene class
TEST_F(TestPropertyMenuScene, PropertyMenuSceneConstructor)
{
    PropertyMenuScene *scene = new PropertyMenuScene();
    EXPECT_NE(scene, nullptr);
    delete scene;
}

TEST_F(TestPropertyMenuScene, PropertyMenuSceneName)
{
    PropertyMenuScene scene;
    QString name = scene.name();
    EXPECT_EQ(name, "PropertyMenu");
}

TEST_F(TestPropertyMenuScene, PropertyMenuSceneInitialize)
{
    PropertyMenuScene scene;
    QVariantHash params;
    // Test with empty params
    bool result = scene.initialize(params);
    EXPECT_FALSE(result); // Should fail with empty params
}

TEST_F(TestPropertyMenuScene, PropertyMenuSceneScene)
{
    PropertyMenuScene scene;
    QAction *action = nullptr;
    AbstractMenuScene *result = scene.scene(action);
    EXPECT_EQ(result, nullptr);
}

TEST_F(TestPropertyMenuScene, PropertyMenuSceneCreate)
{
    PropertyMenuScene scene;
    QMenu menu;
    bool result = scene.create(&menu);
    EXPECT_FALSE(result); // Should fail without proper initialization
}

TEST_F(TestPropertyMenuScene, PropertyMenuSceneUpdateState)
{
    PropertyMenuScene scene;
    QMenu menu;
    EXPECT_NO_THROW(scene.updateState(&menu));
}

TEST_F(TestPropertyMenuScene, PropertyMenuSceneTriggered)
{
    PropertyMenuScene scene;
    QAction action;
    bool result = scene.triggered(&action);
    EXPECT_FALSE(result); // Should return false for unknown action
}