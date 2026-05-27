// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "menuscene/newcreatemenuscene.h"
#include "menuscene/action_defines.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/schemefactory.h>

#include <gtest/gtest.h>

#include <QMenu>
#include <QAction>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;

class UT_NewCreateMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        scene = new NewCreateMenuScene();
    }

    virtual void TearDown() override
    {
        delete scene;
        scene = nullptr;
        stub.clear();
    }

protected:
    NewCreateMenuScene *scene { nullptr };
    stub_ext::StubExt stub;
};

class UT_NewCreateMenuCreator : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        creator = new NewCreateMenuCreator();
    }

    virtual void TearDown() override
    {
        delete creator;
        creator = nullptr;
    }

protected:
    NewCreateMenuCreator *creator { nullptr };
};

// NewCreateMenuCreator 测试

TEST_F(UT_NewCreateMenuCreator, Name_ReturnsCorrectName)
{
    EXPECT_EQ(NewCreateMenuCreator::name(), "NewCreateMenu");
}

TEST_F(UT_NewCreateMenuCreator, Create_ReturnsNewCreateMenuScene)
{
    auto scene = creator->create();
    EXPECT_NE(scene, nullptr);
    EXPECT_EQ(scene->name(), "NewCreateMenu");
    delete scene;
}

// NewCreateMenuScene 测试

TEST_F(UT_NewCreateMenuScene, Name_ReturnsCorrectName)
{
    EXPECT_EQ(scene->name(), "NewCreateMenu");
}

TEST_F(UT_NewCreateMenuScene, Initialize_InvalidCurrentDir_ReturnsFalse)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl();
    params[MenuParamKey::kOnDesktop] = false;

    EXPECT_FALSE(scene->initialize(params));
}

TEST_F(UT_NewCreateMenuScene, Initialize_ValidCurrentDir_ReturnsTrue)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kOnDesktop] = false;
    params[MenuParamKey::kWindowId] = 0ULL;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_TRUE(scene->initialize(params));
}

TEST_F(UT_NewCreateMenuScene, Scene_NullAction_ReturnsNull)
{
    EXPECT_EQ(scene->scene(nullptr), nullptr);
}

TEST_F(UT_NewCreateMenuScene, Scene_OwnAction_ReturnsSelf)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kOnDesktop] = false;

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

    auto actions = menu.actions();
    if (!actions.isEmpty()) {
        EXPECT_EQ(scene->scene(actions.first()), scene);
    }
}

TEST_F(UT_NewCreateMenuScene, Create_NullParent_ReturnsFalse)
{
    EXPECT_FALSE(scene->create(nullptr));
}

TEST_F(UT_NewCreateMenuScene, Create_ValidParent_AddsActions)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kOnDesktop] = false;

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

    auto actions = menu.actions();
    EXPECT_GE(actions.size(), 2);  // 至少有 NewFolder 和 NewDoc

    bool hasNewFolder = false;
    bool hasNewDoc = false;
    for (auto action : actions) {
        QString actionId = action->property(ActionPropertyKey::kActionID).toString();
        if (actionId == QString(ActionID::kNewFolder))
            hasNewFolder = true;
        if (actionId == QString(ActionID::kNewDoc))
            hasNewDoc = true;
    }

    EXPECT_TRUE(hasNewFolder);
    EXPECT_TRUE(hasNewDoc);
}

TEST_F(UT_NewCreateMenuScene, Triggered_NonOwnAction_ReturnsFalse)
{
    QAction action("test");

    stub.set_lamda(VADDR(AbstractMenuScene, triggered), [](AbstractMenuScene *, QAction *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    EXPECT_FALSE(scene->triggered(&action));
}
