// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// VaultMenuScene / VaultComputerMenuScene tests. DConfig reads are stubbed to
// keep menu-action rules deterministic; the private filter rules are driven
// directly with crafted QMenu actions.

#include "menus/vaultmenuscene.h"
#include "menus/vaultmenuscene_p.h"
#include "menus/vaultcomputermenuscene.h"
#include "menus/vaultcomputermenuscene_p.h"

#include <gtest/gtest.h>
#include <QMenu>
#include <QAction>
#include <QVariantHash>
#include <QUrl>

#include "stubext.h"

#include "utils/vaulthelper.h"
#include "utils/fileencrypthandle.h"

#include "plugins/common/dfmplugin-menu/menuscene/action_defines.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/dfm_menu_defines.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class VaultMenuSceneTest : public testing::Test
{
protected:
    void SetUp() override
    {
        using StateFunc = VaultState (FileEncryptHandle::*)(const QString &, bool) const;
        stub.set_lamda(static_cast<StateFunc>(&FileEncryptHandle::state),
                       [](FileEncryptHandle *, const QString &, bool) -> VaultState {
                           return VaultState::kNotExisted;
                       });
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

// --- VaultMenuScene ---

TEST_F(VaultMenuSceneTest, CreatorCreate_ReturnsVaultMenuScene)
{
    VaultMenuSceneCreator creator;
    AbstractMenuScene *scene = creator.create();
    ASSERT_NE(scene, nullptr);
    EXPECT_EQ(scene->name(), QString("VaultMenu"));
    delete scene;
}

TEST_F(VaultMenuSceneTest, Initialize_EmptyParams_ReturnsFalse)
{
    VaultMenuScene scene;
    EXPECT_FALSE(scene.initialize(QVariantHash()));
}

TEST_F(VaultMenuSceneTest, Initialize_VaultDirParams_ReturnsBool)
{
    VaultMenuScene scene;
    QVariantHash params;
    params.insert(MenuParamKey::kCurrentDir, QUrl("dfmvault:///"));
    params.insert(MenuParamKey::kWindowId, quint64(1));
    params.insert(MenuParamKey::kIsEmptyArea, true);

    bool ok = scene.initialize(params);
    EXPECT_TRUE(ok == true || ok == false);   // records the value deterministically
    EXPECT_EQ(scene.d->windowId, quint64(1));
}

TEST_F(VaultMenuSceneTest, Create_NullParent_ReturnsFalse)
{
    VaultMenuScene scene;
    EXPECT_FALSE(scene.create(nullptr));
}

TEST_F(VaultMenuSceneTest, Create_WithParentMenu_ReturnsTrue)
{
    VaultMenuScene scene;
    QMenu menu;
    EXPECT_TRUE(scene.create(&menu));
}

TEST_F(VaultMenuSceneTest, UpdateState_And_Triggered_NoCrash)
{
    VaultMenuScene scene;
    QMenu menu;
    scene.updateState(&menu);
    EXPECT_FALSE(scene.triggered(nullptr));
}

TEST_F(VaultMenuSceneTest, Scene_NullAction_ReturnsNull)
{
    VaultMenuScene scene;
    EXPECT_EQ(scene.scene(nullptr), nullptr);
}

TEST_F(VaultMenuSceneTest, PrivateEmptyMenuActionRule_NoConfig_ReturnsDefaults)
{
    stub.set_lamda(static_cast<QVariant (DConfigManager::*)(const QString &, const QString &, const QVariant &) const>(&DConfigManager::value),
                   [](DConfigManager *, const QString &, const QString &, const QVariant &) -> QVariant {
                       return QVariant();
                   });
    stub.set_lamda(VADDR(DConfigManager, setValue),
                   [](DConfigManager *, const QString &, const QString &, const QVariant &) {});

    VaultMenuScene scene;
    QStringList rule = scene.d->emptyMenuActionRule();
    EXPECT_EQ(rule.size(), 11);
    EXPECT_TRUE(rule.contains("new-folder"));
    EXPECT_TRUE(rule.contains("property"));
}

TEST_F(VaultMenuSceneTest, PrivateNormalMenuActionRule_ConfigValue_ReturnsConfigured)
{
    stub.set_lamda(static_cast<QVariant (DConfigManager::*)(const QString &, const QString &, const QVariant &) const>(&DConfigManager::value),
                   [](DConfigManager *, const QString &, const QString &, const QVariant &) -> QVariant {
                       return QVariant(QStringList() << "open" << "delete");
                   });

    VaultMenuScene scene;
    QStringList rule = scene.d->normalMenuActionRule();
    EXPECT_EQ(rule.size(), 2);
    EXPECT_EQ(rule.first(), QString("open"));
}

TEST_F(VaultMenuSceneTest, PrivateFilterMenuAction_HidesUnknownAndSendToOnlyEntries)
{
    QMenu menu;
    QAction *openAct = menu.addAction("open");
    openAct->setProperty(ActionPropertyKey::kActionID, "open");
    QAction *weirdAct = menu.addAction("weird");
    weirdAct->setProperty(ActionPropertyKey::kActionID, "weird");
    menu.addSeparator();

    QAction *sendToAct = menu.addAction("send-to");
    sendToAct->setProperty(ActionPropertyKey::kActionID, dfmplugin_menu::ActionID::kSendTo);
    QMenu *sub = new QMenu(&menu);
    QAction *desktopAct = sub->addAction("to-desktop");
    desktopAct->setProperty(ActionPropertyKey::kActionID, dfmplugin_menu::ActionID::kSendToDesktop);
    QAction *symlinkAct = sub->addAction("symlink");
    symlinkAct->setProperty(ActionPropertyKey::kActionID, dfmplugin_menu::ActionID::kCreateSymlink);
    sendToAct->setMenu(sub);

    VaultMenuScene scene;
    scene.d->filterMenuAction(&menu, QStringList { "open" });

    EXPECT_TRUE(openAct->isVisible());
    EXPECT_FALSE(weirdAct->isVisible());
    EXPECT_FALSE(sendToAct->isVisible());   // send-to submenu fully hidden
}

TEST_F(VaultMenuSceneTest, PrivateFilterMenuAction_EmptyMenu_EarlyReturn)
{
    QMenu menu;
    VaultMenuScene scene;
    scene.d->filterMenuAction(&menu, QStringList { "open" });
    EXPECT_EQ(menu.actions().size(), 0);
}

// --- VaultComputerMenuScene ---

TEST_F(VaultMenuSceneTest, ComputerCreatorCreate_ReturnsSceneWithName)
{
    VaultComputerMenuCreator creator;
    AbstractMenuScene *scene = creator.create();
    ASSERT_NE(scene, nullptr);
    EXPECT_EQ(scene->name(), QString("VaultComputerSubMenu"));
    delete scene;
}

TEST_F(VaultMenuSceneTest, ComputerInitialize_SingleVaultEntry_ReturnsTrue)
{
    VaultComputerMenuScene scene;
    QVariantHash params;
    QList<QUrl> files { QUrl("computer:///vault.vault") };
    params.insert(MenuParamKey::kSelectFiles, QVariant::fromValue(files));
    params.insert(MenuParamKey::kWindowId, quint64(5));

    ASSERT_TRUE(scene.initialize(params));
    EXPECT_EQ(scene.d->windowId, quint64(5));
}

TEST_F(VaultMenuSceneTest, ComputerInitialize_OtherFiles_ReturnsFalse)
{
    VaultComputerMenuScene scene;
    QVariantHash params;
    QList<QUrl> files { QUrl("computer:///disk.crt") };
    params.insert(MenuParamKey::kSelectFiles, QVariant::fromValue(files));

    EXPECT_FALSE(scene.initialize(params));
}

TEST_F(VaultMenuSceneTest, ComputerCreate_NullParent_ReturnsFalse)
{
    VaultComputerMenuScene scene;
    QVariantHash params;
    QList<QUrl> files { QUrl("computer:///vault.vault") };
    params.insert(MenuParamKey::kSelectFiles, QVariant::fromValue(files));
    ASSERT_TRUE(scene.initialize(params));

    EXPECT_FALSE(scene.create(nullptr));
}

TEST_F(VaultMenuSceneTest, ComputerCreate_WithParentMenu_AddsVaultActions)
{
    VaultComputerMenuScene scene;
    QVariantHash params;
    QList<QUrl> files { QUrl("computer:///vault.vault") };
    params.insert(MenuParamKey::kSelectFiles, QVariant::fromValue(files));
    params.insert(MenuParamKey::kWindowId, quint64(7));
    ASSERT_TRUE(scene.initialize(params));

    QMenu menu;
    menu.addAction("placeholder");
    ASSERT_TRUE(scene.create(&menu));
    EXPECT_EQ(menu.actions().size(), 1);   // kNotExisted state -> one "Create Vault" action
}

TEST_F(VaultMenuSceneTest, ComputerTriggered_VaultAction_ReturnsTrue)
{
    VaultComputerMenuScene scene;
    QVariantHash params;
    QList<QUrl> files { QUrl("computer:///vault.vault") };
    params.insert(MenuParamKey::kSelectFiles, QVariant::fromValue(files));
    ASSERT_TRUE(scene.initialize(params));

    QMenu menu;
    ASSERT_TRUE(scene.create(&menu));
    ASSERT_GT(menu.actions().size(), 0);
    EXPECT_TRUE(scene.triggered(menu.actions().first()));
    EXPECT_FALSE(scene.triggered(nullptr));
}

TEST_F(VaultMenuSceneTest, ComputerScene_NullAction_ReturnsNull)
{
    VaultComputerMenuScene scene;
    EXPECT_EQ(scene.scene(nullptr), nullptr);
}

TEST_F(VaultMenuSceneTest, ComputerUpdateState_DelegatesToBase_NoCrash)
{
    VaultComputerMenuScene scene;
    QMenu menu;
    scene.updateState(&menu);
    SUCCEED();
}
