// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "menus/recentmenuscene.h"
#include "private/recentmenuscene_p.h"
#include "utils/recentmanager.h"
#include <dfm-base/base/application/application.h>
#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-framework/event/event.h>
#include "plugins/common/dfmplugin-menu/menuscene/action_defines.h"
#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"
#include <gtest/gtest.h>

#include <QPaintEvent>
#include <QPainter>
#include <QMenu>
#include <QAction>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_recent;

class RecentMenuSceneTest : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        auto creator = new RecentMenuCreator();
        scene = static_cast<dfmplugin_recent::RecentMenuScene *>(creator->create());
        d = scene->d.data();
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete scene;
    }

protected:
    stub_ext::StubExt stub;
    RecentMenuScene *scene { nullptr };
    RecentMenuScenePrivate *d { nullptr };
};

TEST_F(RecentMenuSceneTest, name)
{
    EXPECT_TRUE(scene->name() == "RecentMenu");
}

TEST_F(RecentMenuSceneTest, initialize)
{
    QList<QUrl> urls { QUrl::fromLocalFile("/hello/world"), QUrl::fromLocalFile("/i/can/eat/glass/without/hurt") };
    EXPECT_FALSE(scene->initialize({ { "currentDir", true } }));
    EXPECT_FALSE(scene->initialize({ { "selectFiles", QVariant::fromValue<QList<QUrl>>(urls) } }));
    EXPECT_FALSE(scene->initialize({ { "onDesktop", true } }));
    EXPECT_TRUE(scene->initialize({ { "isEmptyArea", true } }));
    EXPECT_FALSE(scene->initialize({ { "indexFlags", true } }));
    d->focusFile = QUrl::fromLocalFile("/hello/world");
    stub.set_lamda(VADDR(AbstractMenuScenePrivate, initializeParamsIsValid), [] {
        return true;
    });
    EXPECT_TRUE(scene->initialize({ { "isEmptyArea", true } }));
    urls.takeFirst();
    EXPECT_NO_FATAL_FAILURE(scene->initialize({ { "isEmptyArea", false },
                                                { "selectFiles", QVariant::fromValue<QList<QUrl>>(urls) } }));
}

TEST_F(RecentMenuSceneTest, create)
{
    EXPECT_FALSE(scene->create(nullptr));

    QMenu menu;
    EXPECT_NO_FATAL_FAILURE(scene->create(&menu));
    d->isEmptyArea = true;
    EXPECT_NO_FATAL_FAILURE(scene->create(&menu));
}

TEST_F(RecentMenuSceneTest, triggered)
{
    DPF_USE_NAMESPACE
    auto act = new QAction("hello");
    act->setProperty(ActionPropertyKey::kActionID, "hello");
    d->predicateAction.insert("hello", act);
    d->predicateAction.insert("remove", act);
    d->predicateAction.insert("open-file-location", act);
    d->predicateAction.insert("sort-by-path", act);
    d->predicateAction.insert("sort-by-lastRead", act);
    EXPECT_FALSE(scene->triggered(act));

    stub.set_lamda(&RecentHelper::removeRecent, []() {});

    void (*func)(const QList<QUrl> &) = &RecentHelper::openFileLocation;
    stub.set_lamda(func, []() {});

    typedef QVariant (EventChannelManager::*Push1)(const QString &, const QString &, const quint64 &, Global::ItemRoles &);
    auto push1 = static_cast<Push1>(&EventChannelManager::push);
    stub.set_lamda(push1, [] { __DBG_STUB_INVOKE__ return QVariant(); });

    act->setProperty(ActionPropertyKey::kActionID, "remove");
    EXPECT_TRUE(scene->triggered(act));
    act->setProperty(ActionPropertyKey::kActionID, "open-file-location");
    EXPECT_TRUE(scene->triggered(act));
    act->setProperty(ActionPropertyKey::kActionID, "sort-by-path");
    EXPECT_TRUE(scene->triggered(act));
    act->setProperty(ActionPropertyKey::kActionID, "sort-by-lastRead");
    EXPECT_TRUE(scene->triggered(act));
}

TEST_F(RecentMenuSceneTest, scene)
{
    EXPECT_EQ(nullptr, scene->scene(nullptr));

    auto act = new QAction("hello");
    d->predicateAction.insert("hello", act);
    EXPECT_STREQ(scene->scene(act)->metaObject()->className(), "dfmplugin_recent::RecentMenuScene");

    d->predicateAction.clear();
    EXPECT_NO_FATAL_FAILURE(scene->scene(act));
    delete act;
    act = nullptr;
}

TEST_F(RecentMenuSceneTest, updateMenu)
{
    QMenu menu;
    menu.addSeparator();
    auto act = menu.addAction("1");
    auto act1 = menu.addAction("2");
    auto act2 = menu.addAction("3");
    auto act3 = menu.addAction("4");
    act->setProperty(ActionPropertyKey::kActionID, "remove");
    act1->setProperty(ActionPropertyKey::kActionID, "open-file-location");
    act2->setProperty(ActionPropertyKey::kActionID, "sort-by-path");
    act3->setProperty(ActionPropertyKey::kActionID, "sort-by-lastRead");
    d->predicateAction.insert("hello", act);
    d->predicateAction.insert("remove", act);
    d->predicateAction.insert("open-file-location", act);
    d->predicateAction.insert("sort-by-path", act);
    d->predicateAction.insert("sort-by-lastRead", act);
    d->isEmptyArea = true;
    EXPECT_NO_FATAL_FAILURE(d->updateMenu(&menu));
    d->isEmptyArea = false;
    EXPECT_NO_FATAL_FAILURE(d->updateMenu(&menu));
}

TEST_F(RecentMenuSceneTest, updateSubMenu)
{
    DPF_USE_NAMESPACE
    int flag = 0;
    stub.set_lamda(&QAction::setChecked, [&flag] {
        __DBG_STUB_INVOKE__
        flag++;
    });
    QMenu menu;
    auto act = menu.addAction("1");
    auto act1 = menu.addAction("2");
    act->setProperty(ActionPropertyKey::kActionID, "remove");
    act1->setProperty(ActionPropertyKey::kActionID, "sort-by-time-modified");
    d->predicateAction[RecentActionID::kSortByPath] = act;
    d->predicateAction[RecentActionID::kSortByLastRead] = act1;
    typedef QVariant (EventChannelManager::*Push1)(const QString &, const QString &, quint64);
    auto push1 = static_cast<Push1>(&EventChannelManager::push);
    stub.set_lamda(push1, [&flag] {
        __DBG_STUB_INVOKE__
        flag++;
        return QVariant();
    });
    d->updateSortSubMenu(&menu);

    QMenu menu1;
    auto act2 = menu1.addAction("1");
    act2->setProperty(ActionPropertyKey::kActionID, "sort-by-time-modified");
    stub.set_lamda(push1, [&flag] {
        __DBG_STUB_INVOKE__
        flag++;
        return QVariant(Global::ItemRoles::kItemFilePathRole);
    });
    d->updateSortSubMenu(&menu);

    QMenu menu2;
    auto act3 = menu2.addAction("1");
    act3->setProperty(ActionPropertyKey::kActionID, "sort-by-time-modified");
    stub.set_lamda(push1, [&flag] {
        __DBG_STUB_INVOKE__
        flag++;
        return QVariant(Global::ItemRoles::kItemFileLastReadRole);
    });
    d->updateSortSubMenu(&menu2);
    EXPECT_TRUE(flag == 3);
}

TEST_F(RecentMenuSceneTest, updateState)
{
    QMenu menu;
    EXPECT_NO_FATAL_FAILURE(scene->updateState(&menu));
}
