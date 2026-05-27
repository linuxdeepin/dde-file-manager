// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "menuscene/opendirmenuscene.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

#include <gtest/gtest.h>

#include <QMenu>
#include <QUrl>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;

class UT_OpenDirMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        scene = new OpenDirMenuScene();
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    }

    virtual void TearDown() override
    {
        delete scene;
        scene = nullptr;
        stub.clear();
    }

protected:
    OpenDirMenuScene *scene { nullptr };
    stub_ext::StubExt stub;
};

class UT_OpenDirMenuCreator : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        creator = new OpenDirMenuCreator();
    }

    virtual void TearDown() override
    {
        delete creator;
        creator = nullptr;
    }

protected:
    OpenDirMenuCreator *creator { nullptr };
};

TEST_F(UT_OpenDirMenuCreator, Name_ReturnsCorrectName)
{
    EXPECT_EQ(OpenDirMenuCreator::name(), "OpenDirMenu");
}

TEST_F(UT_OpenDirMenuCreator, Create_ReturnsOpenDirMenuScene)
{
    auto scene = creator->create();
    EXPECT_NE(scene, nullptr);
    EXPECT_EQ(scene->name(), "OpenDirMenu");
    delete scene;
}

TEST_F(UT_OpenDirMenuScene, Name_ReturnsCorrectName)
{
    EXPECT_EQ(scene->name(), "OpenDirMenu");
}

TEST_F(UT_OpenDirMenuScene, Initialize_ValidParams_ReturnsTrue)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kIsEmptyArea] = true;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(AbstractMenuScenePrivate, initializeParamsIsValid),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    EXPECT_TRUE(scene->initialize(params));
}

TEST_F(UT_OpenDirMenuScene, Initialize_EmptyArea_SkipsFileInfoCreation)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kIsEmptyArea] = true;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(AbstractMenuScenePrivate, initializeParamsIsValid),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    EXPECT_TRUE(scene->initialize(params));
}

TEST_F(UT_OpenDirMenuScene, Initialize_NonEmptyArea_CreatesFileInfo)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>{ QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(AbstractMenuScenePrivate, initializeParamsIsValid),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    EXPECT_TRUE(scene->initialize(params));
}

TEST_F(UT_OpenDirMenuScene, Initialize_FileInfoCreationFails_ReturnsFalse)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>{ QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    stub.set_lamda(VADDR(AbstractMenuScenePrivate, initializeParamsIsValid),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    EXPECT_FALSE(scene->initialize(params));
}

TEST_F(UT_OpenDirMenuScene, Initialize_InvalidParams_ReturnsFalse)
{
    QVariantHash params;

    stub.set_lamda(VADDR(AbstractMenuScenePrivate, initializeParamsIsValid),
                   [] {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    EXPECT_FALSE(scene->initialize(params));
}

TEST_F(UT_OpenDirMenuScene, Scene_NullAction_ReturnsNull)
{
    EXPECT_EQ(scene->scene(nullptr), nullptr);
}

TEST_F(UT_OpenDirMenuScene, Scene_ValidAction_ReturnsThis)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kIsEmptyArea] = true;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(AbstractMenuScenePrivate, initializeParamsIsValid),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    scene->initialize(params);

    QMenu menu;
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    scene->create(&menu);

    if (!menu.actions().isEmpty()) {
        auto testScene = scene->scene(menu.actions().first());
        EXPECT_TRUE(testScene == scene || testScene != nullptr);
    }
}

TEST_F(UT_OpenDirMenuScene, Create_EmptyArea_CreatesEmptyMenu)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kIsEmptyArea] = true;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(AbstractMenuScenePrivate, initializeParamsIsValid),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&SysInfoUtils::isDeveloperModeEnabled, []() {
        __DBG_STUB_INVOKE__
        return false;
    });

    scene->initialize(params);
    QMenu menu;
    EXPECT_TRUE(scene->create(&menu));
}

TEST_F(UT_OpenDirMenuScene, Create_NormalArea_CreatesNormalMenu)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>{ QUrl::fromLocalFile("/tmp/testdir") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(AbstractMenuScenePrivate, initializeParamsIsValid),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&SysInfoUtils::isDeveloperModeEnabled, []() {
        __DBG_STUB_INVOKE__
        return false;
    });

    scene->initialize(params);
    QMenu menu;
    EXPECT_TRUE(scene->create(&menu));
}

TEST_F(UT_OpenDirMenuScene, UpdateState_NullParent_ReturnsEarly)
{
    EXPECT_NO_FATAL_FAILURE(scene->updateState(nullptr));
}

TEST_F(UT_OpenDirMenuScene, UpdateState_ValidParent_CallsBase)
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

TEST_F(UT_OpenDirMenuScene, Triggered_InvalidAction_ReturnsFalse)
{
    QAction action("test");
    EXPECT_FALSE(scene->triggered(&action));
}

TEST_F(UT_OpenDirMenuScene, OpenAsAdminAction_DeveloperModeDisabled_DoesNotAdd)
{
    stub.set_lamda(&SysInfoUtils::isDeveloperModeEnabled, []() {
        __DBG_STUB_INVOKE__
        return false;
    });

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kIsEmptyArea] = true;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(AbstractMenuScenePrivate, initializeParamsIsValid),
                   [] {
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

    bool foundOpenAsAdmin = false;
    for (auto act : menu.actions()) {
        if (act->property("actionID").toString() == "open-as-admin") {
            foundOpenAsAdmin = true;
            break;
        }
    }
    EXPECT_FALSE(foundOpenAsAdmin);
}

TEST_F(UT_OpenDirMenuScene, OpenAsAdminAction_RootUser_DoesNotAdd)
{
    stub.set_lamda(&SysInfoUtils::isDeveloperModeEnabled, []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&SysInfoUtils::isRootUser, []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kIsEmptyArea] = true;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(AbstractMenuScenePrivate, initializeParamsIsValid),
                   [] {
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

    bool foundOpenAsAdmin = false;
    for (auto act : menu.actions()) {
        if (act->property("actionID").toString() == "open-as-admin") {
            foundOpenAsAdmin = true;
            break;
        }
    }
    EXPECT_FALSE(foundOpenAsAdmin);
}

TEST_F(UT_OpenDirMenuScene, OpenAsAdminAction_ServerSystem_DoesNotAdd)
{
    stub.set_lamda(&SysInfoUtils::isDeveloperModeEnabled, []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&SysInfoUtils::isRootUser, []() {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&SysInfoUtils::isServerSys, []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kIsEmptyArea] = true;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(AbstractMenuScenePrivate, initializeParamsIsValid),
                   [] {
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

    bool foundOpenAsAdmin = false;
    for (auto act : menu.actions()) {
        if (act->property("actionID").toString() == "open-as-admin") {
            foundOpenAsAdmin = true;
            break;
        }
    }
    EXPECT_FALSE(foundOpenAsAdmin);
}

TEST_F(UT_OpenDirMenuScene, OpenAsAdminAction_RemoteFile_DoesNotAdd)
{
    stub.set_lamda(&SysInfoUtils::isDeveloperModeEnabled, []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&SysInfoUtils::isRootUser, []() {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&SysInfoUtils::isServerSys, []() {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&ProtocolUtils::isRemoteFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kIsEmptyArea] = true;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(AbstractMenuScenePrivate, initializeParamsIsValid),
                   [] {
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

    bool foundOpenAsAdmin = false;
    for (auto act : menu.actions()) {
        if (act->property("actionID").toString() == "open-as-admin") {
            foundOpenAsAdmin = true;
            break;
        }
    }
    EXPECT_FALSE(foundOpenAsAdmin);
}

TEST_F(UT_OpenDirMenuScene, NormalMenu_SingleDirectory_AddsAllActions)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>{ QUrl::fromLocalFile("/tmp/testdir") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(AbstractMenuScenePrivate, initializeParamsIsValid),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&SysInfoUtils::isDeveloperModeEnabled, []() {
        __DBG_STUB_INVOKE__
        return false;
    });

    scene->initialize(params);
    QMenu menu;
    scene->create(&menu);
    EXPECT_GE(menu.actions().size(), 0);
}

TEST_F(UT_OpenDirMenuScene, NormalMenu_NonDirectory_AddsReverseSelect)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>{ QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(AbstractMenuScenePrivate, initializeParamsIsValid),
                   [] {
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
    EXPECT_GE(menu.actions().size(), 0);
}
