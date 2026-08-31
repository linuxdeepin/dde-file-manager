// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_smbbrowsermenucreator.cpp
 * @brief Unit tests for SmbBrowserMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/smbbrowsermenuscene.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class SmbBrowserMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SmbBrowserMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SmbBrowserMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SmbBrowserMenuCreatorTest, create)
{
    // Test getter: DFMBASE_USE_NAMESPACE create()
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });
}

TEST_F(SmbBrowserMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
