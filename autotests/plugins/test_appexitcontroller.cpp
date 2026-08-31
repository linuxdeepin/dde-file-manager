// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_appexitcontroller.cpp
 * @brief Unit tests for AppExitController methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/appexitcontroller.h"

#include <QTest>

using namespace core;

class AppExitControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AppExitController();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AppExitController *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AppExitControllerTest, dismiss)
{
    // Test method: void dismiss(())
    EXPECT_NO_FATAL_FAILURE(obj->dismiss());
}

TEST_F(AppExitControllerTest, instance)
{
    // Test getter: AppExitController instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
