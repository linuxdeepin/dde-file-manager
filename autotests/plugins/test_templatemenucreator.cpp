// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_templatemenucreator.cpp
 * @brief Unit tests for TemplateMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "templatemenuscene/templatemenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class TemplateMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TemplateMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TemplateMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TemplateMenuCreatorTest, create)
{
    // Test getter: DFMBASE_USE_NAMESPACE create()
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });
}

TEST_F(TemplateMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
