// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// PMS sev-2 regression tests for dfmplugin-search (menu scene selection/bounds).

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/searchmenuscene.h"
#include "menus/searchmenuscene_p.h"
#include "utils/searchhelper.h"
#include "utils/custommanager.h"
#include "dfmplugin_search_global.h"

#include "plugins/common/dfmplugin-menu/menuscene/action_defines.h"
#include "dfm-base/dfm_menu_defines.h"

#include <dfm-framework/event/event.h>

#include <QAction>
#include <QMenu>
#include <QUrl>
#include <QVariantHash>

DPF_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class SearchMenuPmsRegressionTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    QAction *makeAction(QMenu *menu, const QString &id)
    {
        QAction *act = menu->addAction(id);
        act->setProperty(ActionPropertyKey::kActionID, id);
        return act;
    }

    stub_ext::StubExt stub;
};

// PMS:250985 search menu on computer-like custom views fell through to the raw
// target scheme; when the scheme registered kUseNormalMenu the workspace file
// scene must be requested instead (pushed scheme becomes "file").
TEST_F(SearchMenuPmsRegressionTest, BUG250985_Initialize_UseNormalMenuScheme_PushesFileScheme)
{
    QVariantMap props;
    props[CustomKey::kUseNormalMenu] = true;
    ASSERT_TRUE(CustomManager::instance()->registerCustomInfo("utnormalscheme", props));

    QString pushedScheme;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, QString);
    auto push = static_cast<Push>(&EventChannelManager::push);
    stub.set_lamda(push, [&](EventChannelManager *&, const QString &name, const QString &topic, QString scheme) -> QVariant {
        if (name == "dfmplugin_workspace" && topic == "slot_FindMenuScene")
            pushedScheme = scheme;
        return QVariant("utNoScene");
    });
    typedef QVariant (EventChannelManager::*PushCStr)(const QString &, const QString &, const char *);
    auto pushCStr = static_cast<PushCStr>(&EventChannelManager::push);
    stub.set_lamda(pushCStr, [&](EventChannelManager *&, const QString &name, const QString &topic, const char *scheme) -> QVariant {
        if (name == "dfmplugin_workspace" && topic == "slot_FindMenuScene")
            pushedScheme = QString::fromUtf8(scheme);
        return QVariant("utNoScene");
    });
    stub.set_lamda(&SearchMenuScenePrivate::disableSubScene, [] {});

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = SearchHelper::fromSearchFile(QUrl("utnormalscheme:///data"), "kw", "1");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>() << QUrl::fromLocalFile("/data/x.txt"));
    params[MenuParamKey::kIsEmptyArea] = false;
    params[MenuParamKey::kWindowId] = 1;

    SearchMenuScene scene;
    EXPECT_TRUE(scene.initialize(params));
    EXPECT_EQ(pushedScheme, QString("file"));   // pre-fix pushed "utnormalscheme"
}

// PMS:250985 control case: schemes without kUseNormalMenu keep asking for their
// own scheme scene.
TEST_F(SearchMenuPmsRegressionTest, BUG250985_Initialize_PlainScheme_PushesOwnScheme)
{
    QString pushedScheme;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, QString);
    auto push = static_cast<Push>(&EventChannelManager::push);
    stub.set_lamda(push, [&](EventChannelManager *&, const QString &name, const QString &topic, QString scheme) -> QVariant {
        if (name == "dfmplugin_workspace" && topic == "slot_FindMenuScene")
            pushedScheme = scheme;
        return QVariant("utNoScene");
    });
    stub.set_lamda(&SearchMenuScenePrivate::disableSubScene, [] {});

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = SearchHelper::fromSearchFile(QUrl("utplainpms:///data"), "kw", "1");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>() << QUrl::fromLocalFile("/data/x.txt"));
    params[MenuParamKey::kIsEmptyArea] = false;
    params[MenuParamKey::kWindowId] = 1;

    SearchMenuScene scene;
    EXPECT_TRUE(scene.initialize(params));
    EXPECT_EQ(pushedScheme, QString("utplainpms"));
}

// PMS:284723 updateMenu re-inserted the OpenFileLocation action at index 1 even
// when the remaining action list was empty (single-action menu), invoking
// QList::insert out of bounds. The fix appends when the list is empty.
TEST_F(SearchMenuPmsRegressionTest, BUG284723_UpdateMenu_OpenLocalOnly_AppendsNotInserts)
{
    SearchMenuScene scene;
    scene.d->isEmptyArea = false;
    scene.d->selectFiles << QUrl::fromLocalFile("/home/ut-target.txt");

    QMenu menu;
    QAction *openLocal = makeAction(&menu, SearchActionId::kOpenFileLocation);

    // Pre-fix this hit QList::insert(1) on an empty list (assert/UB crash).
    EXPECT_NO_FATAL_FAILURE(scene.d->updateMenu(&menu));

    // The action survived and stays usable.
    EXPECT_TRUE(menu.actions().contains(openLocal));
    EXPECT_TRUE(openLocal->isVisible());
}

// PMS:284723 companion: with more than one action left the position-1 insert
// keeps OpenFileLocation second.
TEST_F(SearchMenuPmsRegressionTest, BUG284723_UpdateMenu_MultipleActions_OpenLocalSecond)
{
    SearchMenuScene scene;
    scene.d->isEmptyArea = false;
    scene.d->selectFiles << QUrl::fromLocalFile("/home/ut-target.txt");

    QMenu menu;
    QAction *a1 = makeAction(&menu, "utActionA");
    QAction *openLocal = makeAction(&menu, SearchActionId::kOpenFileLocation);
    QAction *a2 = makeAction(&menu, "utActionB");

    EXPECT_NO_FATAL_FAILURE(scene.d->updateMenu(&menu));

    const QList<QAction *> acts = menu.actions();
    const int idx = acts.indexOf(openLocal);
    EXPECT_EQ(idx, 1);
    EXPECT_TRUE(acts.contains(a1));
    EXPECT_TRUE(acts.contains(a2));
    EXPECT_TRUE(openLocal->isVisible());
}
