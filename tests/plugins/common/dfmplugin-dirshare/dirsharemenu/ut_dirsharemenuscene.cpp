// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugins/common/dfmplugin-dirshare/dirsharemenu/dirsharemenuscene.h"
#include "plugins/common/dfmplugin-dirshare/private/dirsharemenuscene_p.h"
#include "plugins/common/dfmplugin-dirshare/utils/usersharehelper.h"

#include <dfm-base/dfm_menu_defines.h>

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>
#include <dfm-framework/event/eventchannel.h>

#include "stubext.h"

#include <gtest/gtest.h>

#include <QStandardPaths>
#include <QUrl>
#include <QMenu>

using namespace dfmplugin_dirshare;
DFMBASE_USE_NAMESPACE

class UT_DirShareMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        scene = qobject_cast<DirShareMenuScene *>(DirShareMenuCreator().create());
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete scene;
    }

private:
    stub_ext::StubExt stub;
    DirShareMenuScene *scene { nullptr };
};

TEST_F(UT_DirShareMenuScene, Name)
{
    EXPECT_TRUE("DirShareMenu" == scene->name());
    EXPECT_FALSE(scene->name().isEmpty());
}

TEST_F(UT_DirShareMenuScene, Initialize)
{
    QVariantHash params;
    EXPECT_FALSE(scene->initialize(params));

    QList<QUrl> selectedUrls;
    QUrl nonFileUrl("smb://1.2.3.4");
    selectedUrls.append(nonFileUrl);
    params.insert(MenuParamKey::kSelectFiles, QVariant::fromValue<QList<QUrl>>(selectedUrls));
    EXPECT_FALSE(scene->initialize(params));

    params.clear();
    selectedUrls.clear();
    selectedUrls.append(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)));
    params.insert(MenuParamKey::kSelectFiles, QVariant::fromValue<QList<QUrl>>(selectedUrls));
    EXPECT_TRUE(scene->initialize(params));
    EXPECT_NO_FATAL_FAILURE(scene->initialize(params));
}

TEST_F(UT_DirShareMenuScene, Create)
{
    EXPECT_FALSE(scene->create(nullptr));

    QMenu *menu = new QMenu;
    EXPECT_FALSE(scene->create(menu));

    QList<QUrl> selectedUrls;
    selectedUrls.append(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)));
    QVariantHash params;
    params.insert(MenuParamKey::kSelectFiles, QVariant::fromValue<QList<QUrl>>(selectedUrls));

    bool shared = false;
    stub.set_lamda(&UserShareHelper::isShared, [&] { __DBG_STUB_INVOKE__ return shared; });
    stub.set_lamda(&UserShareHelper::canShare, [] { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&UserShareHelper::needDisableShareWidget, [] { __DBG_STUB_INVOKE__ return false; });

    EXPECT_NO_FATAL_FAILURE(scene->initialize(params));
    EXPECT_TRUE(scene->create(menu));
    EXPECT_FALSE(menu->actions().isEmpty());
    EXPECT_TRUE(menu->actions().first()->property(ActionPropertyKey::kActionID).toString() == "add-share");

    shared = true;
    delete menu;
    menu = new QMenu;
    EXPECT_TRUE(scene->create(menu));
    EXPECT_FALSE(menu->actions().isEmpty());
    EXPECT_TRUE(menu->actions().first()->property(ActionPropertyKey::kActionID).toString() == "remove-share");

    delete menu;
}

TEST_F(UT_DirShareMenuScene, UpdateState)
{
    EXPECT_NO_FATAL_FAILURE(scene->updateState(nullptr));

    QMenu *menu = new QMenu;
    EXPECT_NO_FATAL_FAILURE(scene->updateState(menu));
    delete menu;
}

TEST_F(UT_DirShareMenuScene, Triggered)
{
    EXPECT_FALSE(scene->triggered(nullptr));

    QAction *addAct = new QAction;
    addAct->setProperty(ActionPropertyKey::kActionID, "add-share");
    scene->d->predicateAction.insert("add-share", addAct);

    QAction *delAct = new QAction;
    delAct->setProperty(ActionPropertyKey::kActionID, "remove-share");
    scene->d->predicateAction.insert("remove-act", delAct);

    scene->d->selectFiles.append(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)));

    stub.set_lamda(&DirShareMenuScenePrivate::addShare, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&UserShareHelper::removeShareByPath, [] { __DBG_STUB_INVOKE__ });
    EXPECT_TRUE(scene->triggered(addAct));
    EXPECT_TRUE(scene->triggered(delAct));

    delete addAct;
    delete delAct;
}

TEST_F(UT_DirShareMenuScene, Scene)
{
    QAction *addAct = new QAction;
    addAct->setProperty(ActionPropertyKey::kActionID, "add-share");
    scene->d->predicateAction.insert("add-share", addAct);

    QAction *delAct = new QAction;
    delAct->setProperty(ActionPropertyKey::kActionID, "remove-share");

    EXPECT_FALSE(scene->scene(nullptr));
    EXPECT_TRUE(scene == scene->scene(addAct));
    EXPECT_TRUE(nullptr == scene->scene(delAct));
}

TEST_F(UT_DirShareMenuScene, DirShareMenuScenePrivate_addShare)
{
    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QList<QUrl>, QVariantHash &);
    auto push = static_cast<Push>(&dpf::EventChannelManager::push);
    stub.set_lamda(push, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    EXPECT_NO_FATAL_FAILURE(scene->d->addShare(QUrl()));
    EXPECT_NO_FATAL_FAILURE(scene->d->addShare(QUrl::fromLocalFile("/")));
}
