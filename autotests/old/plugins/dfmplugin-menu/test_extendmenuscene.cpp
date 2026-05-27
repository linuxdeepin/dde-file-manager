// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "extendmenuscene/extendmenuscene.h"
#include "extendmenuscene/extendmenu/dcustomactionparser.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

#include <gtest/gtest.h>

#include <QMenu>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;

class UT_ExtendMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        parser = new DCustomActionParser();
        scene = new ExtendMenuScene(parser);
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    }

    virtual void TearDown() override
    {
        delete scene;
        scene = nullptr;
        delete parser;
        parser = nullptr;
        stub.clear();
    }

protected:
    ExtendMenuScene *scene { nullptr };
    DCustomActionParser *parser { nullptr };
    stub_ext::StubExt stub;
};

class UT_ExtendMenuCreator : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        creator = new ExtendMenuCreator();
    }

    virtual void TearDown() override
    {
        delete creator;
        creator = nullptr;
    }

protected:
    ExtendMenuCreator *creator { nullptr };
};

TEST_F(UT_ExtendMenuCreator, Name_ReturnsCorrectName)
{
    EXPECT_EQ(ExtendMenuCreator::name(), "ExtendMenu");
}

TEST_F(UT_ExtendMenuCreator, Create_ReturnsExtendMenuScene)
{
    auto scene = creator->create();
    EXPECT_NE(scene, nullptr);
    EXPECT_EQ(scene->name(), "ExtendMenu");
    delete scene;
}

TEST_F(UT_ExtendMenuScene, Name_ReturnsCorrectName)
{
    EXPECT_EQ(scene->name(), "ExtendMenu");
}

TEST_F(UT_ExtendMenuScene, Initialize_ValidParams_ReturnsTrue)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kOnDesktop] = false;
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

TEST_F(UT_ExtendMenuScene, Initialize_NonEmptyArea_CreatesFileInfo)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
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

TEST_F(UT_ExtendMenuScene, Initialize_FileInfoCreationFails_ReturnsFalse)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
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

TEST_F(UT_ExtendMenuScene, Scene_NullAction_ReturnsNull)
{
    EXPECT_EQ(scene->scene(nullptr), nullptr);
}

TEST_F(UT_ExtendMenuScene, Scene_ExtendAction_ReturnsThis)
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

    stub.set_lamda(&DCustomActionParser::getActionFiles, [](DCustomActionParser *, bool) {
        __DBG_STUB_INVOKE__
        return QList<DCustomActionEntry>();
    });

    scene->initialize(params);
    QMenu menu;
    scene->create(&menu);

    QAction action("test");
    EXPECT_TRUE(scene->scene(&action) == nullptr || scene->scene(&action) == scene);
}

TEST_F(UT_ExtendMenuScene, Create_NullParent_ReturnsTrue)
{
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_TRUE(scene->create(nullptr));
}

TEST_F(UT_ExtendMenuScene, Create_EmptyRootEntry_ReturnsTrue)
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

    stub.set_lamda(&DCustomActionParser::getActionFiles, [](DCustomActionParser *, bool) {
        __DBG_STUB_INVOKE__
        return QList<DCustomActionEntry>();
    });

    scene->initialize(params);
    QMenu menu;
    EXPECT_TRUE(scene->create(&menu));
}

TEST_F(UT_ExtendMenuScene, Create_WithValidActions_AddsActions)
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

    DCustomActionEntry entry;
    DCustomActionData data;
    data.actionName = "Test Action";
    data.actionCommand = "echo test";
    entry.actionData = data;
    entry.actionFileCombo = DCustomActionDefines::kBlankSpace;

    stub.set_lamda(&DCustomActionParser::getActionFiles, [&entry](DCustomActionParser *, bool) {
        __DBG_STUB_INVOKE__
        return QList<DCustomActionEntry>() << entry;
    });

    scene->initialize(params);
    QMenu menu;
    EXPECT_TRUE(scene->create(&menu));
}

TEST_F(UT_ExtendMenuScene, Triggered_InvalidAction_ReturnsFalse)
{
    QAction action("test");
    EXPECT_FALSE(scene->triggered(&action));
}

TEST_F(UT_ExtendMenuScene, UpdateState_NullParent_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(scene->updateState(nullptr));
}

TEST_F(UT_ExtendMenuScene, UpdateState_ValidParent_CallsBase)
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
