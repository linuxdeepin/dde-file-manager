// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "menuscene/openwithmenuscene.h"
#include "menuscene/action_defines.h"
#include "menuscene/private/openwithmenuscene_p.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/mimetype/mimesappsmanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>

#include <QMenu>
#include <QAction>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;

class OwmsDispatchReceiver : public QObject
{
public:
    explicit OwmsDispatchReceiver(QObject *parent = nullptr)
        : QObject(parent) { }

    QVariant onOpenFilesByApp(quint64 id, QList<QUrl> urls, QList<QString> apps)
    {
        lastWindowId = id;
        lastUrls = urls;
        lastApps = apps;
        return QVariant(true);
    }

    quint64 lastWindowId { 0 };
    QList<QUrl> lastUrls;
    QList<QString> lastApps;
};

class UT_OpenWithMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        scene = new OpenWithMenuScene();
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    }

    virtual void TearDown() override
    {
        delete scene;
        scene = nullptr;
        stub.clear();
    }

    void stubPerfectMenuParams()
    {
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

    void stubInitializeBase()
    {
        stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
            __DBG_STUB_INVOKE__
            return true;
        });
        stub.set_lamda(VADDR(AbstractMenuScenePrivate, initializeParamsIsValid),
                       [] {
                           __DBG_STUB_INVOKE__
                           return true;
                       });
    }

    void stubFileInfoCreate()
    {
        stub.set_lamda(&InfoFactory::create<FileInfo>,
                       [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                           __DBG_STUB_INVOKE__
                           return QSharedPointer<FileInfo>(new FileInfo(url));
                       });
    }

    void stubMimesAppsManager(const QStringList &recommendApps)
    {
        stub.set_lamda(&MimesAppsManager::initMimeTypeApps, [] {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&MimesAppsManager::getRecommendedApps, [recommendApps](const QUrl &) -> QStringList {
            __DBG_STUB_INVOKE__
            return recommendApps;
        });
    }

protected:
    OpenWithMenuScene *scene { nullptr };
    stub_ext::StubExt stub;
};

class UT_OpenWithMenuCreator : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        creator = new OpenWithMenuCreator();
    }

    virtual void TearDown() override
    {
        delete creator;
        creator = nullptr;
    }

protected:
    OpenWithMenuCreator *creator { nullptr };
};

static QVariantHash owmsValidParams()
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;
    return params;
}

TEST_F(UT_OpenWithMenuCreator, Name_ReturnsCorrectName)
{
    EXPECT_EQ(OpenWithMenuCreator::name(), "OpenWithMenu");
}

TEST_F(UT_OpenWithMenuCreator, Create_ReturnsOpenWithMenuScene)
{
    auto scene = creator->create();
    EXPECT_NE(scene, nullptr);
    EXPECT_EQ(scene->name(), "OpenWithMenu");
    delete scene;
}

TEST_F(UT_OpenWithMenuScene, Name_ReturnsCorrectName)
{
    EXPECT_EQ(scene->name(), "OpenWithMenu");
}

TEST_F(UT_OpenWithMenuScene, Initialize_EmptySelectFiles_ReturnsFalse)
{
    QVariantHash params;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>());

    EXPECT_FALSE(scene->initialize(params));
}

TEST_F(UT_OpenWithMenuScene, Initialize_ValidFiles_ReturnsTrue)
{
    QVariantHash params = owmsValidParams();

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stubMimesAppsManager(QStringList { "/usr/share/applications/fake-app.desktop" });

    EXPECT_TRUE(scene->initialize(params));
}

TEST_F(UT_OpenWithMenuScene, Scene_NullAction_ReturnsNull)
{
    EXPECT_EQ(scene->scene(nullptr), nullptr);
}

TEST_F(UT_OpenWithMenuScene, Create_EmptySelect_ReturnsFalse)
{
    QMenu menu;
    EXPECT_FALSE(scene->create(&menu));
}

TEST_F(UT_OpenWithMenuScene, Create_InvalidFocus_ReturnsFalse)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl() });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubMimesAppsManager(QStringList());

    // initialize fails because focus file is invalid, selectFiles is kept.
    EXPECT_FALSE(scene->initialize(params));

    QMenu menu;
    EXPECT_FALSE(scene->create(&menu));
}

TEST_F(UT_OpenWithMenuScene, Create_ValidFocus_AddsOpenWithActions)
{
    QVariantHash params = owmsValidParams();

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stubMimesAppsManager(QStringList { "/usr/share/applications/fake-app.desktop" });
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    ASSERT_TRUE(scene->initialize(params));

    QMenu menu;
    EXPECT_TRUE(scene->create(&menu));

    ASSERT_GE(menu.actions().size(), 1);
    QAction *openWithAction = menu.actions().first();
    EXPECT_EQ(openWithAction->property(ActionPropertyKey::kActionID).toString(), QString(ActionID::kOpenWith));

    QMenu *subMenu = openWithAction->menu();
    ASSERT_NE(subMenu, nullptr);
    // one recommended app + one custom action
    EXPECT_EQ(subMenu->actions().size(), 2);
    QAction *appAction = subMenu->actions().first();
    EXPECT_EQ(appAction->property(ActionPropertyKey::kActionID).toString(), QString(ActionID::kOpenWithApp));
    EXPECT_EQ(appAction->property("AppName").toString(), QString("/usr/share/applications/fake-app.desktop"));
    EXPECT_EQ(subMenu->actions().last()->property(ActionPropertyKey::kActionID).toString(), QString(ActionID::kOpenWithCustom));
}

TEST_F(UT_OpenWithMenuScene, Create_FocusOnDesktop_ReturnsEarly)
{
    QVariantHash params = owmsValidParams();
    params[MenuParamKey::kIsFocusOnDDEDesktopFile] = true;
    params[MenuParamKey::kIsSystemPathIncluded] = false;
    params[MenuParamKey::kIsDDEDesktopFileIncluded] = true;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stubMimesAppsManager(QStringList { "/usr/share/applications/fake-app.desktop" });
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    ASSERT_TRUE(scene->initialize(params));

    QMenu menu;
    EXPECT_TRUE(scene->create(&menu));
    // returns early, no open-with action is added
    EXPECT_TRUE(menu.actions().isEmpty());
}

TEST_F(UT_OpenWithMenuScene, Create_SystemPathIncluded_ReturnsEarly)
{
    QVariantHash params = owmsValidParams();
    params[MenuParamKey::kIsFocusOnDDEDesktopFile] = false;
    params[MenuParamKey::kIsSystemPathIncluded] = true;
    params[MenuParamKey::kIsDDEDesktopFileIncluded] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stubMimesAppsManager(QStringList());
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    ASSERT_TRUE(scene->initialize(params));

    QMenu menu;
    EXPECT_TRUE(scene->create(&menu));
    EXPECT_TRUE(menu.actions().isEmpty());
}

TEST_F(UT_OpenWithMenuScene, UpdateState_DelegatesToBase)
{
    bool baseCalled = false;
    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [&baseCalled](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        baseCalled = true;
    });

    QMenu menu;
    scene->updateState(&menu);
    EXPECT_TRUE(baseCalled);
}

TEST_F(UT_OpenWithMenuScene, UpdateState_NotExistsFile_DisablesOpenWith)
{
    QVariantHash params = owmsValidParams();

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stubMimesAppsManager(QStringList { "/usr/share/applications/fake-app.desktop" });
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    ASSERT_TRUE(scene->initialize(params));
    QMenu menu;
    ASSERT_TRUE(scene->create(&menu));

    stub.set_lamda(VADDR(FileInfo, exists), [](FileInfo *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    scene->updateState(&menu);

    QAction *openWithAction = menu.actions().isEmpty() ? nullptr : menu.actions().first();
    ASSERT_NE(openWithAction, nullptr);
    EXPECT_FALSE(openWithAction->isEnabled());
}

TEST_F(UT_OpenWithMenuScene, Triggered_OwnAppAction_ReturnsTrue)
{
    OwmsDispatchReceiver receiver;
    dpfSignalDispatcher->subscribe(GlobalEventType::kOpenFilesByApp, &receiver, &OwmsDispatchReceiver::onOpenFilesByApp);

    QVariantHash params = owmsValidParams();

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stubMimesAppsManager(QStringList { "/usr/share/applications/fake-app.desktop" });
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    ASSERT_TRUE(scene->initialize(params));
    QMenu menu;
    ASSERT_TRUE(scene->create(&menu));

    QAction *appAction { nullptr };
    for (const auto act : menu.actions()) {
        if (act->menu()) {
            for (const auto subAct : act->menu()->actions()) {
                if (subAct->property(ActionPropertyKey::kActionID).toString() == ActionID::kOpenWithApp)
                    appAction = subAct;
            }
        }
    }
    ASSERT_NE(appAction, nullptr);

    EXPECT_TRUE(scene->triggered(appAction));
    EXPECT_EQ(receiver.lastUrls.size(), 1);
    ASSERT_EQ(receiver.lastApps.size(), 1);
    EXPECT_EQ(receiver.lastApps.first(), QString("/usr/share/applications/fake-app.desktop"));

    dpfSignalDispatcher->unsubscribe(GlobalEventType::kOpenFilesByApp, &receiver, &OwmsDispatchReceiver::onOpenFilesByApp);
}

TEST_F(UT_OpenWithMenuScene, Triggered_CustomAction_ReturnsTrue)
{
    QVariantHash params = owmsValidParams();

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stubMimesAppsManager(QStringList { "/usr/share/applications/fake-app.desktop" });
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    ASSERT_TRUE(scene->initialize(params));
    QMenu menu;
    ASSERT_TRUE(scene->create(&menu));

    QAction *customAction { nullptr };
    for (const auto act : menu.actions()) {
        if (act->menu()) {
            for (const auto subAct : act->menu()->actions()) {
                if (subAct->property(ActionPropertyKey::kActionID).toString() == ActionID::kOpenWithCustom)
                    customAction = subAct;
            }
        }
    }
    ASSERT_NE(customAction, nullptr);

    // dpfSlotChannel has no receiver, still returns true
    EXPECT_TRUE(scene->triggered(customAction));
}

TEST_F(UT_OpenWithMenuScene, Triggered_NotOwnAction_DelegatesToBase)
{
    QAction action("test");
    EXPECT_FALSE(scene->triggered(&action));
}

TEST_F(UT_OpenWithMenuScene, Scene_OwnAction_ReturnsThis)
{
    QVariantHash params = owmsValidParams();

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stubMimesAppsManager(QStringList { "/usr/share/applications/fake-app.desktop" });
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    ASSERT_TRUE(scene->initialize(params));
    QMenu menu;
    ASSERT_TRUE(scene->create(&menu));

    QAction *openWithAction = menu.actions().isEmpty() ? nullptr : menu.actions().first();
    ASSERT_NE(openWithAction, nullptr);
    EXPECT_EQ(scene->scene(openWithAction), scene);
}
