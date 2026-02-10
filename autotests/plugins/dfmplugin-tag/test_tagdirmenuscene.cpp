// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "menu/tagdirmenuscene.h"
#include "menu/private/tagdirmenuscene_p.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <gtest/gtest.h>

#include <QMenu>
#include <QPaintEvent>
#include <QPainter>
#include <QProcess>
#include <DDesktopServices>
#include <DGuiApplicationHelper>
#include <dtkwidget_global.h>

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
using namespace dfmplugin_tag;

class TagDirMenuSceneTest : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        auto creator = new TagDirMenuCreator();
        scene = static_cast<dfmplugin_tag::TagDirMenuScene *>(creator->create());
        d = scene->d.data();
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete scene;
    }

protected:
    stub_ext::StubExt stub;
    TagDirMenuScene *scene { nullptr };
    TagDirMenuScenePrivate *d { nullptr };
};

TEST_F(TagDirMenuSceneTest, name)
{
    EXPECT_TRUE(scene->name() == "TagDirMenu");
}

TEST_F(TagDirMenuSceneTest, initialize)
{
    QList<QUrl> urls { QUrl::fromLocalFile("/hello/world"),
                       QUrl::fromLocalFile("/i/can/eat/glass/without/hurt") };
    EXPECT_TRUE(scene->initialize({ { "currentDir", true } }));
    EXPECT_TRUE(scene->initialize({ { "selectFiles", QVariant::fromValue<QList<QUrl>>(urls) } }));
    EXPECT_TRUE(scene->initialize({ { "onDesktop", true } }));
    EXPECT_TRUE(scene->initialize({ { "isEmptyArea", true } }));
    EXPECT_TRUE(scene->initialize({ { "indexFlags", true } }));
}

TEST_F(TagDirMenuSceneTest, create)
{
    EXPECT_FALSE(scene->create(nullptr));

    QMenu menu;
    EXPECT_TRUE(scene->create(&menu));
}

TEST_F(TagDirMenuSceneTest, triggered)
{
    auto act = new QAction("hello");
    act->setProperty(TagActionId::kOpenFileLocation, "hello");
    EXPECT_TRUE(scene->initialize({ { "onDesktop", true }, { "onCollection", true } }));
    EXPECT_FALSE(scene->triggered(act));
}

TEST_F(TagDirMenuSceneTest, updateMenu)
{
    QMenu menu;
    menu.addSeparator();
    auto act = menu.addAction("Open file location");
    act->setProperty(ActionPropertyKey::kActionID, TagActionId::kOpenFileLocation);

    d->isEmptyArea = false;
    EXPECT_NO_FATAL_FAILURE(d->updateMenu(&menu));

    d->isEmptyArea = true;
    EXPECT_NO_FATAL_FAILURE(d->updateMenu(&menu));
}

TEST_F(TagDirMenuSceneTest, updateState)
{
    QMenu menu;
    bool isRun = false;
    stub.set_lamda(&TagDirMenuScenePrivate::updateMenu, [&isRun] { isRun = true; });
    scene->updateState(&menu);
    EXPECT_TRUE(isRun);
}

TEST_F(TagDirMenuSceneTest, scene)
{
    EXPECT_EQ(nullptr, scene->scene(nullptr));

    auto act = new QAction("hello");
    d->predicateAction.insert("hello", act);
    EXPECT_STREQ(scene->scene(act)->metaObject()->className(),
                 "dfmplugin_tag::TagDirMenuScene");

    d->predicateAction.clear();
    scene->scene(act);
    delete act;
    act = nullptr;
}

TEST_F(TagDirMenuSceneTest, openFileLocation)
{
    stub_ext::StubExt st;
    st.set_lamda(SysInfoUtils::isRootUser, [] { return true; });

    typedef bool (*StartFunc)(const QString &, const QStringList &, const QString &, qint64 *pid);
    auto func = static_cast<StartFunc>(QProcess::startDetached);
    st.set_lamda(func, [] { return true; });

    EXPECT_TRUE(d->openFileLocation("/home"));

    st.reset(SysInfoUtils::isRootUser);
    st.set_lamda(SysInfoUtils::isRootUser, [] { return false; });
    auto func2 = qOverload<const QString &, const QString &>(&DDesktopServices::showFileItem);
    st.set_lamda(func2, [] { return true; });
    EXPECT_TRUE(d->openFileLocation("/home"));
}
