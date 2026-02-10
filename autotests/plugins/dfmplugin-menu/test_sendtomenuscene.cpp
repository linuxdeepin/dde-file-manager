// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "menuscene/sendtomenuscene.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

#include <gtest/gtest.h>

#include <QMenu>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;

class UT_SendToMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        scene = new SendToMenuScene();
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
    SendToMenuScene *scene { nullptr };
    stub_ext::StubExt stub;
};

class UT_SendToMenuCreator : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        creator = new SendToMenuCreator();
    }

    virtual void TearDown() override
    {
        delete creator;
        creator = nullptr;
    }

protected:
    SendToMenuCreator *creator { nullptr };
};

TEST_F(UT_SendToMenuCreator, Name_ReturnsCorrectName)
{
    EXPECT_EQ(SendToMenuCreator::name(), "SendToMenu");
}

TEST_F(UT_SendToMenuCreator, Create_ReturnsSendToMenuScene)
{
    auto scene = creator->create();
    EXPECT_NE(scene, nullptr);
    EXPECT_EQ(scene->name(), "SendToMenu");
    delete scene;
}

TEST_F(UT_SendToMenuScene, Name_ReturnsCorrectName)
{
    EXPECT_EQ(scene->name(), "SendToMenu");
}

TEST_F(UT_SendToMenuScene, Initialize_EmptySelectFiles_ReturnsFalse)
{
    QVariantHash params;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>());

    EXPECT_FALSE(scene->initialize(params));
}

TEST_F(UT_SendToMenuScene, Initialize_ValidFiles_ReturnsTrue)
{
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QUrl> urls = { testUrl };

    QVariantHash params;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);

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

TEST_F(UT_SendToMenuScene, Scene_NullAction_ReturnsNull)
{
    EXPECT_EQ(scene->scene(nullptr), nullptr);
}
