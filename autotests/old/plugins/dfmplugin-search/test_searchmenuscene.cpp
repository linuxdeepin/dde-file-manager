// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "menus/searchmenuscene.h"
#include "menus/searchmenuscene_p.h"
#include "utils/searchhelper.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"
#include "plugins/common/dfmplugin-menu/menuscene/action_defines.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/private/syncfileinfo_p.h>
#include <dfm-base/utils/sysinfoutils.h>

#include "stubext.h"

#include <dfm-framework/event/event.h>
#include <gtest/gtest.h>

#include <DDesktopServices>
#include <DGuiApplicationHelper>
#include <dtkwidget_global.h>

#include <QAction>
#include <QMenu>
#include <QProcess>

DPF_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

class TestScene : public AbstractMenuScene
{
public:
    explicit TestScene(QObject *parent = nullptr)
        : AbstractMenuScene(parent) { }
    ~TestScene() override { }

    QString name() const override
    {
        return sceneName;
    }

    QString sceneName = "TestScene";
};

TEST(SearchMenuCreatorTest, ut_name)
{
    EXPECT_EQ(SearchMenuCreator::name(), "SearchMenu");
}

TEST(SearchMenuCreatorTest, ut_create)
{
    SearchMenuCreator creator;
    auto scene = creator.create();

    EXPECT_TRUE(scene);
    delete scene;
}

TEST(SearchMenuSceneTest, ut_name)
{
    SearchMenuScene scene;
    EXPECT_EQ(scene.name(), SearchMenuCreator::name());
}

TEST(SearchMenuSceneTest, ut_initialize)
{
    stub_ext::StubExt st;
    st.set_lamda(VADDR(AbstractMenuScene, initialize), [] { return true; });
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, QString);
    auto push = static_cast<Push>(&EventChannelManager::push);
    st.set_lamda(push, [](EventChannelManager *&, const QString &name, const QString &, QString) {
        if (name == "dfmplugin_workspace") {
            return QVariant("TestScene");
        } else {
            return QVariant::fromValue(new TestScene);
        }
    });
    st.set_lamda(&SearchMenuScenePrivate::disableSubScene, [] {});

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl();
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>() << QUrl::fromLocalFile("/home"));
    params[MenuParamKey::kOnDesktop] = false;
    params[MenuParamKey::kIsEmptyArea] = false;
    params[MenuParamKey::kWindowId] = 123;

    SearchMenuScene scene;
    EXPECT_FALSE(scene.initialize(params));

    auto searchUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "test", "123");
    params[MenuParamKey::kCurrentDir] = searchUrl;
    EXPECT_TRUE(scene.initialize(params));

    searchUrl = SearchHelper::fromSearchFile(QUrl::fromUserInput("test:///home"), "test", "123");
    params[MenuParamKey::kCurrentDir] = searchUrl;
    EXPECT_TRUE(scene.initialize(params));

    params[MenuParamKey::kIsEmptyArea] = true;
    EXPECT_TRUE(scene.initialize(params));
}

TEST(SearchMenuSceneTest, ut_scene)
{
    SearchMenuScene scene;
    EXPECT_FALSE(scene.scene(nullptr));

    QAction action;
    scene.d->predicateAction.insert("test", &action);
    EXPECT_EQ(&scene, scene.scene(&action));

    stub_ext::StubExt st;
    st.set_lamda(VADDR(AbstractMenuScene, scene), [] { return nullptr; });

    scene.d->predicateAction.clear();
    EXPECT_FALSE(scene.scene(&action));
}

TEST(SearchMenuSceneTest, ut_create)
{
    SearchMenuScene scene;
    EXPECT_FALSE(scene.create(nullptr));

    stub_ext::StubExt st;
    st.set_lamda(VADDR(AbstractMenuScene, create), [] { return true; });

    scene.d->isEmptyArea = true;
    QMenu menu;
    EXPECT_TRUE(scene.create(&menu));

    scene.d->isEmptyArea = false;
    EXPECT_TRUE(scene.create(&menu));
}

TEST(SearchMenuSceneTest, ut_updateState)
{
    stub_ext::StubExt st;
    st.set_lamda(VADDR(AbstractMenuScene, updateState), [] {});
    st.set_lamda(&SearchMenuScenePrivate::updateMenu, [] {});

    SearchMenuScene scene;
    EXPECT_NO_FATAL_FAILURE(scene.updateState(nullptr));
}

TEST(SearchMenuSceneTest, ut_triggered)
{
    stub_ext::StubExt st;
    st.set_lamda(&SyncFileInfoPrivate::init, [] {});
    QSharedPointer<SyncFileInfo> info(new SyncFileInfo(QUrl::fromLocalFile("/home")));
    st.set_lamda(&InfoFactory::create<FileInfo>, [&info] { return info; });
    st.set_lamda(&SearchMenuScenePrivate::openFileLocation, [] { return true; });

    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, quint64);
    auto push = static_cast<Push>(&EventChannelManager::push);
    st.set_lamda(push, [] { return QVariant(); });

    QAction action1, action2, action3;
    action1.setProperty(ActionPropertyKey::kActionID, SearchActionId::kOpenFileLocation);
    action2.setProperty(ActionPropertyKey::kActionID, dfmplugin_menu::ActionID::kSelectAll);
    action3.setProperty("test", "test");

    SearchMenuScene scene;
    scene.d->predicateAction.insert(SearchActionId::kOpenFileLocation, &action1);
    scene.d->selectFiles << QUrl::fromLocalFile("/home");
    EXPECT_TRUE(scene.triggered(&action1));

    scene.d->predicateAction.insert(dfmplugin_menu::ActionID::kSelectAll, &action2);
    EXPECT_TRUE(scene.triggered(&action2));

    st.set_lamda(VADDR(AbstractMenuScene, triggered), [] { return true; });
    EXPECT_TRUE(scene.triggered(&action3));
}

// SearchMenuScenePrivate
TEST(SearchMenuScenePrivateTest, ut_updateMenu_1)
{
    QMenu menu;
    menu.addSeparator();
    auto act = menu.addAction("Select all");
    act->setProperty(ActionPropertyKey::kActionID, dfmplugin_menu::ActionID::kSelectAll);

    SearchMenuScene scene;
    scene.d->isEmptyArea = true;
    EXPECT_NO_FATAL_FAILURE(scene.d->updateMenu(&menu));
}

TEST(SearchMenuScenePrivateTest, ut_updateMenu_2)
{
    QMenu menu;
    menu.addSeparator();
    auto act = menu.addAction("Open file location");
    act->setProperty(ActionPropertyKey::kActionID, SearchActionId::kOpenFileLocation);

    SearchMenuScene scene;
    scene.d->isEmptyArea = false;
    EXPECT_NO_FATAL_FAILURE(scene.d->updateMenu(&menu));
}

TEST(SearchMenuScenePrivateTest, ut_openFileLocation)
{
    stub_ext::StubExt st;
    st.set_lamda(SysInfoUtils::isRootUser, [] { return true; });

    typedef bool (*StartFunc)(const QString &, const QStringList &, const QString &, qint64 *pid);
    auto func = static_cast<StartFunc>(QProcess::startDetached);
    st.set_lamda(func, [] { return true; });

    SearchMenuScene scene;
    EXPECT_TRUE(scene.d->openFileLocation("/home"));

    st.reset(SysInfoUtils::isRootUser);
    st.set_lamda(SysInfoUtils::isRootUser, [] { return false; });
    auto func2 = qOverload<const QString &, const QString &>(&DDesktopServices::showFileItem);
    st.set_lamda(func2, [] { return true; });
    EXPECT_TRUE(scene.d->openFileLocation("/home"));
}

TEST(SearchMenuScenePrivateTest, ut_disableSubScene)
{
    QList<AbstractMenuScene *> sceneList;
    auto s1 = new TestScene();
    auto s2 = new TestScene();
    s2->sceneName = "TestScene2";
    sceneList << s1 << s2;

    SearchMenuScene scene;
    scene.setSubscene(sceneList);
    EXPECT_NO_FATAL_FAILURE(scene.d->disableSubScene(&scene, "TestScene2"));
}
