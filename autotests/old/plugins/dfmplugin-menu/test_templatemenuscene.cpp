// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "templatemenuscene/templatemenuscene.h"
#include "templatemenuscene/templatemenu.h"

#include <dfm-base/dfm_menu_defines.h>

#include <gtest/gtest.h>

#include <QMenu>
#include <QUrl>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;

class UT_TemplateMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        menu = new TemplateMenu;
        scene = new TemplateMenuScene(menu);
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
    TemplateMenuScene *scene { nullptr };
    TemplateMenu *menu { nullptr };
    stub_ext::StubExt stub;
};

class UT_TemplateMenuCreator : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        creator = new TemplateMenuCreator();
    }

    virtual void TearDown() override
    {
        delete creator;
        creator = nullptr;
    }

protected:
    TemplateMenuCreator *creator { nullptr };
};

TEST_F(UT_TemplateMenuCreator, Name_ReturnsCorrectName)
{
    EXPECT_EQ(TemplateMenuCreator::name(), "TemplateMenu");
}

TEST_F(UT_TemplateMenuCreator, Create_ReturnsTemplateMenuScene)
{
    auto scene = creator->create();
    EXPECT_NE(scene, nullptr);
    EXPECT_EQ(scene->name(), "TemplateMenu");
    delete scene;
}

TEST_F(UT_TemplateMenuScene, Name_ReturnsCorrectName)
{
    EXPECT_EQ(scene->name(), "TemplateMenu");
}

TEST_F(UT_TemplateMenuScene, Initialize_ValidParams_ReturnsTrue)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kIsEmptyArea] = true;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_TRUE(scene->initialize(params));
}

TEST_F(UT_TemplateMenuScene, Scene_NullAction_ReturnsNull)
{
    EXPECT_EQ(scene->scene(nullptr), nullptr);
}
