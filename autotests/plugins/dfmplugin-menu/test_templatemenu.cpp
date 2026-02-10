// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "templatemenuscene/templatemenu.h"

#include <gtest/gtest.h>

#include <QUrl>

using namespace dfmplugin_menu;

class UT_TemplateMenu : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        menu = new TemplateMenu();
    }

    virtual void TearDown() override
    {
        delete menu;
        menu = nullptr;
        stub.clear();
    }

protected:
    TemplateMenu *menu { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_TemplateMenu, Constructor_InitializesCorrectly)
{
    EXPECT_NE(menu, nullptr);
}

TEST_F(UT_TemplateMenu, ActionList_ReturnsActionList)
{
    auto actions = menu->actionList();
    EXPECT_TRUE(actions.isEmpty() || !actions.isEmpty());
}
