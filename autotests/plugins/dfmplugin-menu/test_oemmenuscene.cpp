// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "oemmenuscene/oemmenuscene.h"
#include "oemmenuscene/oemmenu.h"
#include "oemmenuscene/private/oemmenuscene_p.h"
#include "oemmenuscene/private/oemmenu_p.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>

#include <gtest/gtest.h>

#include <QMenu>
#include <QUrl>
#include <QAction>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;

class UT_OemMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // avoid real inotify watchers created by OemMenuPrivate's ctor
        stub.set_lamda(VADDR(LocalFileWatcher, startWatcher), []() {
            __DBG_STUB_INVOKE__
            return true;
        });

        menu = new OemMenu;
        scene = new OemMenuScene(menu);
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    }

    virtual void TearDown() override
    {
        delete scene;
        scene = nullptr;
        delete menu;
        menu = nullptr;
        stub.clear();
    }

protected:
    OemMenuScene *scene { nullptr };
    OemMenu *menu { nullptr };
    stub_ext::StubExt stub;
};

class UT_OemMenuCreator : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        creator = new OemMenuCreator();
    }

    virtual void TearDown() override
    {
        delete creator;
        creator = nullptr;
    }

protected:
    OemMenuCreator *creator { nullptr };
};

// ================= migrated legacy cases =================

TEST_F(UT_OemMenuCreator, Name_ReturnsCorrectName)
{
    EXPECT_EQ(OemMenuCreator::name(), "OemMenu");
}

TEST_F(UT_OemMenuCreator, Create_ReturnsOemMenuScene)
{
    auto scene = creator->create();
    EXPECT_NE(scene, nullptr);
    EXPECT_EQ(scene->name(), "OemMenu");
    delete scene;
}

TEST_F(UT_OemMenuScene, Name_ReturnsCorrectName)
{
    EXPECT_EQ(scene->name(), "OemMenu");
}

TEST_F(UT_OemMenuScene, Initialize_ValidParams_ReturnsTrue)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kOnDesktop] = false;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_FALSE(scene->initialize(params));
}

TEST_F(UT_OemMenuScene, Scene_NullAction_ReturnsNull)
{
    EXPECT_EQ(scene->scene(nullptr), nullptr);
}

// ============ extended cases ============

TEST_F(UT_OemMenuScene, Initialize_EmptyAreaValidParams_ReturnsTrue)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kOnDesktop] = false;
    params[MenuParamKey::kIsEmptyArea] = true;

    EXPECT_TRUE(scene->initialize(params));
}

TEST_F(UT_OemMenuScene, Create_EmptyAreaNoOemActions_ReturnsTrue)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kOnDesktop] = false;
    params[MenuParamKey::kIsEmptyArea] = true;
    ASSERT_TRUE(scene->initialize(params));

    // make sure no machine dependent oem actions are loaded
    menu->d.data()->actionListByType.clear();

    QMenu parentMenu;
    EXPECT_NO_FATAL_FAILURE(EXPECT_TRUE(scene->create(&parentMenu)));
    EXPECT_TRUE(parentMenu.actions().isEmpty());
}

TEST_F(UT_OemMenuScene, Create_FocusModeEmptyActions_ReturnsTrue)
{
    scene->d->isEmptyArea = false;
    scene->d->focusFile = QUrl::fromLocalFile("/tmp/focus.txt");
    scene->d->selectFiles = { scene->d->focusFile };

    menu->d.data()->actionListByType.clear();

    QMenu parentMenu;
    EXPECT_NO_FATAL_FAILURE(EXPECT_TRUE(scene->create(&parentMenu)));
}

TEST_F(UT_OemMenuScene, UpdateState_NullParent_ReturnsImmediately)
{
    EXPECT_NO_FATAL_FAILURE(scene->updateState(nullptr));
}

TEST_F(UT_OemMenuScene, UpdateState_ValidParent_DelegatesToBase)
{
    QMenu parentMenu;
    EXPECT_NO_FATAL_FAILURE(scene->updateState(&parentMenu));
}

TEST_F(UT_OemMenuScene, Triggered_NotOwnAction_DelegatesToBase)
{
    QAction foreign("foreign");
    EXPECT_FALSE(scene->triggered(&foreign));
}

TEST_F(UT_OemMenuScene, Triggered_OwnAction_ExecutesCommand)
{
    QAction own("own");
    own.setProperty("Exec", "/bin/true arg");
    scene->d->oemActions.append(&own);

    bool invoked = false;
    stub.set_lamda(ADDR(UniversalUtils, runCommand),
                   [&invoked](const QString &, const QStringList &, const QString &) -> bool {
                       __DBG_STUB_INVOKE__
                       invoked = true;
                       return true;
                   });

    EXPECT_TRUE(scene->triggered(&own));
    EXPECT_TRUE(invoked);
}

TEST_F(UT_OemMenuScene, Triggered_OwnActionEmptyCommand_DelegatesToBase)
{
    QAction own("own");
    scene->d->oemActions.append(&own);

    EXPECT_FALSE(scene->triggered(&own));
}

TEST_F(UT_OemMenuScene, Triggered_OwnChildAction_ExecutesCommand)
{
    QAction child("child");
    child.setProperty("Exec", "/bin/true");
    scene->d->oemChildActions.append(&child);

    bool invoked = false;
    stub.set_lamda(ADDR(UniversalUtils, runCommand),
                   [&invoked](const QString &, const QStringList &, const QString &) -> bool {
                       __DBG_STUB_INVOKE__
                       invoked = true;
                       return true;
                   });

    EXPECT_TRUE(scene->triggered(&child));
    EXPECT_TRUE(invoked);
}

TEST_F(UT_OemMenuScene, Scene_OwnAction_ReturnsSelf)
{
    QAction own("own");
    scene->d->oemActions.append(&own);
    EXPECT_EQ(scene->scene(&own), scene);
}

TEST_F(UT_OemMenuScene, Scene_OwnChildAction_ReturnsSelf)
{
    QAction child("child");
    scene->d->oemChildActions.append(&child);
    EXPECT_EQ(scene->scene(&child), scene);
}

TEST_F(UT_OemMenuScene, Scene_ForeignAction_DelegatesToBase)
{
    QAction foreign("foreign");
    EXPECT_EQ(scene->scene(&foreign), nullptr);
}

TEST_F(UT_OemMenuScene, ChildActions_WithMenu_ReturnsAllChildActions)
{
    QMenu subMenu;
    QAction child("child");
    subMenu.addAction(&child);

    QAction parent("parent");
    parent.setMenu(&subMenu);

    auto actions = scene->d->childActions(&parent);
    EXPECT_EQ(actions.size(), 1);
    EXPECT_EQ(actions.first(), &child);
}

TEST_F(UT_OemMenuScene, ChildActions_NoMenu_ReturnsEmpty)
{
    QAction plain("plain");
    EXPECT_TRUE(scene->d->childActions(&plain).isEmpty());
}
