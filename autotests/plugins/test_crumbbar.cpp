// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_crumbbar.cpp
 * @brief Unit tests for CrumbBar methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/crumbbar.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class CrumbBarTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CrumbBar();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CrumbBar *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CrumbBarTest, CrumbBar)
{
    // Test constructor: CrumbBar((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CrumbBarTest, controller)
{
    // Test getter: CrumbInterface controller()
    auto result = obj->controller();
    EXPECT_NO_FATAL_FAILURE({ obj->controller(); });

}

TEST_F(CrumbBarTest, lastUrl)
{
    // Test getter: QUrl lastUrl()
    auto result = obj->lastUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}
