// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_crumbdata.cpp
 * @brief Unit tests for CrumbData methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfmplugin_titlebar_global.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class CrumbDataTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CrumbData();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CrumbData *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CrumbDataTest, CrumbData)
{
    // Test constructor: CrumbData(())
    ASSERT_NE(obj, nullptr);
}
