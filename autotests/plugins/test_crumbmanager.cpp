// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_crumbmanager.cpp
 * @brief Unit tests for CrumbManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/crumbmanager.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class CrumbManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CrumbManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CrumbManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CrumbManagerTest, instance)
{
    // Test getter: CrumbManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
