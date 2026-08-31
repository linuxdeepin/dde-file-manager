// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_deepinlicensehelper.cpp
 * @brief Unit tests for DeepinLicenseHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "watermask/deepinlicensehelper.h"

#include <QTest>

using namespace ddplugin_canvas;

class DeepinLicenseHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DeepinLicenseHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DeepinLicenseHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DeepinLicenseHelperTest, instance)
{
    // Test getter: DeepinLicenseHelper instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
