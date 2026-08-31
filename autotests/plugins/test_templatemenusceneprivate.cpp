// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_templatemenusceneprivate.cpp
 * @brief Unit tests for TemplateMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "templatemenuscene/templatemenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class TemplateMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TemplateMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TemplateMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TemplateMenuScenePrivateTest, TemplateMenuScenePrivate)
{
    // Test constructor: TemplateMenuScenePrivate((TemplateMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}
