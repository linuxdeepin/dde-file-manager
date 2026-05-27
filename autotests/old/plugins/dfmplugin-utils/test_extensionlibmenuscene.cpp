// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/menuimpl/extensionlibmenuscene.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/pluginsload/extensionpluginmanager.h"

#include <dfm-base/dfm_menu_defines.h>

#include <QMenu>
#include <QAction>
#include <QUrl>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE

// ========== ExtensionLibMenuSceneCreator Tests ==========

class UT_ExtensionLibMenuSceneCreator : public testing::Test
{
protected:
    void SetUp() override
    {
        creator = new ExtensionLibMenuSceneCreator();
    }

    void TearDown() override
    {
        delete creator;
        creator = nullptr;
        stub.clear();
    }

    ExtensionLibMenuSceneCreator *creator { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_ExtensionLibMenuSceneCreator, name_ReturnsExtensionLibMenu)
{
    EXPECT_EQ(ExtensionLibMenuSceneCreator::name(), "ExtensionLibMenu");
}

TEST_F(UT_ExtensionLibMenuSceneCreator, create_ReturnsNewScene)
{
    AbstractMenuScene *scene = creator->create();

    EXPECT_NE(scene, nullptr);

    delete scene;
}

// ========== ExtensionLibMenuScene Tests ==========

class UT_ExtensionLibMenuScene : public testing::Test
{
protected:
    void SetUp() override
    {
        scene = new ExtensionLibMenuScene();
    }

    void TearDown() override
    {
        delete scene;
        scene = nullptr;
        stub.clear();
    }

    ExtensionLibMenuScene *scene { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_ExtensionLibMenuScene, Constructor_CreatesScene)
{
    EXPECT_NE(scene, nullptr);
}

TEST_F(UT_ExtensionLibMenuScene, name_ReturnsExtensionLibMenu)
{
    EXPECT_EQ(scene->name(), "ExtensionLibMenu");
}

TEST_F(UT_ExtensionLibMenuScene, initialize_EmptyParams_ReturnsFalse)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/home");

    bool result = scene->initialize(params);

    EXPECT_FALSE(result);
}

TEST_F(UT_ExtensionLibMenuScene, initialize_ValidParams_ReturnsTrue)
{
    stub.set_lamda(ADDR(ExtensionPluginManager, currentState),
                   [] {
                       __DBG_STUB_INVOKE__
                       return ExtensionPluginManager::kInitialized;
                   });

    QVariantHash params;
    params.insert(MenuParamKey::kCurrentDir, QUrl::fromLocalFile("/tmp"));
    params.insert(MenuParamKey::kSelectFiles, QVariant::fromValue(QList<QUrl>({ QUrl::fromLocalFile("/tmp/test.txt") })));
    params.insert(MenuParamKey::kIsEmptyArea, false);

    bool result = scene->initialize(params);

    EXPECT_TRUE(result);
}

TEST_F(UT_ExtensionLibMenuScene, create_NullMenu_ReturnsFalse)
{
    bool result = scene->create(nullptr);

    EXPECT_FALSE(result);
}

TEST_F(UT_ExtensionLibMenuScene, create_PluginsNotInitialized_ReturnsFalse)
{
    stub.set_lamda(ADDR(ExtensionPluginManager, currentState),
                   [] {
                       __DBG_STUB_INVOKE__
                       return ExtensionPluginManager::kReady;
                   });

    QMenu menu;
    bool result = scene->create(&menu);

    EXPECT_FALSE(result);
}

TEST_F(UT_ExtensionLibMenuScene, triggered_CallsBaseClass)
{
    QAction action;

    bool result = scene->triggered(&action);

    EXPECT_FALSE(result);
}

TEST_F(UT_ExtensionLibMenuScene, scene_NullAction_ReturnsNull)
{
    AbstractMenuScene *result = scene->scene(nullptr);

    EXPECT_EQ(result, nullptr);
}

TEST_F(UT_ExtensionLibMenuScene, scene_UnknownAction_ReturnsNull)
{
    QAction unknownAction;

    AbstractMenuScene *result = scene->scene(&unknownAction);

    EXPECT_EQ(result, nullptr);
}

TEST_F(UT_ExtensionLibMenuScene, updateState_NullMenu_Returns)
{
    scene->updateState(nullptr);
}

