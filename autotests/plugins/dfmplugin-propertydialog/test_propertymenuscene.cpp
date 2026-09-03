// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
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
#include "events/propertyeventreceiver.h"
#include "dfmplugin_propertydialog_global.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/dfm_menu_defines.h>

DPPROPERTYDIALOG_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

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

TEST_F(TestPropertyMenuScene, PropertyMenuSceneFullFlow)
{
    UrlRoute::regScheme(Global::Scheme::kFile, "/");
    InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

    // Intercept dialog handling so no real property dialog is created.
    stub.set_lamda(&PropertyEventReceiver::handleShowPropertyDialog,
                   [](PropertyEventReceiver *, const QList<QUrl> &, const QVariantHash &) {
                   });

    PropertyMenuScene scene;
    QVariantHash params;
    params.insert(MenuParamKey::kCurrentDir, QUrl::fromLocalFile("/tmp"));
    params.insert(MenuParamKey::kSelectFiles,
                  QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp") }));
    params.insert(MenuParamKey::kIsEmptyArea, false);
    ASSERT_TRUE(scene.initialize(params));

    QMenu menu;
    ASSERT_TRUE(scene.create(&menu));

    // The menu must contain the property action; scene() must map it back to
    // this scene and triggered() must forward it to the event receiver.
    bool foundPropertyAction = false;
    for (QAction *act : menu.actions()) {
        if (act->property(ActionPropertyKey::kActionID).toString() == PropertyActionId::kProperty) {
            foundPropertyAction = true;
            EXPECT_EQ(scene.scene(act), &scene);
            EXPECT_TRUE(scene.triggered(act));
        }
    }
    EXPECT_TRUE(foundPropertyAction);

    // updateState reorders the menu (updateMenu) without altering actions.
    scene.updateState(&menu);
    EXPECT_FALSE(menu.actions().isEmpty());
}

TEST_F(TestPropertyMenuScene, PropertyMenuSceneUpdateStateMissingFile)
{
    UrlRoute::regScheme(Global::Scheme::kFile, "/");
    InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

    PropertyMenuScene scene;
    QVariantHash params;
    params.insert(MenuParamKey::kCurrentDir, QUrl::fromLocalFile("/tmp"));
    params.insert(MenuParamKey::kSelectFiles,
                  QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/no_such_mfscene_file.txt") }));
    params.insert(MenuParamKey::kIsEmptyArea, false);
    ASSERT_TRUE(scene.initialize(params));

    QMenu menu;
    ASSERT_TRUE(scene.create(&menu));
    scene.updateState(&menu);

    // The property action must be disabled when the focus file does not exist.
    bool foundPropertyAction = false;
    for (QAction *act : menu.actions()) {
        if (act->property(ActionPropertyKey::kActionID).toString() == PropertyActionId::kProperty) {
            foundPropertyAction = true;
            EXPECT_FALSE(act->isEnabled());
        }
    }
    EXPECT_TRUE(foundPropertyAction);
}