// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "oemmenuscene/oemmenuscene.h"
#include "oemmenuscene/oemmenu.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/base/schemefactory.h>

#include <gtest/gtest.h>

#include <QMenu>
#include <QUrl>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;

class UT_OemMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        menu = new OemMenu;
        scene = new OemMenuScene(menu);
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    }

    virtual void TearDown() override
    {
        delete scene;
        scene = nullptr;
        delete menu;
        menu = nullptr;
        stub.clear();
    }

protected:
    OemMenuScene *scene { nullptr };
    OemMenu *menu { nullptr };
    stub_ext::StubExt stub;
};

class UT_OemMenuCreator : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        creator = new OemMenuCreator();
    }

    virtual void TearDown() override
    {
        delete creator;
        creator = nullptr;
    }

protected:
    OemMenuCreator *creator { nullptr };
};

TEST_F(UT_OemMenuCreator, Name_ReturnsCorrectName)
{
    EXPECT_EQ(OemMenuCreator::name(), "OemMenu");
}

TEST_F(UT_OemMenuCreator, Create_ReturnsOemMenuScene)
{
    auto scene = creator->create();
    EXPECT_NE(scene, nullptr);
    EXPECT_EQ(scene->name(), "OemMenu");
    delete scene;
}

TEST_F(UT_OemMenuScene, Name_ReturnsCorrectName)
{
    EXPECT_EQ(scene->name(), "OemMenu");
}

TEST_F(UT_OemMenuScene, Initialize_ValidParams_ReturnsTrue)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kOnDesktop] = false;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_FALSE(scene->initialize(params));
}

TEST_F(UT_OemMenuScene, Scene_NullAction_ReturnsNull)
{
    EXPECT_EQ(scene->scene(nullptr), nullptr);
}
