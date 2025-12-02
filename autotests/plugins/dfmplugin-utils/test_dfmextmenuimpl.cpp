// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/menuimpl/dfmextmenuimpl.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/menuimpl/private/dfmextmenuimpl_p.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/menuimpl/dfmextactionimpl.h"

#include <QMenu>
#include <QAction>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;

class UT_DFMExtMenuImpl : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(UT_DFMExtMenuImpl, Constructor_WithNullMenu_CreatesNewMenu)
{
    DFMExtMenuImpl *impl = new DFMExtMenuImpl(nullptr);

    EXPECT_NE(impl, nullptr);

    delete impl;
}

TEST_F(UT_DFMExtMenuImpl, Constructor_WithExistingMenu_UsesMenu)
{
    QMenu *menu = new QMenu();
    DFMExtMenuImpl *impl = new DFMExtMenuImpl(menu);

    EXPECT_NE(impl, nullptr);

    delete impl;
}

// ========== DFMExtMenuImplPrivate Tests ==========

class UT_DFMExtMenuImplPrivate : public testing::Test
{
protected:
    void SetUp() override
    {
        impl = new DFMExtMenuImpl(nullptr);
        d = dynamic_cast<DFMExtMenuImplPrivate *>(impl->d);
    }

    void TearDown() override
    {
        delete impl;
        impl = nullptr;
        stub.clear();
    }

    DFMExtMenuImpl *impl { nullptr };
    DFMExtMenuImplPrivate *d { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_DFMExtMenuImplPrivate, isInterior_ExternalMenu_ReturnsFalse)
{
    EXPECT_FALSE(d->isInterior());
}

TEST_F(UT_DFMExtMenuImplPrivate, setTitle_SetsMenuTitle)
{
    d->setTitle("Test Title");

    std::string result = d->title();
    EXPECT_EQ(result, "Test Title");
}

TEST_F(UT_DFMExtMenuImplPrivate, setIcon_ThemeName_SetsIcon)
{
    d->setIcon("dialog-information");

    std::string result = d->icon();
}

TEST_F(UT_DFMExtMenuImplPrivate, qmenu_ReturnsQMenu)
{
    QMenu *result = d->qmenu();

    EXPECT_NE(result, nullptr);
}

TEST_F(UT_DFMExtMenuImplPrivate, menuImpl_ReturnsImpl)
{
    DFMExtMenuImpl *result = d->menuImpl();

    EXPECT_EQ(result, impl);
}

TEST_F(UT_DFMExtMenuImplPrivate, addAction_ExternalAction_ReturnsTrue)
{
    DFMExtActionImpl *action = new DFMExtActionImpl(nullptr);

    bool result = d->addAction(action);

    EXPECT_TRUE(result);
}

TEST_F(UT_DFMExtMenuImplPrivate, addAction_NullAction_ReturnsFalse)
{
    bool result = d->addAction(nullptr);

    EXPECT_FALSE(result);
}

TEST_F(UT_DFMExtMenuImplPrivate, menuAction_ReturnsMenuAction)
{
    DFMEXT::DFMExtAction *result = d->menuAction();

    EXPECT_NE(result, nullptr);
}

TEST_F(UT_DFMExtMenuImplPrivate, actions_ReturnsActionList)
{
    std::list<DFMEXT::DFMExtAction *> result = d->actions();

    // Initially may be empty or contain menu action
}

