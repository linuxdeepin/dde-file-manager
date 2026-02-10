// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "menu/mysharemenuscene.h"
#include "private/mysharemenuscene_p.h"
#include "events/shareeventscaller.h"
#include "dfmplugin_myshares_global.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/abstractmenuscene.h>
#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <QMenu>
#include <QAction>
#include <QVariantHash>
#include <QList>
#include <QUrl>
#include <QScopedPointer>

using namespace dfmplugin_myshares;
DFMBASE_USE_NAMESPACE

        class UT_MyShareMenuScene : public testing::Test
{
public:
    virtual void SetUp() override
    {
        scene = new MyShareMenuScene();
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete scene;
        scene = nullptr;
    }

    stub_ext::StubExt stub;
    MyShareMenuScene *scene { nullptr };
};

TEST_F(UT_MyShareMenuScene, Name)
{
    EXPECT_EQ("MyShareMenu", scene->name());
}

TEST_F(UT_MyShareMenuScene, Initialize)
{
    EXPECT_TRUE(scene->initialize({}));
    EXPECT_NO_FATAL_FAILURE(scene->initialize({}));
}

TEST_F(UT_MyShareMenuScene, Create)
{
    QMenu *menu = new QMenu;
    EXPECT_TRUE(scene->create(menu));
    EXPECT_NO_FATAL_FAILURE(scene->create(menu));
    delete menu;
}

TEST_F(UT_MyShareMenuScene, UpdateState)
{
    QMenu *menu = new QMenu;
    EXPECT_NO_FATAL_FAILURE(scene->updateState(menu));
    delete menu;
}

TEST_F(UT_MyShareMenuScene, Triggered)
{
    EXPECT_NO_FATAL_FAILURE(scene->triggered(nullptr));
    auto act = new QAction("Hello");
    act->setProperty("actionID", "Hello");
    EXPECT_FALSE(scene->triggered(act));
    EXPECT_NO_FATAL_FAILURE(scene->triggered(act));

    stub.set_lamda(&MyShareMenuScenePrivate::triggered, [] { __DBG_STUB_INVOKE__ return false; });
    scene->d->predicateAction.insert("Hello", act);
    EXPECT_FALSE(scene->triggered(act));
    delete act;
    scene->d->predicateAction.clear();
}

TEST_F(UT_MyShareMenuScene, Scene)
{
    EXPECT_TRUE(scene->scene(nullptr) == nullptr);
    EXPECT_NO_FATAL_FAILURE(scene->scene(nullptr));

    QAction *act = new QAction("Hello");
    scene->d->predicateAction.insert("Hello", act);
    EXPECT_TRUE(scene->scene(act));
    EXPECT_NO_FATAL_FAILURE(scene->scene(act));
    EXPECT_STREQ(scene->scene(act)->metaObject()->className(), "dfmplugin_myshares::MyShareMenuScene");

    delete act;
    act = new QAction("World");
    EXPECT_NO_FATAL_FAILURE(scene->scene(act));
    EXPECT_TRUE(scene->scene(nullptr) == nullptr);
    delete act;
}

class UT_MyShareMenuCreator : public testing::Test
{
public:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};

TEST_F(UT_MyShareMenuCreator, Name)
{
    EXPECT_EQ("MyShareMenu", MyShareMenuCreator::name());
}

TEST_F(UT_MyShareMenuCreator, Create)
{
    MyShareMenuCreator creator;
    auto scene = creator.create();
    EXPECT_TRUE(scene);
    EXPECT_STREQ(scene->metaObject()->className(), "dfmplugin_myshares::MyShareMenuScene");
    delete scene;
}

class UT_MyShareMenuScenePrivate : public testing::Test
{
public:
    virtual void SetUp() override
    {
        scene = new MyShareMenuScene();
        d = scene->d.data();
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete scene;
        scene = nullptr;
    }

    stub_ext::StubExt stub;
    MyShareMenuScene *scene { nullptr };
    MyShareMenuScenePrivate *d { nullptr };
};

TEST_F(UT_MyShareMenuScenePrivate, CreateFileMenu)
{
    d->isEmptyArea = true;
    EXPECT_NO_FATAL_FAILURE(d->createFileMenu(nullptr));

    d->isEmptyArea = false;
    EXPECT_NO_FATAL_FAILURE(d->createFileMenu(nullptr));

    auto menu = new QMenu;
    EXPECT_NO_FATAL_FAILURE(d->createFileMenu(menu));

    d->selectFiles.append(QUrl::fromLocalFile("/Hello/World"));
    EXPECT_NO_FATAL_FAILURE(d->createFileMenu(menu));

    delete menu;
}

TEST_F(UT_MyShareMenuScenePrivate, Triggered)
{
    d->predicateAction.clear();
    EXPECT_FALSE(d->triggered("Test"));

    stub.set_lamda(ShareEventsCaller::sendOpenDirs, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(ShareEventsCaller::sendCancelSharing, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(ShareEventsCaller::sendShowProperty, [] { __DBG_STUB_INVOKE__ });

    d->predicateAction.insert("test", nullptr);
    EXPECT_FALSE(d->triggered("test"));

    d->predicateAction.insert(MySharesActionId::kOpenShareFolder, nullptr);
    d->predicateAction.insert(MySharesActionId::kOpenShareInNewWin, nullptr);
    d->predicateAction.insert(MySharesActionId::kOpenShareInNewTab, nullptr);
    d->predicateAction.insert(MySharesActionId::kCancleSharing, nullptr);
    d->predicateAction.insert(MySharesActionId::kShareProperty, nullptr);

    EXPECT_TRUE(d->triggered(MySharesActionId::kOpenShareFolder));
    EXPECT_TRUE(d->triggered(MySharesActionId::kOpenShareInNewWin));
    EXPECT_TRUE(d->triggered(MySharesActionId::kOpenShareInNewTab));
    EXPECT_TRUE(d->triggered(MySharesActionId::kShareProperty));

    EXPECT_FALSE(d->triggered(MySharesActionId::kCancleSharing));
    d->selectFiles.append(QUrl::fromLocalFile("/hello/world"));
    EXPECT_TRUE(d->triggered(MySharesActionId::kCancleSharing));
}
