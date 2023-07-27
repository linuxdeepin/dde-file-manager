// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "menu/private/bookmarkmenuscene_p.h"
#include "menu/bookmarkmenuscene.h"
#include "controller/bookmarkmanager.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/utils/clipboard.h>

#include <gtest/gtest.h>

#include <QMenu>
#include <QPaintEvent>
#include <QPainter>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_bookmark;

class BookMarkMenuSceneTest : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        auto creator = new BookmarkMenuCreator();
        scene = static_cast<dfmplugin_bookmark::BookmarkMenuScene *>(creator->create());
        d = scene->d.data();
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete scene;
    }

protected:
    stub_ext::StubExt stub;
    BookmarkMenuScene *scene { nullptr };
    BookmarkMenuScenePrivate *d { nullptr };
};

TEST_F(BookMarkMenuSceneTest, name)
{
    EXPECT_TRUE(scene->name() == "BookmarkMenu");
}

TEST_F(BookMarkMenuSceneTest, initialize)
{
    QList<QUrl> urls { QUrl::fromLocalFile("/hello/world"),
                       QUrl::fromLocalFile("/i/can/eat/glass/without/hurt") };
    EXPECT_TRUE(scene->initialize({ { "currentDir", true } }));
    EXPECT_TRUE(scene->initialize(
            { { "selectFiles", QVariant::fromValue<QList<QUrl>>(urls) } }));
    EXPECT_TRUE(scene->initialize({ { "onDesktop", false } }));
    EXPECT_EQ(d->onDesktop, false);
    EXPECT_TRUE(scene->initialize({ { "indexFlags", true } }));
}

TEST_F(BookMarkMenuSceneTest, create)
{
    EXPECT_FALSE(scene->create(nullptr));

    QMenu menu;
    EXPECT_TRUE(scene->create(&menu));
}

TEST_F(BookMarkMenuSceneTest, triggered)
{
    auto act = new QAction("hello");
    act->setProperty(ActionPropertyKey::kActionID, "hello");
    scene->d->focusFile.setUrl("/hello");
    stub.set_lamda(&BookMarkManager::addBookMark, []() { return true; });
    stub.set_lamda(&BookMarkManager::removeBookMark, []() { return true; });
    EXPECT_TRUE(scene->initialize({ { "onDesktop", true }, { "onCollection", true } }));
    EXPECT_FALSE(scene->triggered(act));
    EXPECT_TRUE(scene->initialize({ { "onDesktop", true } }));
    EXPECT_FALSE(scene->triggered(act));
}

TEST_F(BookMarkMenuSceneTest, scene)
{
    EXPECT_EQ(nullptr, scene->scene(nullptr));

    auto act = new QAction("hello");
    d->predicateAction.insert("hello", act);
    EXPECT_STREQ(scene->scene(act)->metaObject()->className(),
                 "dfmplugin_bookmark::BookmarkMenuScene");

    d->predicateAction.clear();
    scene->scene(act);
    delete act;
    act = nullptr;
}
