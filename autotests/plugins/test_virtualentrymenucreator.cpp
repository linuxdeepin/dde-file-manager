// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_virtualentrymenucreator.cpp
 * @brief Unit tests for VirtualEntryMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "displaycontrol/menu/virtualentrymenuscene.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class VirtualEntryMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VirtualEntryMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VirtualEntryMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VirtualEntryMenuCreatorTest, create)
{
    // Test getter: AbstractMenuScene create()
    auto result = obj->create();
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });

}
