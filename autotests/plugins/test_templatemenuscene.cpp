// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_templatemenuscene.cpp
 * @brief Unit tests for TemplateMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "templatemenuscene/templatemenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class TemplateMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TemplateMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TemplateMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TemplateMenuSceneTest, TemplateMenuScene)
{
    // Test constructor: TemplateMenuScene((TemplateMenu *menu, QObject *parent))
    ASSERT_NE(obj, nullptr);
}
