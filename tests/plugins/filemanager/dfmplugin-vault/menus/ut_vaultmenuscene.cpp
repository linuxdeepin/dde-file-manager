// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "menus/vaultmenuscene.h"
#include "menus/vaultmenuscene_p.h"

#include <gtest/gtest.h>

#include <QVariantHash>
#include <QUrl>
#include <QAction>
#include <QMenu>

#include <dfm-base/dfm_menu_defines.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(UT_VaultMenuScene, name)
{
    VaultMenuScene scene;
    QString name = scene.name();

    EXPECT_TRUE(name == "VaultMenu");
}

TEST(UT_VaultMenuScene, initialize_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(AbstractMenuScenePrivate, initializeParamsIsValid), []{
        return false;
    });

    QVariantHash params;
    QList<QUrl> urls { QUrl("dfmvault:///") };
    QVariant vUrls = QVariant::fromValue<QList<QUrl>>(urls);
    params.insert(MenuParamKey::kSelectFiles, vUrls);
    VaultMenuScene scene;
    bool isOk = scene.initialize(params);

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultMenuScene, initialize_two)
{
    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(AbstractMenuScenePrivate, initializeParamsIsValid), []{
        return true;
    });
    stub.set_lamda(VADDR(AbstractMenuScene, initialize), []{
        return true;
    });


    QVariantHash params;
    QVariant vUrl = QVariant::fromValue<QUrl>(QUrl("dfmvault:///"));
    params.insert(MenuParamKey::kCurrentDir, vUrl);
    VaultMenuScene scene;
    bool isOk = scene.initialize(params);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultMenuScene, scene)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(AbstractMenuScene, scene), [ &isOk ]{
        isOk = true;
        return nullptr;
    });

    VaultMenuScene scene1;
    scene1.scene(nullptr);

    QAction action;
    VaultMenuScene scene2;
    scene2.scene(&action);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultMenuScene, create)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(AbstractMenuScene, create), [ &isOk ]{
        isOk = true;
        return true;
    });

    VaultMenuScene scene1;
    scene1.create(nullptr);

    QMenu menu;
    VaultMenuScene scene2;
    scene2.create(&menu);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultMenuScene, updateState)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(AbstractMenuScene, updateState), []{});
    stub.set_lamda(&VaultMenuScenePrivate::filterMenuAction, [ &isOk ]{
        isOk = true;
    });

    QMenu menu1;
    VaultMenuScene scene1;
    scene1.d->isEmptyArea = true;
    scene1.updateState(&menu1);

    QMenu menu2;
    VaultMenuScene scene2;
    scene2.d->isEmptyArea = false;
    scene2.updateState(&menu2);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultMenuScene, triggered)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(AbstractMenuScene, triggered), [ &isOk ]{
        isOk = true;
        return true;
    });

    QAction action;
    VaultMenuScene scene;
    scene.triggered(&action);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultMenuScenePrivate, filterMenuAction)
{
    QMenu menu1;
    QStringList actions1;
    VaultMenuScenePrivate scenePri1;
    scenePri1.filterMenuAction(&menu1, actions1);

    QMenu menu2;
    QAction *actionA = menu2.addAction("UT_TEST");
    actionA->setProperty(ActionPropertyKey::kActionID, "send-to");

    QMenu subMenu;
    actionA->setMenu(&subMenu);
    QAction *actionSub = subMenu.addAction("UT_TEST2");
    actionSub->setProperty(ActionPropertyKey::kActionID, "send-to-desktop");

    menu2.addSeparator();
    QStringList actions2;
    VaultMenuScenePrivate scenePri2;
    scenePri2.filterMenuAction(&menu2, actions2);
}
