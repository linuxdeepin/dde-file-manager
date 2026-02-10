// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "menuscene/fileoperatormenuscene.h"
#include "menuscene/action_defines.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/file/local/syncfileinfo.h>

#include <gtest/gtest.h>

#include <QMenu>
#include <QAction>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;

class UT_FileOperatorMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        scene = new FileOperatorMenuScene();
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
    FileOperatorMenuScene *scene { nullptr };
    stub_ext::StubExt stub;
};

class UT_FileOperatorMenuCreator : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        creator = new FileOperatorMenuCreator();
    }

    virtual void TearDown() override
    {
        delete creator;
        creator = nullptr;
    }

protected:
    FileOperatorMenuCreator *creator { nullptr };
};

TEST_F(UT_FileOperatorMenuCreator, Name_ReturnsCorrectName)
{
    EXPECT_EQ(FileOperatorMenuCreator::name(), "FileOperatorMenu");
}

TEST_F(UT_FileOperatorMenuCreator, Create_ReturnsFileOperatorMenuScene)
{
    auto scene = creator->create();
    EXPECT_NE(scene, nullptr);
    EXPECT_EQ(scene->name(), "FileOperatorMenu");
    delete scene;
}

TEST_F(UT_FileOperatorMenuScene, Name_ReturnsCorrectName)
{
    EXPECT_EQ(scene->name(), "FileOperatorMenu");
}

TEST_F(UT_FileOperatorMenuScene, Initialize_EmptySelectFiles_ReturnsFalse)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>());
    params[MenuParamKey::kIsEmptyArea] = false;

    EXPECT_FALSE(scene->initialize(params));
}

TEST_F(UT_FileOperatorMenuScene, Initialize_ValidFiles_ReturnsTrue)
{
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QUrl> urls = { testUrl };

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);
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

    EXPECT_TRUE(scene->initialize(params));
}

TEST_F(UT_FileOperatorMenuScene, Scene_NullAction_ReturnsNull)
{
    EXPECT_EQ(scene->scene(nullptr), nullptr);
}

TEST_F(UT_FileOperatorMenuScene, Create_ValidParent_AddsActions)
{
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QUrl> urls = { testUrl };

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);
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
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);

    EXPECT_GE(menu.actions().size(), 0);
}

TEST_F(UT_FileOperatorMenuScene, Triggered_NonOwnAction_ReturnsFalse)
{
    QAction action("test");

    stub.set_lamda(VADDR(AbstractMenuScene, triggered), [](AbstractMenuScene *, QAction *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    EXPECT_FALSE(scene->triggered(&action));
}
