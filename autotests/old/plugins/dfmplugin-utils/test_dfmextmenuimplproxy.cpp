// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/menuimpl/dfmextmenuimplproxy.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/menuimpl/private/dfmextmenuimplproxy_p.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/menuimpl/dfmextmenuimpl.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/menuimpl/dfmextactionimpl.h"

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
USING_DFMEXT_NAMESPACE

class UT_DFMExtMenuImplProxy : public testing::Test
{
protected:
    void SetUp() override
    {
        proxy = new DFMExtMenuImplProxy();
    }

    void TearDown() override
    {
        delete proxy;
        proxy = nullptr;
        stub.clear();
    }

    DFMExtMenuImplProxy *proxy { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_DFMExtMenuImplProxy, Constructor_CreatesProxy)
{
    EXPECT_NE(proxy, nullptr);
}

// ========== DFMExtMenuImplProxyPrivate Tests ==========

class UT_DFMExtMenuImplProxyPrivate : public testing::Test
{
protected:
    void SetUp() override
    {
        d = new DFMExtMenuImplProxyPrivate();
    }

    void TearDown() override
    {
        delete d;
        d = nullptr;
        stub.clear();
    }

    DFMExtMenuImplProxyPrivate *d { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_DFMExtMenuImplProxyPrivate, createMenu_ReturnsNewMenu)
{
    DFMExtMenu *menu = d->createMenu();

    EXPECT_NE(menu, nullptr);

    d->deleteMenu(menu);
}

TEST_F(UT_DFMExtMenuImplProxyPrivate, deleteMenu_NullMenu_ReturnsTrue)
{
    bool result = d->deleteMenu(nullptr);

    EXPECT_TRUE(result);
}

TEST_F(UT_DFMExtMenuImplProxyPrivate, deleteMenu_ExternalMenu_ReturnsTrue)
{
    DFMExtMenu *menu = d->createMenu();

    bool result = d->deleteMenu(menu);

    EXPECT_TRUE(result);
}

TEST_F(UT_DFMExtMenuImplProxyPrivate, createAction_ReturnsNewAction)
{
    DFMExtAction *action = d->createAction();

    EXPECT_NE(action, nullptr);

    d->deleteAction(action);
}

TEST_F(UT_DFMExtMenuImplProxyPrivate, deleteAction_NullAction_ReturnsTrue)
{
    bool result = d->deleteAction(nullptr);

    EXPECT_TRUE(result);
}

TEST_F(UT_DFMExtMenuImplProxyPrivate, deleteAction_ExternalAction_ReturnsTrue)
{
    DFMExtAction *action = d->createAction();

    bool result = d->deleteAction(action);

    EXPECT_TRUE(result);
}

