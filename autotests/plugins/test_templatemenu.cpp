// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_templatemenu.cpp
 * @brief Unit tests for TemplateMenu methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "templatemenuscene/templatemenu.h"

#include <QTest>

using namespace dfmplugin_menu;

class TemplateMenuTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TemplateMenu();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TemplateMenu *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TemplateMenuTest, TemplateMenu)
{
    // Test constructor: TemplateMenu((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TemplateMenuTest, actionList)
{
    // Test getter: QList<QAction *> actionList()
    auto result = obj->actionList();
    // Pointer return type
    EXPECT_TRUE(result.isEmpty());

}
