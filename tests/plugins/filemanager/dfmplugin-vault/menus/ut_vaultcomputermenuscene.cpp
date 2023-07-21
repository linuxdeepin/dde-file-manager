// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "menus/vaultcomputermenuscene.h"
#include "menus/vaultcomputermenuscene_p.h"
#include "utils/vaulthelper.h"

#include <gtest/gtest.h>

#include <QVariantHash>
#include <QUrl>
#include <QMenu>

#include <DMenu>

#include <dfm-base/dfm_menu_defines.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

TEST(UT_VaultComputerMenuScene, name)
{
    VaultComputerMenuScene scene;
    QString name = scene.name();

    EXPECT_TRUE(name == "VaultComputerSubMenu");
}

TEST(UT_VaultComputerMenuScene, initialize_one)
{
    QVariantHash params;
    params.insert(MenuParamKey::kWindowId, 123);
    QList<QUrl> urls { QUrl("dfmvault:///UT_TEST.vault") };
    QVariant vUrls = QVariant::fromValue<QList<QUrl>>(urls);
    params.insert(MenuParamKey::kSelectFiles, vUrls);

    VaultComputerMenuScene scene;
    bool isOk = scene.initialize(params);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultComputerMenuScene, initialize_two)
{
    QVariantHash params;
    params.insert(MenuParamKey::kWindowId, 123);
    QList<QUrl> urls { QUrl("dfmvault:///UT_TEST") };
    QVariant vUrls = QVariant::fromValue<QList<QUrl>>(urls);
    params.insert(MenuParamKey::kSelectFiles, vUrls);

    VaultComputerMenuScene scene;
    bool isOk = scene.initialize(params);

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultComputerMenuScene, create)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::createMenu, []{
        DMenu *menu = new DMenu;
        return menu;
    });

    QMenu menu;
    VaultComputerMenuScene scene;
    bool isOk = scene.create(&menu);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultComputerMenuScene, updateState)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [ &isOk ]{
        isOk = true;
    });

    QMenu menu;
    VaultComputerMenuScene scene;
    scene.updateState(&menu);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultComputerMenuScene, triggered_one)
{
    QAction action;
    VaultComputerMenuScene scene;
    scene.d->acts.push_back(&action);
    bool isOk = scene.triggered(&action);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultComputerMenuScene, triggered_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(AbstractMenuScene, triggered), [ &isOk ]{
        isOk = true;
        return true;
    });

    QAction action;
    VaultComputerMenuScene scene;
    scene.triggered(&action);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultComputerMenuScene, scene_one)
{
    VaultComputerMenuScene scene;
    AbstractMenuScene *result = scene.scene(nullptr);

    EXPECT_TRUE(result == nullptr);
}

TEST(UT_VaultComputerMenuScene, scene_two)
{
    QAction action;
    VaultComputerMenuScene scene;
    scene.d->predicateAction.insert("UT_TEST", &action);
    AbstractMenuScene *result = scene.scene(&action);

    EXPECT_TRUE(result == &scene);
}

TEST(UT_VaultComputerMenuScene, scene_three)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(AbstractMenuScene, scene), [ &isOk ]{
        isOk = true;
        return nullptr;
    });

    QAction action;
    VaultComputerMenuScene scene;
    AbstractMenuScene *result = scene.scene(&action);

    EXPECT_TRUE(isOk);
}
