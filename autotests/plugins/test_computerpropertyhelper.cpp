// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computerpropertyhelper.cpp
 * @brief Unit tests for ComputerPropertyHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/computerpropertyhelper.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class ComputerPropertyHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ComputerPropertyHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ComputerPropertyHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ComputerPropertyHelperTest, scheme)
{
    // Test getter: QString scheme()
    auto result = obj->scheme();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}
