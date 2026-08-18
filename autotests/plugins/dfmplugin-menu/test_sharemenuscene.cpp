// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "menuscene/sharemenuscene.h"
#include "menuscene/action_defines.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/systempathutil.h>

#include <gtest/gtest.h>

#include <QMenu>
#include <QAction>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;

class UT_ShareMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        scene = new ShareMenuScene();
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        // MenuUtils::perfectMenuParams is static inline and can not be stubbed,
        // neutralize its environment probes instead.
        stub.set_lamda(&FileUtils::isComputerDesktopFile, [](const QUrl &) {
            __DBG_STUB_INVOKE__
            return false;
        });
        stub.set_lamda(&FileUtils::isTrashDesktopFile, [](const QUrl &) {
            __DBG_STUB_INVOKE__
            return false;
        });
        stub.set_lamda(&FileUtils::isHomeDesktopFile, [](const QUrl &) {
            __DBG_STUB_INVOKE__
            return false;
        });
        stub.set_lamda(ADDR(SystemPathUtil, isSystemPath), [](SystemPathUtil *, QString) {
            __DBG_STUB_INVOKE__
            return false;
        });
    }

    virtual void TearDown() override
    {
        delete scene;
        scene = nullptr;
        stub.clear();
    }

protected:
    ShareMenuScene *scene { nullptr };
    stub_ext::StubExt stub;
};

class UT_ShareMenuCreator : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        creator = new ShareMenuCreator();
    }

    virtual void TearDown() override
    {
        delete creator;
        creator = nullptr;
    }

protected:
    ShareMenuCreator *creator { nullptr };
};

TEST_F(UT_ShareMenuCreator, Name_ReturnsShareMenuName)
{
    EXPECT_EQ(ShareMenuCreator::name(), "ShareMenu");
}

TEST_F(UT_ShareMenuCreator, Create_ReturnsShareMenuScene)
{
    auto scene = creator->create();
    EXPECT_NE(scene, nullptr);
    EXPECT_EQ(scene->name(), "ShareMenu");
    delete scene;
}

TEST_F(UT_ShareMenuScene, Name_ReturnsShareMenuName)
{
    EXPECT_EQ(scene->name(), "ShareMenu");
}

TEST_F(UT_ShareMenuScene, Initialize_EmptySelectFiles_ReturnsFalse)
{
    QVariantHash params;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>());

    EXPECT_FALSE(scene->initialize(params));
}

TEST_F(UT_ShareMenuScene, Initialize_ValidFiles_ReturnsTrue)
{
    QUrl testUrl = QUrl::fromLocalFile("/tmp/testdir");
    QList<QUrl> urls = { testUrl };

    QVariantHash params;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });
    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_TRUE(scene->initialize(params));
}

TEST_F(UT_ShareMenuScene, Scene_NullAction_ReturnsNull)
{
    EXPECT_EQ(scene->scene(nullptr), nullptr);
}

TEST_F(UT_ShareMenuScene, Create_NullParent_ReturnsFalse)
{
    EXPECT_FALSE(scene->create(nullptr));
}

TEST_F(UT_ShareMenuScene, Create_NotEmptyArea_AddsHiddenShareAction)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/testdir") });
    params[MenuParamKey::kIsEmptyArea] = false;
    params[MenuParamKey::kIsSystemPathIncluded] = false;
    params[MenuParamKey::kIsDDEDesktopFileIncluded] = false;
    params[MenuParamKey::kIsFocusOnDDEDesktopFile] = false;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    scene->initialize(params);

    QMenu menu;
    EXPECT_TRUE(scene->create(&menu));

    QAction *shareAction = nullptr;
    for (QAction *action : menu.actions()) {
        if (action->property(ActionPropertyKey::kActionID).toString() == QString(ActionID::kShare))
            shareAction = action;
    }
    ASSERT_NE(shareAction, nullptr);
    // the share sub menu is still empty at this point, so the action is hidden
    EXPECT_FALSE(shareAction->isVisible());
}

TEST_F(UT_ShareMenuScene, Create_EmptyArea_NoShareAction)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/testdir") });
    params[MenuParamKey::kIsEmptyArea] = true;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    scene->initialize(params);

    QMenu menu;
    EXPECT_TRUE(scene->create(&menu));
    EXPECT_TRUE(menu.actions().isEmpty());
}

TEST_F(UT_ShareMenuScene, Create_FocusOnDDEDesktopFile_NoShareAction)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/dde-computer.desktop") });
    params[MenuParamKey::kIsEmptyArea] = false;
    params[MenuParamKey::kIsSystemPathIncluded] = false;
    params[MenuParamKey::kIsDDEDesktopFileIncluded] = true;
    params[MenuParamKey::kIsFocusOnDDEDesktopFile] = true;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    scene->initialize(params);

    QMenu menu;
    EXPECT_TRUE(scene->create(&menu));
    // focused on a dde desktop file: create returns early, no share action
    EXPECT_TRUE(menu.actions().isEmpty());
}

TEST_F(UT_ShareMenuScene, Create_SystemPathIncluded_NoShareAction)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/usr/share");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/usr/share/testdir") });
    params[MenuParamKey::kIsEmptyArea] = false;
    params[MenuParamKey::kIsSystemPathIncluded] = true;
    params[MenuParamKey::kIsDDEDesktopFileIncluded] = false;
    params[MenuParamKey::kIsFocusOnDDEDesktopFile] = false;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    scene->initialize(params);

    QMenu menu;
    EXPECT_TRUE(scene->create(&menu));
    EXPECT_TRUE(menu.actions().isEmpty());
}

TEST_F(UT_ShareMenuScene, Scene_OwnShareAction_ReturnsThis)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/testdir") });
    params[MenuParamKey::kIsEmptyArea] = false;
    params[MenuParamKey::kIsSystemPathIncluded] = false;
    params[MenuParamKey::kIsDDEDesktopFileIncluded] = false;
    params[MenuParamKey::kIsFocusOnDDEDesktopFile] = false;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);

    QAction *shareAction = nullptr;
    for (QAction *action : menu.actions()) {
        if (action->property(ActionPropertyKey::kActionID).toString() == QString(ActionID::kShare))
            shareAction = action;
    }
    ASSERT_NE(shareAction, nullptr);
    EXPECT_EQ(scene->scene(shareAction), scene);
}

TEST_F(UT_ShareMenuScene, Scene_UnknownAction_DelegatesToBase)
{
    QAction unknownAction("unknown");
    // not an own action and no subscenes: base returns nullptr
    EXPECT_EQ(scene->scene(&unknownAction), nullptr);
}

TEST_F(UT_ShareMenuScene, UpdateState_DelegatesToBase)
{
    QMenu menu;

    bool baseCalled = false;
    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [&baseCalled](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        baseCalled = true;
    });

    EXPECT_NO_FATAL_FAILURE(scene->updateState(&menu));
    EXPECT_TRUE(baseCalled);
}

TEST_F(UT_ShareMenuScene, Triggered_NotOwnAction_DelegatesToBase)
{
    QAction unknownAction("unknown");

    bool baseCalled = false;
    stub.set_lamda(VADDR(AbstractMenuScene, triggered), [&baseCalled](AbstractMenuScene *, QAction *) {
        __DBG_STUB_INVOKE__
        baseCalled = true;
        return true;
    });

    EXPECT_TRUE(scene->triggered(&unknownAction));
    EXPECT_TRUE(baseCalled);
}

TEST_F(UT_ShareMenuScene, Triggered_ShareAction_DelegatesToBase)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/testdir") });
    params[MenuParamKey::kIsEmptyArea] = false;
    params[MenuParamKey::kIsSystemPathIncluded] = false;
    params[MenuParamKey::kIsDDEDesktopFileIncluded] = false;
    params[MenuParamKey::kIsFocusOnDDEDesktopFile] = false;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool baseCalled = false;
    stub.set_lamda(VADDR(AbstractMenuScene, triggered), [&baseCalled](AbstractMenuScene *, QAction *) {
        __DBG_STUB_INVOKE__
        baseCalled = true;
        return true;
    });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);

    QAction *shareAction = nullptr;
    for (QAction *action : menu.actions()) {
        if (action->property(ActionPropertyKey::kActionID).toString() == QString(ActionID::kShare))
            shareAction = action;
    }
    ASSERT_NE(shareAction, nullptr);

    // even the own share action is only forwarded to the base implementation
    EXPECT_TRUE(scene->triggered(shareAction));
    EXPECT_TRUE(baseCalled);
}
