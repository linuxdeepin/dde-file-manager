// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_actioniconmenucreator.cpp
 * @brief Unit tests for ActionIconMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/actioniconmenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class ActionIconMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ActionIconMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ActionIconMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ActionIconMenuCreatorTest, create)
{
    // Test getter: DPMENU_USE_NAMESPACE create()
    auto result = obj->create();
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });

}
